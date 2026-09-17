/* ==========================================================================
   animation.h  -  Timer driven animation state
   --------------------------------------------------------------------------
   CGM concepts: TIMER BASED ANIMATION, KEYFRAME / PHYSICS INTERPOLATION,
                 PARTICLE SYSTEMS (rain, fireworks).
   Every value below is *read* by stadium.cpp while drawing, and *written*
   only by animUpdate(), so the scene graph stays a pure function of state.
   ========================================================================== */
#ifndef ANIMATION_H
#define ANIMATION_H

#include "common.h"

/* Delivery state machine */
enum BallPhase {
    BP_READY = 0,   /* bowler waiting at the top of his mark   */
    BP_RUNUP,       /* running in                              */
    BP_DELIVERY,    /* bowling arm rotating                    */
    BP_TRAVEL,      /* ball on its way to the batsman          */
    BP_STRUCK,      /* ball hit, flying towards the boundary   */
    BP_RESULT       /* result banner / celebration             */
};

extern int   gPhase;
extern Vec3  gBallPos;
extern Vec3  gBallVel;
extern bool  gBallVisible;

extern float gBowlerZ;        /* bowler position during the run-up          */
extern float gBowlerArm;      /* bowling arm rotation, degrees              */
extern float gBatSwing;       /* bat rotation, degrees                      */
extern float gBatsmanLean;    /* small body lean while playing the shot     */

extern float gCrowdPhase;     /* drives the Mexican wave                    */
extern float gCrowdExcite;    /* 0..1, rises after a boundary               */
extern float gFlagPhase;      /* drives the waving flags                    */
extern float gCloudShift;     /* cloud drift offset                         */
extern float gAutoBowlTimer;  /* countdown to the next automatic delivery   */
extern bool  gAutoPlay;       /* keep bowling by itself                     */

void animInit();
void animUpdate(float dt);
void animBowl();              /* start a delivery now (SPACE)               */
void animResetMatch();        /* clear the scoreboard                       */

void drawFireworks();         /* particle burst after a six                 */
void drawRain();              /* rain streaks in W_RAIN mode                */

#endif /* ANIMATION_H */
