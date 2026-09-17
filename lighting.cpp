/* ==========================================================================
   lighting.cpp  -  OpenGL lighting for day / sunset / night / rain / fog
   --------------------------------------------------------------------------
   GL_LIGHT0            : the sun (day) or the moon (night) - directional.
   GL_LIGHT1..GL_LIGHT4 : the four floodlight towers - positional spotlights,
                          switched on only in night / rain / fog modes.
   ========================================================================== */
#include "lighting.h"

/* Four towers at the "corners" of the circular ground. */
const float FLOOD_TOWER_ANGLE[4] = { 45.0f, 135.0f, 225.0f, 315.0f };
const float FLOOD_TOWER_R        = 96.0f;
const float FLOOD_TOWER_H        = 40.0f;

void lightingInit()
{
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    /* Specular highlights are computed from the real eye position instead of
       assuming an infinitely distant viewer - looks much better on the ball. */
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,     GL_TRUE);

    /* glColor*() now drives ambient + diffuse material, which keeps the
       drawing code short; specular/shininess are still set explicitly. */
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    glEnable(GL_NORMALIZE);   /* keep normals unit-length after glScalef()  */
}

/* Convenience: full material description for one object. */
void setMaterial(float r, float g, float b, float specular, float shininess)
{
    GLfloat spec[4] = { specular, specular, specular, 1.0f };
    glColor3f(r, g, b);                       /* -> ambient + diffuse       */
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec);
    glMaterialf (GL_FRONT_AND_BACK, GL_SHININESS, shininess);
}

void setEmissive(float r, float g, float b)
{
    GLfloat e[4] = { r, g, b, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, e);
}

void clearEmissive()
{
    GLfloat e[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, e);
}

/* Sun / moon direction and colour, plus the floodlights.
   Must be called AFTER the camera view matrix is loaded, so that the light
   positions are transformed by the same modelview matrix as the geometry. */
void lightingUpdate()
{
    if (!gLightingOn) { glDisable(GL_LIGHTING); return; }
    glEnable(GL_LIGHTING);

    GLfloat amb[4], dif[4], spc[4], pos[4];

    switch (gWeather) {
    case W_SUNNY:                                   /* high midday sun     */
        amb[0]=0.35f; amb[1]=0.35f; amb[2]=0.38f; amb[3]=1.0f;
        dif[0]=1.00f; dif[1]=0.97f; dif[2]=0.88f; dif[3]=1.0f;
        spc[0]=1.00f; spc[1]=1.00f; spc[2]=0.95f; spc[3]=1.0f;
        pos[0]=0.35f; pos[1]=1.00f; pos[2]=0.30f; pos[3]=0.0f;
        break;
    case W_SUNSET:                                  /* low warm sun        */
        amb[0]=0.30f; amb[1]=0.22f; amb[2]=0.20f; amb[3]=1.0f;
        dif[0]=1.00f; dif[1]=0.55f; dif[2]=0.25f; dif[3]=1.0f;
        spc[0]=1.00f; spc[1]=0.70f; spc[2]=0.40f; spc[3]=1.0f;
        pos[0]=-1.00f; pos[1]=0.16f; pos[2]=0.20f; pos[3]=0.0f;
        break;
    case W_NIGHT:                                   /* pale moonlight      */
        amb[0]=0.12f; amb[1]=0.13f; amb[2]=0.20f; amb[3]=1.0f;
        dif[0]=0.22f; dif[1]=0.24f; dif[2]=0.38f; dif[3]=1.0f;
        spc[0]=0.35f; spc[1]=0.35f; spc[2]=0.50f; spc[3]=1.0f;
        pos[0]=-0.30f; pos[1]=0.90f; pos[2]=-0.40f; pos[3]=0.0f;
        break;
    case W_RAIN:                                    /* flat overcast       */
        amb[0]=0.28f; amb[1]=0.29f; amb[2]=0.32f; amb[3]=1.0f;
        dif[0]=0.48f; dif[1]=0.50f; dif[2]=0.55f; dif[3]=1.0f;
        spc[0]=0.30f; spc[1]=0.30f; spc[2]=0.35f; spc[3]=1.0f;
        pos[0]=0.20f; pos[1]=1.00f; pos[2]=0.10f; pos[3]=0.0f;
        break;
    default:                                        /* W_FOG - soft haze   */
        amb[0]=0.42f; amb[1]=0.42f; amb[2]=0.44f; amb[3]=1.0f;
        dif[0]=0.55f; dif[1]=0.55f; dif[2]=0.58f; dif[3]=1.0f;
        spc[0]=0.20f; spc[1]=0.20f; spc[2]=0.22f; spc[3]=1.0f;
        pos[0]=0.10f; pos[1]=1.00f; pos[2]=0.25f; pos[3]=0.0f;
        break;
    }

    glLightfv(GL_LIGHT0, GL_AMBIENT,  amb);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  dif);
    glLightfv(GL_LIGHT0, GL_SPECULAR, spc);
    glLightfv(GL_LIGHT0, GL_POSITION, pos);   /* w = 0 -> directional light */

    /* --- Floodlights: positional spotlights aimed at the pitch ---------- */
    bool floodsOn = (gWeather == W_NIGHT || gWeather == W_RAIN || gWeather == W_FOG);

    for (int i = 0; i < 4; ++i) {
        GLenum L = GL_LIGHT1 + i;
        if (!floodsOn) { glDisable(L); continue; }

        float a  = FLOOD_TOWER_ANGLE[i] * DEG2RAD;
        float lx = FLOOD_TOWER_R * cosf(a);
        float lz = FLOOD_TOWER_R * sinf(a);

        GLfloat lpos[4] = { lx, FLOOD_TOWER_H, lz, 1.0f };   /* w = 1 */
        GLfloat ldir[3] = { -lx, -FLOOD_TOWER_H, -lz };      /* aim at origin */
        GLfloat la[4]   = { 0.02f, 0.02f, 0.02f, 1.0f };
        GLfloat ld[4]   = { 0.95f, 0.95f, 0.88f, 1.0f };
        GLfloat ls[4]   = { 1.00f, 1.00f, 0.95f, 1.0f };

        glEnable(L);
        glLightfv(L, GL_POSITION,       lpos);
        glLightfv(L, GL_AMBIENT,        la);
        glLightfv(L, GL_DIFFUSE,        ld);
        glLightfv(L, GL_SPECULAR,       ls);
        glLightfv(L, GL_SPOT_DIRECTION, ldir);
        glLightf (L, GL_SPOT_CUTOFF,    52.0f);
        glLightf (L, GL_SPOT_EXPONENT,  6.0f);
        /* Distance attenuation: 1 / (kc + kl*d + kq*d^2) */
        glLightf (L, GL_CONSTANT_ATTENUATION,  1.0f);
        glLightf (L, GL_LINEAR_ATTENUATION,    0.004f);
        glLightf (L, GL_QUADRATIC_ATTENUATION, 0.00004f);
    }
}

/* Background colour + GL fog for the current weather mode. */
void applyWeather()
{
    GLfloat fogCol[4];
    float   density = 0.0f;
    bool    useFog  = true;

    switch (gWeather) {
    case W_SUNNY:
        glClearColor(0.45f, 0.70f, 0.95f, 1.0f);
        fogCol[0]=0.55f; fogCol[1]=0.72f; fogCol[2]=0.92f;
        density = 0.0015f;
        break;
    case W_SUNSET:
        glClearColor(0.92f, 0.48f, 0.24f, 1.0f);
        fogCol[0]=0.90f; fogCol[1]=0.52f; fogCol[2]=0.30f;
        density = 0.0045f;
        break;
    case W_NIGHT:
        glClearColor(0.03f, 0.04f, 0.10f, 1.0f);
        fogCol[0]=0.04f; fogCol[1]=0.05f; fogCol[2]=0.12f;
        density = 0.0035f;
        break;
    case W_RAIN:
        glClearColor(0.34f, 0.37f, 0.42f, 1.0f);
        fogCol[0]=0.36f; fogCol[1]=0.39f; fogCol[2]=0.44f;
        density = 0.0090f;
        break;
    default: /* W_FOG */
        glClearColor(0.78f, 0.79f, 0.80f, 1.0f);
        fogCol[0]=0.78f; fogCol[1]=0.79f; fogCol[2]=0.80f;
        density = 0.0180f;
        break;
    }

    fogCol[3] = 1.0f;
    if (useFog) {
        glEnable(GL_FOG);
        glFogi (GL_FOG_MODE,    GL_EXP2);   /* f = e^-(density*z)^2        */
        glFogfv(GL_FOG_COLOR,   fogCol);
        glFogf (GL_FOG_DENSITY, density);
        glHint (GL_FOG_HINT,    GL_NICEST);
    } else {
        glDisable(GL_FOG);
    }
}

const char* weatherName(int mode)
{
    switch (mode) {
    case W_SUNNY:  return "SUNNY (Day)";
    case W_SUNSET: return "SUNSET";
    case W_NIGHT:  return "NIGHT (Floodlights)";
    case W_RAIN:   return "RAIN";
    case W_FOG:    return "FOG";
    }
    return "?";
}
