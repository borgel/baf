#include "baf/baf.h"

#include <stdint.h>
#include <stdbool.h>

struct baf_State {
   bool                          init;

   //FIXME make a time-until-run?
   bool                          running;
   baf_AnimationStep             step;
   struct baf_Animation const *  animActive;
   struct baf_Animation const *  animNext;

   struct baf_Config       config;
};
static struct baf_State state = {.init = false};

static bool baf_areAnimationsSame(struct baf_Animation const * const a, struct baf_Animation const * const b);

baf_Error baf_init(struct baf_Config* const cfg) {
   if(!cfg) {
      return BAF_BAD_PARAM;
   }
   state.config = *cfg;

   //TODO finish
   //TODO state-tracking structs?

   return BAF_OK;
}

baf_Error baf_giveTime(uint32_t systimeMS) {
   //TODO finish
   //run any animation triggers which should have already happened
   //if none to run, return
   //if this was an RTOS, this thread would sleep between events

   if(!state.init) {
      return BAF_ERR;
   }
   else if(!state.running) {
      return BAF_ANIMATION_NOT_STARTED;
   }

   //TODO
   //check to see if any control points have passed
   //execute those channel changes

   //TODO add a 'get animation slice' command, that advances the current anim state
   //(new struct for postion + anim?) and returns a const ptr to the one to execute

   return BAF_UNIMPLIMENTED;


   //TODO return MS until next event
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
      return BAF_UNIMPLIMENTED;
   }
   else if(anim->type != BAF_ASCHED_ONESHOT) {
      return BAF_UNIMPLIMENTED;
   }

   state.animActive = anim;

   //prime it
   //FIXME use a real systime? maybe from a callback we take in config?
   baf_giveTime(0);

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


