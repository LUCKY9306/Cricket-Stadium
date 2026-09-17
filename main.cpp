/* ==========================================================================
   3D CRICKET STADIUM SIMULATION
   Computer Graphics & Multimedia (RTU B.Tech IV Semester) mini project
   C++ + OpenGL (FreeGLUT).  No game engine, no external libraries.
   --------------------------------------------------------------------------
   main.cpp  -  window creation, render loop, HUD and the animation timer.

   Demonstrated CGM concepts (see README.md for the detailed write-up):
     * Translation, rotation, scaling        -> stadium.cpp, animation.cpp
     * Perspective & orthographic projection -> camera.cpp
     * Camera / view transformation          -> camera.cpp (gluLookAt)
     * Lighting & material properties        -> lighting.cpp
     * Texture mapping                       -> texture.cpp
     * Hierarchical modelling                -> drawPlayer() in stadium.cpp
     * Timer based animation                 -> timerCallback() below
     * Double buffering and depth testing    -> initGL() below
   ========================================================================== */
#include "common.h"
#include "camera.h"
#include "stadium.h"
#include "lighting.h"
#include "texture.h"
#include "animation.h"
#include "input.h"

/* ---- Global state (declared extern in common.h) ------------------------ */
int        gWinW = 1280, gWinH = 720;
int        gWeather = W_SUNNY;
bool       gShowHUD = true;
bool       gWireframe = false;
bool       gLightingOn = true;
float      gTimeSec = 0.0f;
float      gDeltaSec = 0.0f;
ScoreState gScore;

static int   lastMillis = 0;
static float fps = 0.0f;
static int   frameCount = 0;
static float fpsTimer = 0.0f;

/* ------------------------------------------------------------------ */
/* 2D HUD (orthographic overlay)                                       */
/* ------------------------------------------------------------------ */
static void hudText(float x, float y, const char* s, void* font)
{
    glRasterPos2f(x, y);
    for (const char* p = s; *p; ++p)
        glutBitmapCharacter(font, *p);
}

static void drawHUD()
{
    if (!gShowHUD) return;

    /* Switch to a 2D orthographic projection in pixel coordinates - the
       textbook way of overlaying a HUD on a 3D scene. */
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0.0, (double)gWinW, 0.0, (double)gWinH);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_FOG);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    /* translucent panel behind the text */
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, 0.45f);
    glBegin(GL_QUADS);
      glVertex2f(8.0f,   (float)gWinH - 150.0f);
      glVertex2f(430.0f, (float)gWinH - 150.0f);
      glVertex2f(430.0f, (float)gWinH - 8.0f);
      glVertex2f(8.0f,   (float)gWinH - 8.0f);
    glEnd();

    char buf[160];
    float y = (float)gWinH - 30.0f;

    glColor3f(0.35f, 1.0f, 0.55f);
    hudText(20.0f, y, "3D CRICKET STADIUM SIMULATION - RTU CGM", GLUT_BITMAP_9_BY_15);
    y -= 20.0f;

    glColor3f(1.0f, 0.92f, 0.45f);
    sprintf(buf, "SCORE  %d/%d   OVERS %d.%d   TARGET %d",
            gScore.runs, gScore.wickets, gScore.overs, gScore.balls, gScore.target);
    hudText(20.0f, y, buf, GLUT_BITMAP_9_BY_15);
    y -= 18.0f;

    glColor3f(0.85f, 0.90f, 1.00f);
    sprintf(buf, "Weather: %s   Projection: %s   %s",
            weatherName(gWeather),
            gOrtho ? "ORTHOGRAPHIC" : "PERSPECTIVE",
            gBirdsEye ? "[BIRD'S EYE]" : (gTour ? "[AUTO TOUR]" : ""));
    hudText(20.0f, y, buf, GLUT_BITMAP_8_BY_13);
    y -= 16.0f;

    sprintf(buf, "Cam (%.1f, %.1f, %.1f)  FOV %.0f  Light:%s Tex:%s  FPS %.0f",
            gCam.eye.x, gCam.eye.y, gCam.eye.z, gCam.fov,
            gLightingOn ? "ON" : "OFF", gTexturesOn ? "ON" : "OFF", fps);
    hudText(20.0f, y, buf, GLUT_BITMAP_8_BY_13);
    y -= 16.0f;

    glColor3f(0.75f, 0.80f, 0.85f);
    hudText(20.0f, y, "WASD move  Q/E up-down  Arrows look  Drag/M mouse-look",
            GLUT_BITMAP_8_BY_13);
    y -= 14.0f;
    hudText(20.0f, y, "SPACE bowl  B bird  O tour  P proj  N day/night  C weather  F1-F5",
            GLUT_BITMAP_8_BY_13);
    y -= 14.0f;
    hudText(20.0f, y, "1-5 views  +/- zoom  L light  T texture  X wire  V hud  ESC quit",
            GLUT_BITMAP_8_BY_13);

    /* Big centre banner right after a boundary */
    if (gPhase == BP_RESULT && (strcmp(gScore.event, "SIX!") == 0 ||
                                strcmp(gScore.event, "FOUR!") == 0 ||
                                strcmp(gScore.event, "WICKET!") == 0)) {
        float blink = 0.55f + 0.45f * sinf(gTimeSec * 10.0f);
        glColor3f(blink, blink * 0.85f, 0.2f);
        int w = glutBitmapLength(GLUT_BITMAP_TIMES_ROMAN_24,
                                 (const unsigned char*)gScore.event);
        hudText((gWinW - w) * 0.5f, gWinH * 0.72f, gScore.event,
                GLUT_BITMAP_TIMES_ROMAN_24);
    }

    glPopAttrib();
    glPopMatrix();                   /* modelview  */
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();                   /* projection */
    glMatrixMode(GL_MODELVIEW);
}

/* ------------------------------------------------------------------ */
/* GLUT callbacks                                                      */
/* ------------------------------------------------------------------ */
static void display()
{
    applyWeather();                              /* clear colour + fog     */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPolygonMode(GL_FRONT_AND_BACK, gWireframe ? GL_LINE : GL_FILL);

    cameraApplyProjection();                     /* PROJECTION matrix      */
    cameraApplyView();                           /* VIEW matrix            */
    lightingUpdate();                            /* lights in eye space    */

    drawScene();                                 /* MODEL transformations  */

    drawHUD();

    glutSwapBuffers();                           /* DOUBLE BUFFERING       */
}

static void reshape(int w, int h)
{
    if (h == 0) h = 1;
    gWinW = w; gWinH = h;
    glViewport(0, 0, w, h);                      /* viewport transformation */
    cameraApplyProjection();
}

/* TIMER BASED ANIMATION: re-registers itself for a steady ~60 Hz update. */
static void timerCallback(int value)
{
    (void)value;

    int   now = glutGet(GLUT_ELAPSED_TIME);
    float dt  = (now - lastMillis) / 1000.0f;
    lastMillis = now;
    if (dt <= 0.0f) dt = 0.001f;
    if (dt > 0.25f) dt = 0.25f;

    gDeltaSec = dt;
    gTimeSec += dt;

    /* fps counter */
    frameCount++;
    fpsTimer += dt;
    if (fpsTimer >= 0.5f) {
        fps = frameCount / fpsTimer;
        frameCount = 0;
        fpsTimer = 0.0f;
    }

    inputUpdate(dt);
    cameraUpdate(dt);
    animUpdate(dt);

    glutPostRedisplay();
    glutTimerFunc(16, timerCallback, 0);         /* ~60 frames per second  */
}

/* ------------------------------------------------------------------ */
static void initGL()
{
    glEnable(GL_DEPTH_TEST);                     /* HIDDEN SURFACE REMOVAL */
    glDepthFunc(GL_LEQUAL);
    glShadeModel(GL_SMOOTH);                     /* Gouraud shading        */
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    lightingInit();
    loadAllTextures();
    stadiumInit();
    cameraInit();
    animInit();
    inputInit();
    applyWeather();
}

static void printControls()
{
    printf("\n=== 3D CRICKET STADIUM SIMULATION (RTU CGM Project) ===\n");
    printf(" W A S D      move camera        Q / E      rise / descend\n");
    printf(" Arrows       look around        PgUp/PgDn  zoom in / out\n");
    printf(" Mouse drag   look around        M          toggle mouse look\n");
    printf(" Wheel, +/-   zoom               1..5       preset viewpoints\n");
    printf(" B            bird's-eye view    O          automatic tour\n");
    printf(" P            perspective/ortho  SPACE      bowl a delivery\n");
    printf(" N            day / night        C          cycle weather\n");
    printf(" F1..F5       sunny/sunset/night/rain/fog\n");
    printf(" L light  T texture  X wireframe  V HUD  K autoplay  I reset\n");
    printf(" ESC          quit\n\n");
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    /* GLUT_DOUBLE = double buffering, GLUT_DEPTH = a z-buffer */
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_MULTISAMPLE);
    glutInitWindowSize(gWinW, gWinH);
    glutInitWindowPosition(60, 40);
    glutCreateWindow("3D Cricket Stadium Simulation - OpenGL / FreeGLUT (RTU CGM)");

    initGL();
    printControls();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(onKeyDown);
    glutKeyboardUpFunc(onKeyUp);
    glutSpecialFunc(onSpecialDown);
    glutSpecialUpFunc(onSpecialUp);
    glutMouseFunc(onMouseButton);
    glutMotionFunc(onMouseMotion);
    glutPassiveMotionFunc(onMousePassive);
#ifdef FREEGLUT
    glutMouseWheelFunc(onMouseWheel);
#endif

    lastMillis = glutGet(GLUT_ELAPSED_TIME);
    glutTimerFunc(16, timerCallback, 0);

    glutMainLoop();
    return 0;
}
