/* ==========================================================================
   camera.cpp  -  View and projection transformations
   ========================================================================== */
#include "camera.h"

Camera gCam;
bool   gOrtho     = false;
bool   gBirdsEye  = false;
bool   gTour      = false;
bool   gMouseLook = false;

static float tourAngle = 0.0f;   /* current angle on the circular tour path */

void cameraInit()
{
    /* Start behind the bowler's end, looking down the pitch. */
    gCam.eye   = vec3(0.0f, 6.0f, -34.0f);
    gCam.yaw   = 90.0f;
    gCam.pitch = -8.0f;
    gCam.fov   = 60.0f;
}

/* Unit vector the camera is looking along, from yaw/pitch (spherical -> cart). */
Vec3 cameraForward()
{
    float cy = cosf(gCam.yaw   * DEG2RAD), sy = sinf(gCam.yaw   * DEG2RAD);
    float cp = cosf(gCam.pitch * DEG2RAD), sp = sinf(gCam.pitch * DEG2RAD);
    return vec3(cp * cy, sp, cp * sy);
}

/* ---- PROJECTION TRANSFORMATION ---------------------------------------- */
void cameraApplyProjection()
{
    float aspect = (gWinH > 0) ? (float)gWinW / (float)gWinH : 1.0f;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    if (gOrtho) {
        /* Orthographic: parallel projection, no perspective foreshortening.
           The half-height is tied to fov so that zoom still works. */
        float h = gCam.fov * 1.6f;
        float w = h * aspect;
        glOrtho(-w, w, -h, h, 0.1f, 800.0f);
    } else {
        /* Perspective: the usual viewing frustum. */
        gluPerspective(gCam.fov, aspect, 0.1f, 800.0f);
    }
    glMatrixMode(GL_MODELVIEW);
}

/* ---- VIEW / CAMERA TRANSFORMATION -------------------------------------- */
void cameraApplyView()
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (gBirdsEye) {
        /* Straight down over the middle of the ground. The "up" vector is
           +Z because looking along -Y makes the usual +Y up degenerate. */
        gluLookAt(0.0f, 150.0f, 0.001f,
                  0.0f, 0.0f,   0.0f,
                  0.0f, 0.0f,   1.0f);
        return;
    }

    Vec3 f = cameraForward();
    gluLookAt(gCam.eye.x, gCam.eye.y, gCam.eye.z,
              gCam.eye.x + f.x, gCam.eye.y + f.y, gCam.eye.z + f.z,
              0.0f, 1.0f, 0.0f);
}

/* ---- Movement ---------------------------------------------------------- */
void cameraMove(float fwd, float strafe, float upDown)
{
    if (gTour) return;                       /* tour owns the camera */

    Vec3 f = cameraForward();
    /* Walk on the horizontal plane so W/S do not dive into the ground. */
    float len = sqrtf(f.x * f.x + f.z * f.z);
    if (len < 0.0001f) len = 1.0f;
    float fx = f.x / len, fz = f.z / len;
    /* Right vector = forward x up, for the XZ plane this is (fz, 0, -fx). */
    float rx = fz, rz = -fx;

    gCam.eye.x += fwd * fx + strafe * rx;
    gCam.eye.z += fwd * fz + strafe * rz;
    gCam.eye.y += upDown;

    gCam.eye.y = clampf(gCam.eye.y, 0.6f, 160.0f);
    /* Keep the camera inside the stadium bowl. */
    float d = sqrtf(gCam.eye.x * gCam.eye.x + gCam.eye.z * gCam.eye.z);
    if (d > 120.0f) {
        gCam.eye.x *= 120.0f / d;
        gCam.eye.z *= 120.0f / d;
    }
}

void cameraLook(float dYaw, float dPitch)
{
    gCam.yaw  += dYaw;
    if (gCam.yaw > 360.0f) gCam.yaw -= 360.0f;
    if (gCam.yaw < 0.0f)   gCam.yaw += 360.0f;
    gCam.pitch = clampf(gCam.pitch + dPitch, -89.0f, 89.0f);
}

void cameraZoom(float delta)
{
    gCam.fov = clampf(gCam.fov + delta, 12.0f, 100.0f);
}

/* ---- Automatic stadium tour -------------------------------------------- */
void cameraUpdate(float dt)
{
    if (!gTour) return;

    tourAngle += dt * 9.0f;                       /* degrees per second  */
    if (tourAngle >= 360.0f) tourAngle -= 360.0f;

    float a = tourAngle * DEG2RAD;
    float r = 78.0f + 12.0f * sinf(a * 2.0f);     /* gentle in/out sweep */
    float h = 18.0f + 10.0f * sinf(a * 1.5f);     /* and rise/fall       */

    gCam.eye = vec3(r * cosf(a), h, r * sinf(a));

    /* Always aim back at the pitch: yaw from the position, pitch from height. */
    gCam.yaw   = atan2f(-gCam.eye.z, -gCam.eye.x) * RAD2DEG;
    gCam.pitch = -atan2f(gCam.eye.y - 1.5f, r) * RAD2DEG;
}

/* ---- Fixed viewpoints for the demo / viva ------------------------------ */
void cameraPreset(int n)
{
    gTour     = false;
    gBirdsEye = false;

    switch (n) {
    case 1:   /* bowler's run-up view */
        gCam.eye = vec3(0.0f, 5.0f, -34.0f);  gCam.yaw = 90.0f;  gCam.pitch = -6.0f;
        break;
    case 2:   /* batsman's end */
        gCam.eye = vec3(0.0f, 4.0f, 30.0f);   gCam.yaw = 270.0f; gCam.pitch = -6.0f;
        break;
    case 3:   /* square-leg umpire */
        gCam.eye = vec3(26.0f, 4.0f, 0.0f);   gCam.yaw = 180.0f; gCam.pitch = -5.0f;
        break;
    case 4:   /* high in the stands */
        gCam.eye = vec3(-62.0f, 24.0f, -52.0f); gCam.yaw = 40.0f; gCam.pitch = -18.0f;
        break;
    case 5:   /* behind the scoreboard */
        gCam.eye = vec3(0.0f, 22.0f, -80.0f); gCam.yaw = 90.0f;  gCam.pitch = -12.0f;
        break;
    }
    gCam.fov = 60.0f;
}
