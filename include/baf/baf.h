#ifndef BAF_H__
#define BAF_H__

#include <stdint.h>
#include <stdbool.h>

typedef enum {
   BAF_OK,
   BAF_UNIMPLIMENTED,
   BAF_ANIMATION_NOT_STARTED,
   BAF_ERR,
} baf_Error;

//TODO support other types?
typedef uint32_t     baf_ChannelID;
typedef uint32_t     baf_ChannelValue;
typedef uint32_t     baf_AnimationStep;

typedef enum {
   BAF_ASTART_IMMEDIATE,
   BAF_ASTART_ON_FINISH,
} baf_AnimationWhen;

//FIXME rename this
struct baf_ChannelSetting {
   baf_ChannelID     id;
   baf_ChannelValue  val;
   uint32_t          transitionTimeMS;
};

typedef enum {
   BAF_ASCHED_ONESHOT,
   BAF_ASCHED_LOOP,
   BAF_ASCHED_RANDOM_LOOP,
   //BAF_ASCHED_ONESHOT_SWEEP,
} baf_AnimationType;

struct baf_AnimationStepSimple {
   struct baf_ChannelSetting  setting;
};

struct baf_AnimationStepRandom {
   struct baf_ChannelSetting  setting;
   baf_ChannelValue           maxValue;
   baf_ChannelValue           minValue;

   //FIXME push up to user?
   baf_ChannelValue           biasValue;
   uint8_t                    biasWeight; //0 (none) - 100
};

struct baf_Animation {
   uint32_t                            numSteps;
   uint32_t                            timeStepMS;

   baf_AnimationType                   type;
   //there should be `numSteps` elements in this array
   union {
      struct baf_AnimationStepSimple*  aSimple;
      struct baf_AnimationStepSimple*  aLooped;
      struct baf_AnimationStepRandom*  aRandomLoop;
   };
};

typedef void (*baf_GetRNG)(uint32_t rng);
typedef void (*baf_AnimationStartCallback)(struct baf_Animation* const anim);
typedef void (*baf_AnimationStopCallback)(struct baf_Animation* const anim);
typedef void (*baf_SetChannelGroup)(struct baf_ChannelSetting* const channels, uint32_t num);

//TODO do these make sense? Should they be titled 'global'? The void* is for user config data
typedef void* const (*baf_HardwareSetup)(void);
typedef void (*baf_HardwareTeardown)(void* const hwConfig);

struct baf_Config {
   baf_GetRNG                 rngCB;
   baf_AnimationStartCallback animationStartCB;
   baf_AnimationStopCallback  animationStopCB;
   baf_SetChannelGroup        setChannelGroupCB;

   baf_HardwareSetup          hwSetupCB;
   baf_HardwareTeardown       hwTeardownCB;
};


baf_Error baf_init(struct baf_Config cfg);
baf_Error baf_giveTime(uint32_t systimeMS);
baf_Error baf_startAnimation(struct baf_Animation* const anim, baf_AnimationWhen when);
baf_Error baf_stopAnimation(struct baf_Animation* const anim, baf_AnimationWhen when);
baf_Error baf_stopAllAnimations(baf_AnimationWhen when);
bool baf_isInProgress(struct baf_Animation* const anim);


#endif//BAF_H__

