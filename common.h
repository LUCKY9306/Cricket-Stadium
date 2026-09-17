/* ==========================================================================
   common.h  -  Shared declarations for the 3D Cricket Stadium Simulation
   RTU B.Tech IV Sem - Computer Graphics & Multimedia Project
   --------------------------------------------------------------------------
   Holds the platform-specific GL includes, world dimensions, and the global
   simulation state that every module reads from / writes to.
   ========================================================================== */
#ifndef COMMON_H
#define COMMON_H

/* ---- Platform specific OpenGL / GLUT headers --------------------------- */
#ifdef __APPLE__
  #include <GLUT/glut.h>
  #include <OpenGL/glu.h>
  #include <OpenGL/gl.h>
#else
  #include <GL/freeglut.h>
  #include <GL/glu.h>
  #include <GL/gl.h>
#endif

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifndef M_PI
  #define M_PI 3.14159265358979323846
#endif

const float PI      = 3.14159265358979323846f;
const float DEG2RAD = PI / 180.0f;
const float RAD2DEG = 180.0f / PI;

/* ---- Simple 3D vector (kept tiny on purpose - no external maths lib) ---- */
struct Vec3 {
    float x, y, z;
};
inline Vec3 vec3(float x, float y, float z) { Vec3 v = {x, y, z}; return v; }
inline float clampf(float v, float a, float b) { return v < a ? a : (v > b ? b : v); }

/* ---- Stadium world dimensions (metres, roughly to real scale) ---------- */
const float BOUNDARY_R    = 60.0f;   /* boundary rope radius               */
const float STAND_INNER_R = 66.0f;   /* where the spectator stands begin   */
const float STAND_OUTER_R = 90.0f;   /* outer wall of the stands           */
const float STAND_HEIGHT  = 20.0f;   /* height of the top tier             */
const float GROUND_R      = 130.0f;  /* outer ground / sky disc            */
const float PITCH_LEN     = 20.12f;  /* 22 yards                           */
const float PITCH_HALF    = PITCH_LEN * 0.5f;
const float PITCH_W       = 3.05f;
const float STUMP_H       = 0.71f;
const float BALL_R        = 0.36f;   /* exaggerated so it stays visible    */

/* ---- Weather / time-of-day modes --------------------------------------- */
enum WeatherMode {
    W_SUNNY = 0,
    W_SUNSET,
    W_NIGHT,
    W_RAIN,
    W_FOG,
    W_COUNT
};

/* ---- Scoreboard state --------------------------------------------------- */
struct ScoreState {
    int  runs;
    int  wickets;
    int  balls;       /* legal balls bowled in the current over (0..5)      */
    int  overs;       /* completed overs                                    */
    int  target;
    char event[32];   /* last event banner: "SIX!", "FOUR", "DOT BALL" ...  */
};

/* ---- Globals (defined in main.cpp) -------------------------------------- */
extern int         gWinW, gWinH;
extern int         gWeather;      /* one of WeatherMode                     */
extern bool        gShowHUD;
extern bool        gWireframe;
extern bool        gLightingOn;
extern float       gTimeSec;      /* seconds since the program started      */
extern float       gDeltaSec;     /* seconds since the previous frame       */
extern ScoreState  gScore;

/* Night is a property of the weather mode, so every module agrees on it. */
inline bool isNight() { return gWeather == W_NIGHT; }
const char* weatherName(int mode);

#endif /* COMMON_H */
