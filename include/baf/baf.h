#ifndef BAF_H__
#define BAF_H__

#include <stdint.h>
#include <stdbool.h>

//FIXME find a better way to handle this
//the number of channels 'wide' each animation is
#define BAF_CHANNEL_WIDTH           10

//TODO add macros for allocating animations of certain length

typedef enum {
   BAF_OK = 0,
   BAF_UNIMPLIMENTED,
   BAF_BAD_PARAM,
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
   uint32_t          transitionTimeMS;
};

typedef enum {
   BAF_ASCHED_ONESHOT_START,
   BAF_ASCHED_ONESHOT,
   //BAF_ASCHED_ONESHOT_SWEEP,

   BAF_ASCHED_LOOP_START,
   BAF_ASCHED_LOOP,
   BAF_ASCHED_RANDOM_LOOP,
   BAF_ASCHED_SIMPLE_RANDOM_LOOP,      // one 'step', which is randomly generated across all channels forever

   BAF_ASCHED_END,
} baf_AnimationType;

struct baf_AnimationStepSimple {
   //FIXME does this need to be an array?
   struct baf_ChannelSetting  setting;
   baf_ChannelValue           val;
};

struct baf_RandomParameters {
   baf_ChannelValue           maxValue;
   baf_ChannelValue           minValue;

   //FIXME push up to user?
   baf_ChannelValue           biasValue;        // what value to tend the RNG value towards
   uint8_t                    biasWeight;       //0 (none) - 100, how much should the RNG value tend towards the bias value
   baf_ChannelValue           biasVariance;     //how much RNG to apply to the bias value
};

struct baf_AnimationStepRandom {
   //ignore val, but use transition time and id
   struct baf_ChannelSetting     setting;
   struct baf_RandomParameters   params;
};

struct baf_AnimationSimpleRandom {
   baf_ChannelID                 id[BAF_CHANNEL_WIDTH];
   uint32_t                      transitionTimeMS;
   struct baf_RandomParameters   params;
};

struct baf_Animation {
   uint32_t                id;            //a tag for this animation
   uint32_t                numSteps;      //the number of steps long this animation is
   uint32_t                timeStepMS;    //the time between steps for this animation

   baf_AnimationType       type;
   //there should be `numSteps` elements in this array
   union {
      struct baf_AnimationStepSimple*  aSimple[BAF_CHANNEL_WIDTH];
      struct baf_AnimationStepSimple*  aLooped[BAF_CHANNEL_WIDTH];
      struct baf_AnimationSimpleRandom aRandomSimpleLoop;
      struct baf_AnimationStepRandom*  aRandomLoop[BAF_CHANNEL_WIDTH];
   };
};

typedef uint32_t (*baf_GetRNG)(uint32_t range);
typedef void (*baf_AnimationStartCallback)(struct baf_Animation const * anim);
typedef void (*baf_AnimationStopCallback)(struct baf_Animation const * anim);
typedef void (*baf_SetChannelGroup)(struct baf_ChannelSetting const * const channels, baf_ChannelValue* const values, uint32_t num);

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

baf_Error baf_init(struct baf_Config* const cfg);
baf_Error baf_giveTime(uint32_t const systimeMS, uint32_t* const timeTillNextMS);
baf_Error baf_startAnimation(struct baf_Animation const * const anim, baf_AnimationWhen when);
baf_Error baf_stopAnimation(struct baf_Animation const * const anim, baf_AnimationWhen when);
baf_Error baf_stopAllAnimations(baf_AnimationWhen when);
bool baf_isInProgress(struct baf_Animation* const anim);


#endif//BAF_H__

