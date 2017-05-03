#include "baf/baf.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#define IS_ONESHOT(x)   (x->type > BAF_ASCHED_ONESHOT_START && x->type < BAF_ASCHED_LOOP_START)
#define IS_LOOPED(x)    (x->type > BAF_ASCHED_LOOP_START && x->type < BAF_ASCHED_END)

struct baf_State {
   bool                          init;

   bool                          running;
   baf_AnimationStep             step;
   uint32_t                      lastStepTimeMS;
   struct baf_Animation const *  animActive;
   struct baf_Animation const *  animNext;

   struct baf_Config             config;
};
static struct baf_State state = {.init = false};

static baf_ChannelValue baf_calcRandomChannelValue(struct baf_RandomParameters const * const params);
static bool baf_areAnimationsSame(struct baf_Animation const * const a, struct baf_Animation const * const b);

baf_Error baf_init(struct baf_Config* const cfg) {
   if(!cfg) {
      return BAF_BAD_PARAM;
   }
   else if(!cfg->setChannelGroupCB) {
      //this is the min for an animation to work
      return BAF_BAD_PARAM;
   }

   state.config = *cfg;
   state.animActive = NULL;
   state.animNext = NULL;

   state.init = true;
   return BAF_OK;
}

/*
 * Optional relative time until the next event will occur (in MS).
 */
baf_Error baf_giveTime(uint32_t const systimeMS, uint32_t* const timeTillNextMS) {
   //TODO finish
   //run any animation triggers which should have already happened
   //if none to run, return
   //if this was an RTOS, this thread would sleep between events

   //TODO make these asserts
   if(!state.init) {
      return BAF_ERR;
   }
   else if(!state.running) {
      return BAF_ANIMATION_NOT_STARTED;
   }
   else if(!state.animActive) {
      return BAF_ERR;
   }

   //check to see if any control points have passed
   baf_AnimationStep newStep = systimeMS / state.animActive->timeStepMS;
   newStep %= state.animActive->numSteps;

   if(systimeMS < state.lastStepTimeMS + state.animActive->timeStepMS) {
      //nothing to do, no steps have passed
      return BAF_OK;
   }
   if(newStep >= state.animActive->numSteps) {
      //indicate this animation is done
      if(state.config.animationStopCB) {
         state.config.animationStopCB(state.animActive);
      }

      if(IS_ONESHOT(state.animActive)) {
         //animation is done, start the next one if needed
         state.animActive = state.animNext;
         state.animNext = NULL;
         newStep = 0;
         state.lastStepTimeMS = 0;

         if(!state.animActive) {
            return BAF_OK;
         }
      }
      else if(IS_LOOPED(state.animActive)) {
         //restart
         newStep = 0;
      }

      //indicate the next animation is starting
      if(state.config.animationStartCB) {
         state.config.animationStartCB(state.animActive);
      }
   }

   //execute just the FINAL channel change (aka if we are 5 behind, just jump to the last
   //get the step to display
   state.step = newStep;
   state.lastStepTimeMS = systimeMS;
   switch(state.animActive->type) {

      case BAF_ASCHED_SIMPLE_RANDOM_LOOP:
      {
         struct baf_ChannelSetting                 setting;
         baf_ChannelValue                          value;
         baf_ChannelID const *                     id;

         struct baf_AnimationSimpleRandom const * const asr = &state.animActive->aRandomSimpleLoop;

         setting.transitionTimeMS = asr->transitionTimeMS;

         for(int i = 0; i < BAF_CHANNEL_WIDTH; i++) {
            id = &asr->id[i];
            setting.id = *id;

            //calculate all values
            value = baf_calcRandomChannelValue(&state.animActive->aRandomSimpleLoop.params);

            //send updates one at a time
            state.config.setChannelGroupCB(&setting, &value, 1);
         }

         //TODO apply updates all at once
         break;
      }

      case BAF_ASCHED_ONESHOT:
      case BAF_ASCHED_LOOP:
      case BAF_ASCHED_RANDOM_LOOP:
      default:
         return BAF_UNIMPLIMENTED;
   }

   if(timeTillNextMS) {
      *timeTillNextMS = state.animActive->timeStepMS;
   }

   return BAF_OK;
}

/*
 * Note:`anim` will NOT be deep copied. The user is expected to keep it (and all
 * of it's fields) around until the animation is stopped.
 */
baf_Error baf_startAnimation(struct baf_Animation const * const anim, baf_AnimationWhen when) {
   if(!anim) {
      return BAF_BAD_PARAM;
   }
   else if(when != BAF_ASTART_IMMEDIATE) {
      //TODO test and unlock this
      return BAF_UNIMPLIMENTED;
   }

   if(when == BAF_ASTART_ON_FINISH) {
      state.animNext = anim;
   }
   else {
      state.animActive = anim;
      state.lastStepTimeMS = 0;
      state.step = 0;
   }
   state.step = 0;
   state.running = true;

   //prime it
   //FIXME use a real systime? maybe from a callback we take in config?
   baf_giveTime(0, NULL);

   return BAF_OK;
}

baf_Error baf_stopAnimation(struct baf_Animation const * const anim, baf_AnimationWhen when) {
   if(!anim) {
      return BAF_BAD_PARAM;
   }
   else if(when != BAF_ASTART_IMMEDIATE) {
      return BAF_UNIMPLIMENTED;
   }

   if(state.running && baf_areAnimationsSame(anim, state.animActive)) {
      state.running = false;
   }

   return BAF_OK;
}

baf_Error baf_stopAllAnimations(baf_AnimationWhen when) {
   if(when != BAF_ASTART_IMMEDIATE) {
      return BAF_UNIMPLIMENTED;
   }

   state.running = false;

   return BAF_OK;
}

bool baf_isInProgress(struct baf_Animation* const anim) {
   return baf_areAnimationsSame(anim, state.animActive);
}

static bool baf_areAnimationsSame(struct baf_Animation const * const a, struct baf_Animation const * const b) {
   return a->id == b->id;
}

static baf_ChannelValue baf_calcRandomChannelValue(struct baf_RandomParameters const * const params) {
   baf_ChannelValue v = (baf_ChannelValue)(state.config.rngCB(params->maxValue - params->minValue));
   v += params->minValue;

   //now apply bias
   float bias = (float)params->biasValue;
   //TODO apply bias variance
   float bweight = (float)params->biasWeight / 100.0;
   v = (baf_ChannelValue)(((float)v * (1.0 - bweight)) + (bias * bweight));

   return v;
}


