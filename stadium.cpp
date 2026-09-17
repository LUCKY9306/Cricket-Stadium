/* ==========================================================================
   stadium.cpp  -  Geometry of the whole cricket stadium
   --------------------------------------------------------------------------
   Layout convention used everywhere in this project:
       +Z is "down the pitch" towards the batsman,
       the bowler's end   is at z = -PITCH_HALF,
       the batsman's end  is at z = +PITCH_HALF,
       the scoreboard sits behind the bowler's end (-Z),
       the pavilion sits on the -X side.
   ========================================================================== */
#include "stadium.h"
#include "texture.h"
#include "lighting.h"
#include "animation.h"

static GLUquadric* quad = 0;

void stadiumInit()
{
    quad = gluNewQuadric();
    gluQuadricNormals(quad, GLU_SMOOTH);
    gluQuadricTexture(quad, GL_TRUE);
}

/* ------------------------------------------------------------------ */
/* Generic primitives                                                  */
/* ------------------------------------------------------------------ */

/* Axis aligned box centred on the origin, with normals and tex coords. */
void drawBox(float w, float h, float d)
{
    float x = w * 0.5f, y = h * 0.5f, z = d * 0.5f;
    glBegin(GL_QUADS);
      /* front (+Z) */
      glNormal3f(0, 0, 1);
      glTexCoord2f(0,0); glVertex3f(-x,-y, z);
      glTexCoord2f(1,0); glVertex3f( x,-y, z);
      glTexCoord2f(1,1); glVertex3f( x, y, z);
      glTexCoord2f(0,1); glVertex3f(-x, y, z);
      /* back (-Z) */
      glNormal3f(0, 0, -1);
      glTexCoord2f(0,0); glVertex3f( x,-y,-z);
      glTexCoord2f(1,0); glVertex3f(-x,-y,-z);
      glTexCoord2f(1,1); glVertex3f(-x, y,-z);
      glTexCoord2f(0,1); glVertex3f( x, y,-z);
      /* right (+X) */
      glNormal3f(1, 0, 0);
      glTexCoord2f(0,0); glVertex3f( x,-y, z);
      glTexCoord2f(1,0); glVertex3f( x,-y,-z);
      glTexCoord2f(1,1); glVertex3f( x, y,-z);
      glTexCoord2f(0,1); glVertex3f( x, y, z);
      /* left (-X) */
      glNormal3f(-1, 0, 0);
      glTexCoord2f(0,0); glVertex3f(-x,-y,-z);
      glTexCoord2f(1,0); glVertex3f(-x,-y, z);
      glTexCoord2f(1,1); glVertex3f(-x, y, z);
      glTexCoord2f(0,1); glVertex3f(-x, y,-z);
      /* top (+Y) */
      glNormal3f(0, 1, 0);
      glTexCoord2f(0,0); glVertex3f(-x, y, z);
      glTexCoord2f(1,0); glVertex3f( x, y, z);
      glTexCoord2f(1,1); glVertex3f( x, y,-z);
      glTexCoord2f(0,1); glVertex3f(-x, y,-z);
      /* bottom (-Y) */
      glNormal3f(0, -1, 0);
      glTexCoord2f(0,0); glVertex3f(-x,-y,-z);
      glTexCoord2f(1,0); glVertex3f( x,-y,-z);
      glTexCoord2f(1,1); glVertex3f( x,-y, z);
      glTexCoord2f(0,1); glVertex3f(-x,-y, z);
    glEnd();
}

/* Cylinder growing along +Y (GLU builds them along +Z, so we rotate). */
static void cylinderY(float rBottom, float rTop, float h, int slices)
{
    glPushMatrix();
      glRotatef(-90.0f, 1, 0, 0);
      gluCylinder(quad, rBottom, rTop, h, slices, 1);
      gluDisk(quad, 0.0, rBottom, slices, 1);           /* bottom cap */
      glPushMatrix();
        glTranslatef(0, 0, h);
        gluDisk(quad, 0.0, rTop, slices, 1);            /* top cap    */
      glPopMatrix();
    glPopMatrix();
}

/* Vector stroke text, used on the scoreboard and the advertising boards. */
void strokeText3D(const char* s, float scale, float thickness)
{
    glLineWidth(thickness);
    glPushMatrix();
      glScalef(scale, scale, scale);
      for (const char* p = s; *p; ++p)
          glutStrokeCharacter(GLUT_STROKE_ROMAN, *p);
    glPopMatrix();
}

float strokeTextWidth(const char* s, float scale)
{
    float w = 0.0f;
    for (const char* p = s; *p; ++p)
        w += (float)glutStrokeWidth(GLUT_STROKE_ROMAN, *p);
    return w * scale;
}

/* ------------------------------------------------------------------ */
/* Sky, clouds                                                         */
/* ------------------------------------------------------------------ */
void drawSkyDome()
{
    /* The dome is drawn unlit and without depth writes so everything else
       is painted on top of it. */
    glPushAttrib(GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_FOG);
    glDepthMask(GL_FALSE);

    float tint = 1.0f;
    if (gWeather == W_NIGHT)  tint = 0.18f;
    if (gWeather == W_SUNSET) tint = 0.85f;
    if (gWeather == W_RAIN)   tint = 0.55f;
    if (gWeather == W_FOG)    tint = 0.90f;

    glColor3f(tint, tint, tint);
    if (gWeather == W_SUNSET) glColor3f(1.0f, 0.62f, 0.42f);
    if (gWeather == W_NIGHT)  glColor3f(0.12f, 0.14f, 0.28f);

    texEnable(texSky);

    const int RINGS = 12, SEG = 28;
    const float R = 330.0f;
    for (int i = 0; i < RINGS; ++i) {
        float p0 = (float)i / RINGS * (PI * 0.5f);
        float p1 = (float)(i + 1) / RINGS * (PI * 0.5f);
        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= SEG; ++j) {
            float t = (float)j / SEG * 2.0f * PI;
            for (int k = 0; k < 2; ++k) {
                float p = k ? p1 : p0;
                float x = R * cosf(p) * cosf(t);
                float y = R * sinf(p);
                float z = R * cosf(p) * sinf(t);
                glTexCoord2f((float)j / SEG * 3.0f, p / (PI * 0.5f));
                glNormal3f(-x, -y, -z);
                glVertex3f(x, y, z);
            }
        }
        glEnd();
    }

    /* Stars at night */
    if (gWeather == W_NIGHT) {
        texDisable();
        glPointSize(2.0f);
        glColor3f(1.0f, 1.0f, 0.92f);
        glBegin(GL_POINTS);
        unsigned int s = 7u;
        for (int i = 0; i < 400; ++i) {
            s = s * 1103515245u + 12345u;
            float a = (float)((s >> 8) & 0xffff) / 65535.0f * 2.0f * PI;
            s = s * 1103515245u + 12345u;
            float b = (float)((s >> 8) & 0xffff) / 65535.0f * (PI * 0.48f) + 0.05f;
            glVertex3f(300.0f * cosf(b) * cosf(a),
                       300.0f * sinf(b),
                       300.0f * cosf(b) * sinf(a));
        }
        glEnd();
    }

    texDisable();
    glDepthMask(GL_TRUE);
    glPopAttrib();
}

/* Drifting clouds - a few flattened spheres per cloud (SCALING demo). */
void drawClouds()
{
    if (gWeather == W_NIGHT) return;

    glPushAttrib(GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT | GL_CURRENT_BIT);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    clearEmissive();

    float bright = (gWeather == W_RAIN) ? 0.45f : (gWeather == W_SUNSET ? 0.95f : 1.0f);
    float alpha  = (gWeather == W_FOG) ? 0.35f : 0.85f;

    const int NC = 9;
    for (int i = 0; i < NC; ++i) {
        float baseX = -180.0f + i * 46.0f;
        float x = fmodf(baseX + gCloudShift * (1.0f + 0.1f * i) + 400.0f, 400.0f) - 200.0f;
        float y = 78.0f + (i % 3) * 12.0f;
        float z = -170.0f + ((i * 53) % 340);

        glPushMatrix();
          glTranslatef(x, y, z);                       /* TRANSLATION   */
          for (int b = 0; b < 4; ++b) {
              glPushMatrix();
                glTranslatef(b * 9.0f - 13.0f, (b % 2) * 3.0f, (b % 3) * 4.0f);
                glScalef(1.9f, 0.7f, 1.3f);            /* SCALING       */
                if (gWeather == W_SUNSET)
                    glColor4f(1.0f, 0.75f * bright, 0.68f * bright, alpha);
                else
                    glColor4f(bright, bright, bright, alpha);
                glutSolidSphere(7.0, 12, 10);
              glPopMatrix();
          }
        glPopMatrix();
    }

    glDepthMask(GL_TRUE);
    glPopAttrib();
}

/* ------------------------------------------------------------------ */
/* Ground, boundary, pitch                                             */
/* ------------------------------------------------------------------ */
void drawGround()
{
    setMaterial(1.0f, 1.0f, 1.0f, 0.05f, 6.0f);
    texEnable(texGrass);

    /* Outfield: one big triangle fan, tex coords scaled so the grass tile
       repeats many times (GL_REPEAT wrap mode). */
    const int SEG = 72;
    glBegin(GL_TRIANGLE_FAN);
      glNormal3f(0, 1, 0);
      glTexCoord2f(0.5f, 0.5f);
      glVertex3f(0.0f, 0.0f, 0.0f);
      for (int i = 0; i <= SEG; ++i) {
          float a = (float)i / SEG * 2.0f * PI;
          float x = GROUND_R * cosf(a), z = GROUND_R * sinf(a);
          glTexCoord2f(0.5f + cosf(a) * 14.0f, 0.5f + sinf(a) * 14.0f);
          glVertex3f(x, 0.0f, z);
      }
    glEnd();
    texDisable();

    /* --- boundary rope: a ring of small white cylinders ---------------- */
    setMaterial(0.95f, 0.95f, 0.95f, 0.2f, 12.0f);
    glBegin(GL_QUAD_STRIP);
      for (int i = 0; i <= SEG * 2; ++i) {
          float a = (float)i / (SEG * 2) * 2.0f * PI;
          float cx = cosf(a), cz = sinf(a);
          glNormal3f(cx, 0.4f, cz);
          glVertex3f(BOUNDARY_R * cx, 0.45f, BOUNDARY_R * cz);
          glVertex3f(BOUNDARY_R * cx, 0.02f, BOUNDARY_R * cz);
      }
    glEnd();

    /* --- 30 yard inner circle (white painted line) --------------------- */
    glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT);
      glDisable(GL_LIGHTING);
      glColor3f(0.92f, 0.94f, 0.92f);
      glLineWidth(2.0f);
      glBegin(GL_LINE_LOOP);
        for (int i = 0; i < SEG * 2; ++i) {
            float a = (float)i / (SEG * 2) * 2.0f * PI;
            glVertex3f(27.4f * cosf(a), 0.03f, 27.4f * sinf(a));
        }
      glEnd();
    glPopAttrib();
}

void drawPitch()
{
    /* The 22-yard strip, lifted 2 cm so it does not z-fight with the grass. */
    setMaterial(1.0f, 1.0f, 1.0f, 0.03f, 4.0f);
    texEnable(texPitch);
    glBegin(GL_QUADS);
      glNormal3f(0, 1, 0);
      glTexCoord2f(0, 0); glVertex3f(-PITCH_W, 0.02f, -PITCH_HALF - 2.0f);
      glTexCoord2f(2, 0); glVertex3f( PITCH_W, 0.02f, -PITCH_HALF - 2.0f);
      glTexCoord2f(2, 8); glVertex3f( PITCH_W, 0.02f,  PITCH_HALF + 2.0f);
      glTexCoord2f(0, 8); glVertex3f(-PITCH_W, 0.02f,  PITCH_HALF + 2.0f);
    glEnd();
    texDisable();

    /* --- creases (white lines painted on the pitch) -------------------- */
    glPushAttrib(GL_ENABLE_BIT);
      glDisable(GL_LIGHTING);
      glColor3f(0.96f, 0.96f, 0.94f);
      for (int e = 0; e < 2; ++e) {
          float z  = (e == 0) ? -PITCH_HALF : PITCH_HALF;
          float sg = (e == 0) ? 1.0f : -1.0f;
          glBegin(GL_QUADS);                       /* popping crease */
            glVertex3f(-PITCH_W, 0.03f, z + sg * 1.22f - 0.05f);
            glVertex3f( PITCH_W, 0.03f, z + sg * 1.22f - 0.05f);
            glVertex3f( PITCH_W, 0.03f, z + sg * 1.22f + 0.05f);
            glVertex3f(-PITCH_W, 0.03f, z + sg * 1.22f + 0.05f);
          glEnd();
          glBegin(GL_QUADS);                       /* bowling crease */
            glVertex3f(-1.32f, 0.03f, z - 0.05f);
            glVertex3f( 1.32f, 0.03f, z - 0.05f);
            glVertex3f( 1.32f, 0.03f, z + 0.05f);
            glVertex3f(-1.32f, 0.03f, z + 0.05f);
          glEnd();
          for (int s = -1; s <= 1; s += 2) {       /* return creases  */
              glBegin(GL_QUADS);
                glVertex3f(s * 1.32f - 0.05f, 0.03f, z);
                glVertex3f(s * 1.32f + 0.05f, 0.03f, z);
                glVertex3f(s * 1.32f + 0.05f, 0.03f, z + sg * 1.5f);
                glVertex3f(s * 1.32f - 0.05f, 0.03f, z + sg * 1.5f);
              glEnd();
          }
      }
    glPopAttrib();
}

/* A set of stumps + bails. Hierarchical: the whole wicket is positioned
   once, then each stump is placed relative to it. */
void drawWicket(float x, float z)
{
    glPushMatrix();
      glTranslatef(x, 0.0f, z);
      setMaterial(0.93f, 0.90f, 0.78f, 0.35f, 24.0f);
      for (int i = -1; i <= 1; ++i) {
          glPushMatrix();
            glTranslatef(i * 0.11f, 0.0f, 0.0f);
            cylinderY(0.018f, 0.018f, STUMP_H, 8);
          glPopMatrix();
      }
      /* bails */
      setMaterial(0.88f, 0.84f, 0.70f, 0.3f, 20.0f);
      for (int i = 0; i < 2; ++i) {
          glPushMatrix();
            glTranslatef(-0.055f + i * 0.11f, STUMP_H + 0.015f, 0.0f);
            glRotatef(90.0f, 0, 0, 1);
            cylinderY(0.013f, 0.013f, 0.10f, 6);
          glPopMatrix();
      }
    glPopMatrix();
}

/* ------------------------------------------------------------------ */
/* Spectator stands + animated crowd                                   */
/* ------------------------------------------------------------------ */
void drawStands()
{
    const int  SEG   = 72;
    const int  TIERS = 12;
    const float dR   = (STAND_OUTER_R - STAND_INNER_R) / TIERS;
    const float dH   = (STAND_HEIGHT  - 2.0f) / TIERS;

    setMaterial(1.0f, 1.0f, 1.0f, 0.1f, 8.0f);

    for (int t = 0; t < TIERS; ++t) {
        float r0 = STAND_INNER_R + t * dR;
        float r1 = r0 + dR;
        float h0 = 2.0f + t * dH;
        float h1 = h0 + dH;

        /* riser (vertical face) - concrete */
        texEnable(texConcrete);
        glBegin(GL_QUAD_STRIP);
        for (int i = 0; i <= SEG; ++i) {
            float a = (float)i / SEG * 2.0f * PI;
            float cx = cosf(a), cz = sinf(a);
            glNormal3f(-cx, 0, -cz);
            glTexCoord2f((float)i * 0.6f, 0.0f); glVertex3f(r0 * cx, h0, r0 * cz);
            glTexCoord2f((float)i * 0.6f, 1.0f); glVertex3f(r0 * cx, h1, r0 * cz);
        }
        glEnd();
        texDisable();

        /* tread (the seating deck) - seat texture */
        texEnable(t > 7 ? texCrowd : texSeat);
        glBegin(GL_QUAD_STRIP);
        for (int i = 0; i <= SEG; ++i) {
            float a = (float)i / SEG * 2.0f * PI;
            float cx = cosf(a), cz = sinf(a);
            glNormal3f(0, 1, 0);
            glTexCoord2f((float)i * 1.2f, 0.0f); glVertex3f(r0 * cx, h1, r0 * cz);
            glTexCoord2f((float)i * 1.2f, 1.0f); glVertex3f(r1 * cx, h1, r1 * cz);
        }
        glEnd();
        texDisable();
    }

    /* --- outer wall ---------------------------------------------------- */
    texEnable(texConcrete);
    setMaterial(0.85f, 0.85f, 0.87f, 0.08f, 6.0f);
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= SEG; ++i) {
        float a = (float)i / SEG * 2.0f * PI;
        float cx = cosf(a), cz = sinf(a);
        glNormal3f(cx, 0, cz);
        glTexCoord2f((float)i * 0.8f, 0.0f);
        glVertex3f(STAND_OUTER_R * cx, 0.0f, STAND_OUTER_R * cz);
        glTexCoord2f((float)i * 0.8f, 1.0f);
        glVertex3f(STAND_OUTER_R * cx, STAND_HEIGHT + 1.5f, STAND_OUTER_R * cz);
    }
    glEnd();
    texDisable();

    /* --- CROWD WAVE: rows of small spectators that rise in sequence ----- */
    const int  CSEG  = 56;
    const int  CROWS = 6;
    for (int row = 0; row < CROWS; ++row) {
        float rr = STAND_INNER_R + 2.0f + row * (dR * 1.9f);
        float hh = 2.0f + (row * 1.9f + 1.0f) * dH;
        for (int i = 0; i < CSEG; ++i) {
            float a = (float)i / CSEG * 2.0f * PI;

            /* skip the arc occupied by the pavilion / scoreboard */
            float deg = a * RAD2DEG;
            if (deg > 160.0f && deg < 200.0f) continue;   /* pavilion (-X) */
            if (deg > 250.0f && deg < 290.0f) continue;   /* scoreboard    */

            /* the wave is a travelling sine along the angle */
            float wave = sinf(a * 3.0f - gCrowdPhase * 2.0f);
            float lift = (wave > 0.55f) ? (wave - 0.55f) * 2.0f : 0.0f;
            lift *= (0.6f + 0.8f * gCrowdExcite);

            float cx = rr * cosf(a), cz = rr * sinf(a);
            /* cheap per-spectator colour variety */
            int   k = (i * 7 + row * 13) % 5;
            float cr = 0.85f, cg = 0.35f, cb = 0.35f;
            if (k == 1) { cr = 0.25f; cg = 0.45f; cb = 0.85f; }
            if (k == 2) { cr = 0.95f; cg = 0.85f; cb = 0.25f; }
            if (k == 3) { cr = 0.30f; cg = 0.75f; cb = 0.40f; }
            if (k == 4) { cr = 0.90f; cg = 0.90f; cb = 0.90f; }

            glPushMatrix();
              glTranslatef(cx, hh + 0.45f + lift * 0.8f, cz);
              glRotatef(-deg, 0, 1, 0);
              setMaterial(cr, cg, cb, 0.05f, 4.0f);
              glutSolidSphere(0.36, 6, 5);                    /* body     */
              glPushMatrix();
                glTranslatef(0.0f, 0.42f, 0.0f);
                setMaterial(0.55f, 0.38f, 0.26f, 0.05f, 4.0f);
                glutSolidSphere(0.20, 6, 5);                  /* head     */
              glPopMatrix();
              if (lift > 0.1f) {                              /* arms up  */
                  setMaterial(0.55f, 0.38f, 0.26f, 0.05f, 4.0f);
                  for (int s = -1; s <= 1; s += 2) {
                      glPushMatrix();
                        glTranslatef(s * 0.3f, 0.45f, 0.0f);
                        glScalef(0.09f, 0.42f, 0.09f);
                        glutSolidCube(1.0);
                      glPopMatrix();
                  }
              }
            glPopMatrix();
        }
    }
}

/* ------------------------------------------------------------------ */
/* Roof + waving flags                                                 */
/* ------------------------------------------------------------------ */
void drawRoofAndFlags()
{
    const int SEG = 72;

    /* Cantilever roof ring above the top tier. */
    setMaterial(0.55f, 0.58f, 0.62f, 0.55f, 40.0f);
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= SEG; ++i) {
        float a = (float)i / SEG * 2.0f * PI;
        float cx = cosf(a), cz = sinf(a);
        glNormal3f(0, 1, 0);
        glVertex3f((STAND_INNER_R + 8.0f) * cx, STAND_HEIGHT + 8.0f,
                   (STAND_INNER_R + 8.0f) * cz);
        glVertex3f((STAND_OUTER_R + 2.0f) * cx, STAND_HEIGHT + 10.5f,
                   (STAND_OUTER_R + 2.0f) * cz);
    }
    glEnd();

    /* Support pillars */
    setMaterial(0.45f, 0.47f, 0.50f, 0.4f, 30.0f);
    for (int i = 0; i < 24; ++i) {
        float a = (float)i / 24.0f * 2.0f * PI;
        glPushMatrix();
          glTranslatef((STAND_OUTER_R + 1.0f) * cosf(a), 0.0f,
                       (STAND_OUTER_R + 1.0f) * sinf(a));
          cylinderY(0.35f, 0.30f, STAND_HEIGHT + 10.0f, 8);
        glPopMatrix();
    }

    /* --- WAVING FLAGS on the roof rim ---------------------------------- */
    const int NFLAG = 16;
    for (int f = 0; f < NFLAG; ++f) {
        float a  = (float)f / NFLAG * 2.0f * PI;
        float px = (STAND_OUTER_R + 1.0f) * cosf(a);
        float pz = (STAND_OUTER_R + 1.0f) * sinf(a);

        glPushMatrix();
          glTranslatef(px, STAND_HEIGHT + 10.5f, pz);
          /* pole */
          setMaterial(0.80f, 0.80f, 0.82f, 0.6f, 50.0f);
          cylinderY(0.08f, 0.06f, 7.0f, 8);

          /* cloth: a quad strip whose vertices are displaced by a sine
             wave travelling along the flag -> the classic waving effect */
          glTranslatef(0.0f, 4.6f, 0.0f);
          glRotatef(-a * RAD2DEG, 0, 1, 0);
          float cr = (f % 3 == 0) ? 1.0f : (f % 3 == 1) ? 0.15f : 0.95f;
          float cg = (f % 3 == 0) ? 0.55f : (f % 3 == 1) ? 0.55f : 0.90f;
          float cb = (f % 3 == 0) ? 0.10f : (f % 3 == 1) ? 0.25f : 0.92f;
          setMaterial(cr, cg, cb, 0.15f, 10.0f);

          const int FSEG = 10;
          glBegin(GL_QUAD_STRIP);
          for (int i = 0; i <= FSEG; ++i) {
              float t  = (float)i / FSEG;
              float x  = t * 2.6f;
              float dz = 0.42f * sinf(t * 5.0f - gFlagPhase + f) * t;
              glNormal3f(0, 0, 1);
              glVertex3f(x, 0.0f,  dz);
              glVertex3f(x, 1.5f,  dz * 0.85f);
          }
          glEnd();
        glPopMatrix();
    }
}

/* ------------------------------------------------------------------ */
/* Pavilion, dugouts, scoreboard, ad boards, floodlights               */
/* ------------------------------------------------------------------ */
void drawPavilion()
{
    /* A white multi-storey pavilion on the -X side of the ground. */
    glPushMatrix();
      glTranslatef(-(STAND_INNER_R + 8.0f), 0.0f, 0.0f);

      texEnable(texConcrete);
      setMaterial(0.95f, 0.94f, 0.90f, 0.25f, 24.0f);
      glPushMatrix();
        glTranslatef(0.0f, 9.0f, 0.0f);
        drawBox(16.0f, 18.0f, 34.0f);
      glPopMatrix();
      texDisable();

      /* three balcony decks facing the ground (+X) */
      for (int i = 0; i < 3; ++i) {
          setMaterial(0.25f, 0.30f, 0.45f, 0.5f, 40.0f);
          glPushMatrix();
            glTranslatef(8.6f, 5.0f + i * 5.0f, 0.0f);
            drawBox(2.0f, 0.4f, 32.0f);
          glPopMatrix();
          /* glass front */
          setMaterial(0.35f, 0.55f, 0.70f, 0.9f, 90.0f);
          glPushMatrix();
            glTranslatef(9.4f, 6.2f, 0.0f);
            glTranslatef(0.0f, i * 5.0f, 0.0f);
            drawBox(0.2f, 2.2f, 31.0f);
          glPopMatrix();
      }

      /* roof + a small clock tower */
      setMaterial(0.55f, 0.20f, 0.18f, 0.3f, 20.0f);
      glPushMatrix();
        glTranslatef(0.0f, 18.4f, 0.0f);
        drawBox(17.5f, 1.0f, 35.5f);
      glPopMatrix();
      glPushMatrix();
        glTranslatef(0.0f, 22.0f, 0.0f);
        setMaterial(0.95f, 0.94f, 0.90f, 0.2f, 20.0f);
        drawBox(4.0f, 6.0f, 4.0f);
        glTranslatef(2.1f, 1.5f, 0.0f);
        setMaterial(1.0f, 1.0f, 1.0f, 0.8f, 60.0f);
        glRotatef(90.0f, 0, 1, 0);
        glutSolidTorus(0.12, 1.3, 8, 20);            /* clock face ring   */
      glPopMatrix();

      /* PAVILION sign */
      glPushMatrix();
        glTranslatef(8.4f, 20.0f, -6.0f);
        glRotatef(90.0f, 0, 1, 0);
        setMaterial(0.1f, 0.1f, 0.12f, 0.2f, 10.0f);
        setEmissive(0.35f, 0.32f, 0.10f);
        strokeText3D("PAVILION", 0.012f, 2.0f);
        clearEmissive();
      glPopMatrix();
    glPopMatrix();
}

void drawDugouts()
{
    /* Two team dugouts just outside the boundary, at +/-X-ish angles. */
    for (int s = 0; s < 2; ++s) {
        float a = (s == 0) ? 120.0f * DEG2RAD : 240.0f * DEG2RAD;
        glPushMatrix();
          glTranslatef((BOUNDARY_R + 3.0f) * cosf(a), 0.0f,
                       (BOUNDARY_R + 3.0f) * sinf(a));
          glRotatef(-a * RAD2DEG, 0, 1, 0);

          /* back wall + canopy */
          setMaterial(0.18f, 0.20f, 0.24f, 0.4f, 30.0f);
          glPushMatrix();
            glTranslatef(1.6f, 1.6f, 0.0f);
            drawBox(0.3f, 3.2f, 9.0f);
          glPopMatrix();
          setMaterial(0.30f, 0.34f, 0.38f, 0.5f, 40.0f);
          glPushMatrix();
            glTranslatef(0.2f, 3.3f, 0.0f);
            drawBox(3.4f, 0.25f, 9.4f);
          glPopMatrix();

          /* bench + seated reserves */
          texEnable(texWood);
          setMaterial(0.85f, 0.75f, 0.55f, 0.2f, 15.0f);
          glPushMatrix();
            glTranslatef(0.6f, 0.9f, 0.0f);
            drawBox(1.0f, 0.15f, 8.4f);
          glPopMatrix();
          texDisable();

          for (int p = 0; p < 5; ++p) {
              glPushMatrix();
                glTranslatef(0.6f, 1.35f, -3.4f + p * 1.7f);
                setMaterial(s ? 0.15f : 0.85f, s ? 0.35f : 0.75f,
                            s ? 0.75f : 0.15f, 0.05f, 4.0f);
                glutSolidSphere(0.32, 8, 6);
                glTranslatef(0.0f, 0.45f, 0.0f);
                setMaterial(0.55f, 0.38f, 0.26f, 0.05f, 4.0f);
                glutSolidSphere(0.19, 8, 6);
              glPopMatrix();
          }
        glPopMatrix();
    }
}

/* Giant LED scoreboard behind the bowler's end, updated every delivery. */
void drawScoreboard()
{
    glPushMatrix();
      glTranslatef(0.0f, 0.0f, -(STAND_INNER_R + 6.0f));

      /* support legs */
      setMaterial(0.35f, 0.36f, 0.40f, 0.4f, 30.0f);
      for (int s = -1; s <= 1; s += 2) {
          glPushMatrix();
            glTranslatef(s * 7.0f, 0.0f, 0.0f);
            cylinderY(0.7f, 0.5f, 16.0f, 10);
          glPopMatrix();
      }

      /* the panel itself */
      glPushMatrix();
        glTranslatef(0.0f, 23.0f, 0.0f);
        setMaterial(0.22f, 0.22f, 0.25f, 0.3f, 20.0f);
        drawBox(23.0f, 15.0f, 1.2f);                  /* frame */

        texEnable(texBoard);
        setMaterial(0.9f, 0.9f, 0.9f, 0.1f, 8.0f);
        glPushMatrix();
          glTranslatef(0.0f, 0.0f, 0.75f);
          drawBox(21.5f, 13.5f, 0.15f);               /* screen */
        glPopMatrix();
        texDisable();

        /* ---- live text on the board (SCOREBOARD UPDATE animation) ----- */
        char line1[64], line2[64], line3[64];
        sprintf(line1, "INDIA  %d/%d", gScore.runs, gScore.wickets);
        sprintf(line2, "OVERS %d.%d   TGT %d", gScore.overs, gScore.balls,
                gScore.target);
        sprintf(line3, "%s", gScore.event);

        glDisable(GL_LIGHTING);
        glPushMatrix();
          glTranslatef(0.0f, 0.0f, 0.95f);

          glColor3f(0.20f, 1.00f, 0.45f);
          glPushMatrix();
            glTranslatef(-strokeTextWidth(line1, 0.016f) * 0.5f, 3.4f, 0.0f);
            strokeText3D(line1, 0.016f, 3.0f);
          glPopMatrix();

          glColor3f(1.00f, 0.85f, 0.25f);
          glPushMatrix();
            glTranslatef(-strokeTextWidth(line2, 0.009f) * 0.5f, 0.2f, 0.0f);
            strokeText3D(line2, 0.009f, 2.0f);
          glPopMatrix();

          /* the event banner blinks right after a boundary */
          float blink = 0.55f + 0.45f * sinf(gTimeSec * 9.0f);
          glColor3f(blink, 0.35f * blink, 0.15f * blink);
          if (strcmp(gScore.event, "SIX!") == 0) glColor3f(blink, blink, 0.2f);
          glPushMatrix();
            glTranslatef(-strokeTextWidth(line3, 0.013f) * 0.5f, -3.6f, 0.0f);
            strokeText3D(line3, 0.013f, 3.0f);
          glPopMatrix();
        glPopMatrix();
        glEnable(GL_LIGHTING);
      glPopMatrix();
    glPopMatrix();
}

void drawAdBoards()
{
    /* Ring of advertising hoardings just outside the boundary rope. */
    const int N = 40;
    setMaterial(1.0f, 1.0f, 1.0f, 0.25f, 18.0f);
    for (int i = 0; i < N; ++i) {
        float a0 = (float)i / N * 2.0f * PI;
        float a1 = (float)(i + 1) / N * 2.0f * PI - 0.012f;
        float r  = BOUNDARY_R + 2.5f;

        texEnable(texAd);
        glBegin(GL_QUADS);
          float x0 = r * cosf(a0), z0 = r * sinf(a0);
          float x1 = r * cosf(a1), z1 = r * sinf(a1);
          float nx = cosf((a0 + a1) * 0.5f), nz = sinf((a0 + a1) * 0.5f);
          glNormal3f(-nx, 0.0f, -nz);
          glTexCoord2f(0, 0); glVertex3f(x0, 0.0f, z0);
          glTexCoord2f(1, 0); glVertex3f(x1, 0.0f, z1);
          glTexCoord2f(1, 1); glVertex3f(x1, 1.3f, z1);
          glTexCoord2f(0, 1); glVertex3f(x0, 1.3f, z0);
        glEnd();
        texDisable();
    }

    /* One board carries the project title (stroke text on a board). */
    glPushMatrix();
      glTranslatef(0.0f, 0.55f, BOUNDARY_R + 2.35f);
      glRotatef(180.0f, 0, 1, 0);
      glDisable(GL_LIGHTING);
      glColor3f(0.05f, 0.05f, 0.08f);
      glPushMatrix();
        glTranslatef(-strokeTextWidth("RTU CGM PROJECT", 0.006f) * 0.5f, -0.25f, 0.0f);
        strokeText3D("RTU CGM PROJECT", 0.006f, 2.0f);
      glPopMatrix();
      glEnable(GL_LIGHTING);
    glPopMatrix();
}

void drawFloodlights()
{
    for (int i = 0; i < 4; ++i) {
        float a = FLOOD_TOWER_ANGLE[i] * DEG2RAD;
        float x = FLOOD_TOWER_R * cosf(a);
        float z = FLOOD_TOWER_R * sinf(a);

        glPushMatrix();
          glTranslatef(x, 0.0f, z);

          /* lattice mast: four legs + cross bracing */
          setMaterial(0.45f, 0.46f, 0.50f, 0.6f, 45.0f);
          for (int c = 0; c < 4; ++c) {
              float ox = (c & 1) ? 1.4f : -1.4f;
              float oz = (c & 2) ? 1.4f : -1.4f;
              glPushMatrix();
                glTranslatef(ox, 0.0f, oz);
                cylinderY(0.30f, 0.16f, FLOOD_TOWER_H, 8);
              glPopMatrix();
          }
          for (int b = 1; b < 8; ++b) {
              glPushMatrix();
                glTranslatef(0.0f, b * (FLOOD_TOWER_H / 8.0f), 0.0f);
                drawBox(3.1f, 0.18f, 0.18f);
                drawBox(0.18f, 0.18f, 3.1f);
              glPopMatrix();
          }

          /* lamp head: a grid of bulbs tilted towards the pitch */
          glPushMatrix();
            glTranslatef(0.0f, FLOOD_TOWER_H, 0.0f);
            glRotatef(-a * RAD2DEG + 180.0f, 0, 1, 0);
            glRotatef(24.0f, 1, 0, 0);

            setMaterial(0.25f, 0.26f, 0.30f, 0.4f, 30.0f);
            drawBox(9.0f, 5.0f, 0.5f);

            bool on = (gWeather == W_NIGHT || gWeather == W_RAIN || gWeather == W_FOG);
            for (int r = 0; r < 3; ++r) {
                for (int c = 0; c < 6; ++c) {
                    glPushMatrix();
                      glTranslatef(-3.6f + c * 1.45f, -1.5f + r * 1.5f, 0.35f);
                      if (on) {
                          setEmissive(1.0f, 0.98f, 0.85f);
                          setMaterial(1.0f, 1.0f, 0.92f, 1.0f, 90.0f);
                      } else {
                          clearEmissive();
                          setMaterial(0.55f, 0.56f, 0.58f, 0.7f, 60.0f);
                      }
                      glScalef(1.0f, 1.0f, 0.4f);
                      glutSolidSphere(0.55, 10, 8);
                    glPopMatrix();
                }
            }
            clearEmissive();

            /* soft light cone (blended) so the beam is visible at night */
            if (on) {
                glPushAttrib(GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT);
                  glDisable(GL_LIGHTING);
                  glEnable(GL_BLEND);
                  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
                  glDepthMask(GL_FALSE);
                  glColor4f(1.0f, 0.98f, 0.85f, 0.055f);
                  glPushMatrix();
                    glTranslatef(0.0f, 0.0f, 1.0f);
                    gluCylinder(quad, 3.0, 40.0, 95.0, 16, 1);
                  glPopMatrix();
                  glDepthMask(GL_TRUE);
                glPopAttrib();
            }
          glPopMatrix();
        glPopMatrix();
    }
}

/* ------------------------------------------------------------------ */
/* Players - HIERARCHICAL MODELLING                                    */
/* ------------------------------------------------------------------ */
/* Body parts are nested: torso -> shoulder -> upper arm -> forearm -> bat.
   Rotating a parent automatically carries all its children with it,
   which is exactly what hierarchical modelling means.                  */
static void drawHead()
{
    setMaterial(0.72f, 0.55f, 0.40f, 0.15f, 12.0f);
    glutSolidSphere(0.17, 12, 10);
}

static void drawLimb(float len, float rad)
{
    cylinderY(rad, rad * 0.85f, len, 8);
}

static void drawPlayer(float sr, float sg, float sb,
                       float leftArm, float rightArm,
                       float legSwing, bool helmet, bool bat,
                       float batAngle)
{
    /* legs */
    setMaterial(0.92f, 0.92f, 0.94f, 0.1f, 8.0f);
    for (int s = -1; s <= 1; s += 2) {
        glPushMatrix();
          glTranslatef(s * 0.11f, 0.86f, 0.0f);
          glRotatef(s * legSwing, 1, 0, 0);
          glTranslatef(0.0f, -0.86f, 0.0f);
          glPushMatrix();
            drawLimb(0.86f, 0.075f);
          glPopMatrix();
          /* shoe */
          setMaterial(0.9f, 0.9f, 0.9f, 0.3f, 20.0f);
          glPushMatrix();
            glTranslatef(0.0f, 0.04f, 0.06f);
            glScalef(0.16f, 0.08f, 0.30f);
            glutSolidCube(1.0);
          glPopMatrix();
          setMaterial(0.92f, 0.92f, 0.94f, 0.1f, 8.0f);
        glPopMatrix();
    }

    /* torso (parent of the arms and head) */
    glPushMatrix();
      glTranslatef(0.0f, 0.86f, 0.0f);
      setMaterial(sr, sg, sb, 0.12f, 10.0f);
      glPushMatrix();
        glScalef(0.46f, 0.62f, 0.26f);
        glTranslatef(0.0f, 0.5f, 0.0f);
        glutSolidCube(1.0);
      glPopMatrix();

      /* head / helmet */
      glPushMatrix();
        glTranslatef(0.0f, 0.80f, 0.0f);
        drawHead();
        if (helmet) {
            setMaterial(0.15f, 0.16f, 0.20f, 0.7f, 60.0f);
            glPushMatrix();
              glScalef(1.15f, 1.05f, 1.15f);
              glutSolidSphere(0.18, 12, 10);
            glPopMatrix();
            setMaterial(0.80f, 0.80f, 0.82f, 0.6f, 50.0f);
            glPushMatrix();                       /* grille */
              glTranslatef(0.0f, -0.04f, 0.17f);
              glScalef(0.26f, 0.12f, 0.04f);
              glutSolidCube(1.0);
            glPopMatrix();
        }
      glPopMatrix();

      /* left arm */
      glPushMatrix();
        glTranslatef(-0.28f, 0.60f, 0.0f);
        glRotatef(leftArm, 1, 0, 0);
        setMaterial(sr * 0.95f, sg * 0.95f, sb * 0.95f, 0.12f, 10.0f);
        glPushMatrix();
          glRotatef(180.0f, 1, 0, 0);
          drawLimb(0.34f, 0.065f);               /* upper arm */
          glPushMatrix();                        /* forearm (child) */
            glTranslatef(0.0f, 0.34f, 0.0f);
            setMaterial(0.72f, 0.55f, 0.40f, 0.15f, 12.0f);
            drawLimb(0.32f, 0.058f);
          glPopMatrix();
        glPopMatrix();
      glPopMatrix();

      /* right arm - carries the ball (bowler) or the bat (batsman) */
      glPushMatrix();
        glTranslatef(0.28f, 0.60f, 0.0f);
        glRotatef(rightArm, 1, 0, 0);
        setMaterial(sr * 0.95f, sg * 0.95f, sb * 0.95f, 0.12f, 10.0f);
        glPushMatrix();
          glRotatef(180.0f, 1, 0, 0);
          drawLimb(0.34f, 0.065f);
          glPushMatrix();
            glTranslatef(0.0f, 0.34f, 0.0f);
            setMaterial(0.72f, 0.55f, 0.40f, 0.15f, 12.0f);
            drawLimb(0.32f, 0.058f);

            if (bat) {                            /* BAT is a child of the
                                                     forearm -> swings with
                                                     the whole arm chain   */
                glPushMatrix();
                  glTranslatef(0.0f, 0.30f, 0.0f);
                  glRotatef(batAngle, 1, 0, 0);
                  texEnable(texWood);
                  setMaterial(0.93f, 0.82f, 0.58f, 0.25f, 18.0f);
                  glPushMatrix();                 /* handle */
                    glTranslatef(0.0f, 0.0f, 0.0f);
                    glScalef(0.05f, 0.34f, 0.05f);
                    glTranslatef(0.0f, 0.5f, 0.0f);
                    glutSolidCube(1.0);
                  glPopMatrix();
                  glPushMatrix();                 /* blade  */
                    glTranslatef(0.0f, 0.34f, 0.0f);
                    glScalef(0.22f, 0.58f, 0.07f);
                    glTranslatef(0.0f, 0.5f, 0.0f);
                    glutSolidCube(1.0);
                  glPopMatrix();
                  texDisable();
                glPopMatrix();
            }
          glPopMatrix();
        glPopMatrix();
      glPopMatrix();
    glPopMatrix();
}

void drawPlayers()
{
    /* ---- Bowler: runs in (translation) and rotates his arm ------------ */
    glPushMatrix();
      glTranslatef(0.45f, 0.0f, gBowlerZ);
      float runSwing = (gPhase == BP_RUNUP) ? 42.0f * sinf(gTimeSec * 15.0f) : 0.0f;
      float arm = (gPhase == BP_DELIVERY || gPhase == BP_TRAVEL) ? gBowlerArm : 0.0f;
      drawPlayer(0.15f, 0.30f, 0.65f,             /* blue shirt           */
                 -arm * 0.35f, arm,
                 runSwing, false, false, 0.0f);
    glPopMatrix();

    /* ---- Striking batsman --------------------------------------------- */
    glPushMatrix();
      glTranslatef(-0.55f, 0.0f, PITCH_HALF - 1.1f);
      glRotatef(180.0f, 0, 1, 0);
      glRotatef(gBatsmanLean * 0.4f, 0, 0, 1);
      drawPlayer(0.95f, 0.93f, 0.90f,             /* whites               */
                 -20.0f - gBatSwing * 0.35f,
                 -30.0f - gBatSwing * 0.55f,
                 8.0f, true, true, 40.0f - gBatSwing * 0.8f);
    glPopMatrix();

    /* ---- Non-striker at the bowler's end ------------------------------ */
    glPushMatrix();
      glTranslatef(-1.1f, 0.0f, -PITCH_HALF + 1.4f);
      drawPlayer(0.95f, 0.93f, 0.90f, -25.0f, -25.0f, 4.0f, true, true, 55.0f);
    glPopMatrix();

    /* ---- Wicket keeper (crouching - scaled body) ----------------------- */
    glPushMatrix();
      glTranslatef(0.0f, 0.0f, PITCH_HALF + 3.2f);
      glRotatef(180.0f, 0, 1, 0);
      glScalef(1.0f, 0.78f, 1.0f);                /* SCALING to crouch    */
      drawPlayer(0.15f, 0.30f, 0.65f, -70.0f, -70.0f, 30.0f, true, false, 0.0f);
    glPopMatrix();

    /* ---- Umpires ------------------------------------------------------- */
    glPushMatrix();
      glTranslatef(1.9f, 0.0f, -PITCH_HALF - 1.6f);
      drawPlayer(0.12f, 0.12f, 0.14f, 0.0f, 0.0f, 0.0f, false, false, 0.0f);
    glPopMatrix();
    glPushMatrix();
      glTranslatef(19.0f, 0.0f, PITCH_HALF - 1.0f);
      glRotatef(-90.0f, 0, 1, 0);
      drawPlayer(0.12f, 0.12f, 0.14f, 0.0f, 0.0f, 0.0f, false, false, 0.0f);
    glPopMatrix();

    /* ---- Nine fielders spread around the ground ------------------------ */
    static const float fx[9] = {  22.0f, -24.0f,  38.0f, -40.0f,  12.0f,
                                 -14.0f,  47.0f, -48.0f,   0.0f };
    static const float fz[9] = {  18.0f,  20.0f, -14.0f, -12.0f, -26.0f,
                                  30.0f,  26.0f,  22.0f, -44.0f };
    for (int i = 0; i < 9; ++i) {
        glPushMatrix();
          glTranslatef(fx[i], 0.0f, fz[i]);
          /* every fielder faces the batsman: atan2 gives the heading */
          float ang = atan2f(fx[i] - 0.0f, fz[i] - PITCH_HALF) * RAD2DEG;
          glRotatef(-ang + 180.0f, 0, 1, 0);
          float idle = 6.0f * sinf(gTimeSec * 2.0f + i);
          drawPlayer(0.15f, 0.30f, 0.65f, idle, -idle, 0.0f, false, false, 0.0f);
        glPopMatrix();
    }
}

void drawBall()
{
    if (!gBallVisible) return;

    glPushMatrix();
      glTranslatef(gBallPos.x, gBallPos.y, gBallPos.z);
      glRotatef(gTimeSec * 720.0f, 1, 0, 0);       /* spinning seam */
      setMaterial(0.62f, 0.07f, 0.09f, 0.95f, 110.0f);  /* shiny leather */
      glutSolidSphere(BALL_R, 16, 14);

      /* white seam ring */
      setMaterial(0.95f, 0.95f, 0.92f, 0.5f, 40.0f);
      glPushMatrix();
        glRotatef(90.0f, 1, 0, 0);
        glutSolidTorus(0.022, BALL_R * 0.99, 6, 18);
      glPopMatrix();
    glPopMatrix();

    /* Ball shadow: a dark disc on the grass right under the ball. */
    glPushAttrib(GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT);
      glDisable(GL_LIGHTING);
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      float h = clampf(gBallPos.y, 0.0f, 30.0f);
      float sc = 1.0f + h * 0.08f;
      glColor4f(0.0f, 0.0f, 0.0f, clampf(0.45f - h * 0.012f, 0.0f, 0.45f));
      glPushMatrix();
        glTranslatef(gBallPos.x, 0.05f, gBallPos.z);
        glRotatef(-90.0f, 1, 0, 0);
        gluDisk(quad, 0.0, BALL_R * sc, 14, 1);
      glPopMatrix();
    glPopAttrib();
}

/* ------------------------------------------------------------------ */
/* Whole scene                                                         */
/* ------------------------------------------------------------------ */
void drawScene()
{
    drawSkyDome();
    drawClouds();

    drawGround();
    drawPitch();
    drawWicket(0.0f, -PITCH_HALF);
    drawWicket(0.0f,  PITCH_HALF);

    drawAdBoards();
    drawStands();
    drawRoofAndFlags();
    drawPavilion();
    drawDugouts();
    drawScoreboard();
    drawFloodlights();

    drawPlayers();
    drawBall();

    drawFireworks();
    drawRain();
}
