#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include "baf/baf.h"

#define US_TO_MS    (1000)

static void test1(void);

void doLoop(int iters) {
   baf_Error res;
   uint32_t nextTimeMS;
   uint32_t fakeTime = 0;

   for(int i = 0; i < iters; i++) {
      fakeTime = i * 100;

      res = baf_giveTime(fakeTime, &nextTimeMS);
      if(res != BAF_OK) {
         printf("%d: Give Time returned %d\n", i, res);
      }

      usleep(US_TO_MS * fakeTime);
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

struct baf_Animation t1a1 = {
   .id                     = 3,
   .numSteps               = 1,
   .timeStepMS             = 100,
   .type                   = BAF_ASCHED_SIMPLE_RANDOM_LOOP,

   .aRandomSimpleLoop      = {
      //.id set programmatically
      .transitionTimeMS     = 50,
      .params              = {
         .maxValue         = 100,
         .minValue         = 0,
         .biasValue        = 0,
         .biasWeight       = 0,
      },
   },
};

void test1(void) {
   /*
   yabi_Error res;

   struct yabi_Config cfg = {
      .frameStartCB           = t1_FrameStart,
      .frameEndCB             = t1_FrameEnd,
      .channelChangeCB        = t1_ChanCH,
      .channelChangeGroupCB   = t1_ChanGroupCH,
      .hwConfig = {
         .setup               = t1_HwSetup,
         .teardown            = t1_HwTeardown,
         .hwConfig            = NULL,
      },
   };
   */

   baf_Error res;

   struct baf_Config cfg = {
      .rngCB               = NULL,
      .animationStartCB    = NULL,
      .animationStopCB     = NULL,
      .setChannelGroupCB   = t1ChanGroupSet,

      .hwSetupCB           = NULL,
      .hwTeardownCB        = NULL,
   };

   res = baf_init(&cfg);
   if(res != BAF_OK) {
      printf("Start returned %d\n", res);
      return;
   }

   //setup the channel IDs for this animation
   for(int i = 0; i < BAF_CHANNEL_WIDTH; i++) {
      t1a1.aRandomSimpleLoop.id[i] = i;
   }

   res = baf_startAnimation(&t1a1, BAF_ASTART_IMMEDIATE);
   if(res != BAF_OK) {
      printf("Start returned %d\n", res);
   }

   doLoop(10);
}
