#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "baf/baf.h"

#define US_TO_MS    (1000)
#define STEPGAP_MS  (100)

static void test1(void);

void doLoop(int iters) {
   baf_Error res;
   uint32_t nextTimeMS;
   uint32_t fakeTime = 0;

   for(int i = 0; i < iters; i++) {
      fakeTime = i * STEPGAP_MS;

      res = baf_giveTime(fakeTime, &nextTimeMS);
      if(res != BAF_OK) {
         printf("%d: Give Time returned %d\n", i, res);
      }

      usleep(STEPGAP_MS * US_TO_MS);
   }
}

int main(void) {
   printf("Starting BAF Tests...\n");

   test1();

   return 0;
}

static void t1ChanGroupSet(struct baf_ChannelSetting const * const channels, baf_ChannelValue* const values, uint32_t num) {
   for(int i = 0; i < num; i++) {
      printf("\tSet Chan #%d to %u in %ums\n", channels[i].id, values[i], channels[i].transitionTimeMS);
   }
}

static void t1AStart(struct baf_Animation const * anim) {
   printf("Animation #%u Start\n", anim->id);
}
static void t1AStop(struct baf_Animation const * anim) {
   printf("Animation #%u Stop\n", anim->id);
}

static uint32_t t1RNG(uint32_t range) {
   //return (float)rand()/(float)(RAND_MAX / 100);
   return rand() % range;
}

//backing array of channel IDs
static baf_ChannelID chans[10];

struct baf_Animation t1a1 = {
   .id                     = 3,
   .numSteps               = 1,
   .timeStepMS             = 1000,
   .type                   = BAF_ASCHED_SIMPLE_RANDOM_LOOP,

   .aRandomSimpleLoop      = {
      .id                  = chans,
      .idLen               = 10,

      .transitionTimeMS    = 50,
      .params              = {
         .maxValue         = 100,
         .minValue         = 0,
         .biasValue        = 90,
         .biasWeight       = 50,
      },
   },
};

void test1(void) {
   baf_Error res;

   struct baf_Config cfg = {
      .rngCB               = t1RNG,
      .animationStartCB    = t1AStart,
      .animationStopCB     = t1AStop,
      .setChannelGroupCB   = t1ChanGroupSet,
   };

   res = baf_init(&cfg);
   if(res != BAF_OK) {
      printf("Start returned %d\n", res);
      return;
   }

   //setup the channel IDs for this animation
   for(int i = 0; i < t1a1.aRandomSimpleLoop.idLen; i++) {
      t1a1.aRandomSimpleLoop.id[i] = i;
   }

   res = baf_startAnimation(&t1a1, BAF_ASTART_IMMEDIATE);
   if(res != BAF_OK) {
      printf("Start returned %d\n", res);
   }

   doLoop(50);
}
