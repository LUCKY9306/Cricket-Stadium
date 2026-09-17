/* ==========================================================================
   camera.h  -  Free-fly camera, projections and the automatic stadium tour
   --------------------------------------------------------------------------
   CGM concepts: VIEWING / CAMERA TRANSFORMATION (gluLookAt),
                 PERSPECTIVE vs ORTHOGRAPHIC PROJECTION.
   ========================================================================== */
#ifndef CAMERA_H
#define CAMERA_H

#include "common.h"

struct Camera {
    Vec3  eye;        /* camera position in world space                     */
    float yaw;        /* left/right rotation, degrees                       */
    float pitch;      /* up/down rotation, degrees (clamped +-89)           */
    float fov;        /* vertical field of view -> zoom                     */
};

extern Camera gCam;
extern bool   gOrtho;       /* orthographic projection toggle ('P')         */
extern bool   gBirdsEye;    /* top-down bird's-eye view ('B')               */
extern bool   gTour;        /* automatic stadium tour ('T'... key 'O')      */
extern bool   gMouseLook;   /* mouse look enabled ('M')                     */

void  cameraInit();
void  cameraApplyProjection();          /* builds GL_PROJECTION matrix      */
void  cameraApplyView();                /* builds the gluLookAt view matrix */
void  cameraMove(float fwd, float strafe, float upDown);
void  cameraLook(float dYaw, float dPitch);
void  cameraZoom(float delta);
void  cameraUpdate(float dt);           /* advances the automatic tour      */
void  cameraPreset(int n);              /* 1..5 fixed viewpoints            */
Vec3  cameraForward();

#endif /* CAMERA_H */
