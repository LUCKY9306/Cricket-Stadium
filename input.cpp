/* ==========================================================================
   input.cpp  -  Keyboard / mouse control of the simulation
   ========================================================================== */
#include "input.h"
#include "camera.h"
#include "animation.h"
#include "texture.h"
#include "lighting.h"

static bool keyDown[256];
static bool spDown[256];        /* GLUT special keys (arrows, F-keys)       */
static bool dragging = false;
static int  lastX = 0, lastY = 0;

void inputInit()
{
    memset(keyDown, 0, sizeof(keyDown));
    memset(spDown,  0, sizeof(spDown));
}

/* Called once per timer tick: converts held keys into camera motion. */
void inputUpdate(float dt)
{
    float speed = 22.0f * dt;
    if (keyDown['z'] || keyDown['Z']) speed *= 3.0f;     /* Z = sprint */

    float fwd = 0.0f, strafe = 0.0f, up = 0.0f;
    if (keyDown['w'] || keyDown['W']) fwd    += speed;
    if (keyDown['s'] || keyDown['S']) fwd    -= speed;
    if (keyDown['d'] || keyDown['D']) strafe += speed;
    if (keyDown['a'] || keyDown['A']) strafe -= speed;
    if (keyDown['q'] || keyDown['Q']) up     += speed * 0.7f;
    if (keyDown['e'] || keyDown['E']) up     -= speed * 0.7f;

    if (fwd || strafe || up) cameraMove(fwd, strafe, up);

    /* arrow keys look around */
    float look = 70.0f * dt;
    if (spDown[GLUT_KEY_LEFT])  cameraLook(-look, 0.0f);
    if (spDown[GLUT_KEY_RIGHT]) cameraLook( look, 0.0f);
    if (spDown[GLUT_KEY_UP])    cameraLook(0.0f,  look);
    if (spDown[GLUT_KEY_DOWN])  cameraLook(0.0f, -look);

    /* page up / down change the field of view (zoom) */
    if (spDown[GLUT_KEY_PAGE_UP])   cameraZoom(-40.0f * dt);
    if (spDown[GLUT_KEY_PAGE_DOWN]) cameraZoom( 40.0f * dt);
}

void onKeyDown(unsigned char key, int x, int y)
{
    (void)x; (void)y;
    keyDown[key] = true;

    switch (key) {
    case 27:                                   /* ESC - quit              */
#ifdef FREEGLUT
        glutLeaveMainLoop();
#else
        exit(0);
#endif
        break;

    case ' ':  animBowl();                     break;   /* bowl a delivery */
    case 'k': case 'K': gAutoPlay = !gAutoPlay; break;
    case 'i': case 'I': animResetMatch();      break;

    case 'b': case 'B':                        /* bird's-eye view         */
        gBirdsEye = !gBirdsEye;
        if (gBirdsEye) gTour = false;
        break;

    case 'o': case 'O':                        /* automatic stadium tour  */
        gTour = !gTour;
        if (gTour) gBirdsEye = false;
        break;

    case 'p': case 'P':                        /* perspective <-> ortho   */
        gOrtho = !gOrtho;
        break;

    case 'm': case 'M':                        /* mouse look on/off       */
        gMouseLook = !gMouseLook;
        glutSetCursor(gMouseLook ? GLUT_CURSOR_NONE : GLUT_CURSOR_INHERIT);
        if (gMouseLook) {
            lastX = gWinW / 2; lastY = gWinH / 2;
            glutWarpPointer(lastX, lastY);
        }
        break;

    case 'n': case 'N':                        /* day <-> night           */
        gWeather = (gWeather == W_NIGHT) ? W_SUNNY : W_NIGHT;
        break;

    case 'c': case 'C':                        /* cycle all weather modes */
        gWeather = (gWeather + 1) % W_COUNT;
        break;

    case 'l': case 'L': gLightingOn = !gLightingOn; break;
    case 't': case 'T': gTexturesOn = !gTexturesOn; break;
    case 'x': case 'X': gWireframe  = !gWireframe;  break;
    case 'v': case 'V': gShowHUD    = !gShowHUD;    break;

    case '+': case '=': cameraZoom(-4.0f); break;
    case '-': case '_': cameraZoom( 4.0f); break;

    case '1': case '2': case '3': case '4': case '5':
        cameraPreset(key - '0');
        break;
    }
}

void onKeyUp(unsigned char key, int x, int y)
{
    (void)x; (void)y;
    keyDown[key] = false;
}

void onSpecialDown(int key, int x, int y)
{
    (void)x; (void)y;
    if (key >= 0 && key < 256) spDown[key] = true;

    /* F1..F5 select the weather / time of day directly */
    switch (key) {
    case GLUT_KEY_F1: gWeather = W_SUNNY;  break;
    case GLUT_KEY_F2: gWeather = W_SUNSET; break;
    case GLUT_KEY_F3: gWeather = W_NIGHT;  break;
    case GLUT_KEY_F4: gWeather = W_RAIN;   break;
    case GLUT_KEY_F5: gWeather = W_FOG;    break;
    }
}

void onSpecialUp(int key, int x, int y)
{
    (void)x; (void)y;
    if (key >= 0 && key < 256) spDown[key] = false;
}

void onMouseButton(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON) {
        dragging = (state == GLUT_DOWN);
        lastX = x; lastY = y;
    }
    /* Classic GLUT reports the wheel as buttons 3 (up) and 4 (down). */
    if (state == GLUT_DOWN) {
        if (button == 3) cameraZoom(-3.0f);
        if (button == 4) cameraZoom( 3.0f);
    }
}

/* Drag with the left button held: look around. */
void onMouseMotion(int x, int y)
{
    if (!dragging) return;
    float dx = (float)(x - lastX);
    float dy = (float)(y - lastY);
    lastX = x; lastY = y;
    cameraLook(dx * 0.22f, -dy * 0.22f);
}

/* Free mouse-look: the pointer is re-centred every frame. */
void onMousePassive(int x, int y)
{
    if (!gMouseLook) return;
    int cx = gWinW / 2, cy = gWinH / 2;
    if (x == cx && y == cy) return;              /* our own warp event */
    cameraLook((float)(x - cx) * 0.14f, -(float)(y - cy) * 0.14f);
    glutWarpPointer(cx, cy);
}

void onMouseWheel(int wheel, int dir, int x, int y)
{
    (void)wheel; (void)x; (void)y;
    cameraZoom(dir > 0 ? -3.0f : 3.0f);
}
