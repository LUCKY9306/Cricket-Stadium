/* ==========================================================================
   texture.cpp  -  Procedural textures for the stadium
   --------------------------------------------------------------------------
   Every texture is a small RGB byte array filled by code and uploaded with
   glTexImage2D().  Bilinear filtering (GL_LINEAR) is used so no GLU mipmap
   helper is required, and GL_REPEAT lets one small tile cover a huge surface.
   ========================================================================== */
#include "texture.h"

GLuint texGrass = 0, texPitch = 0, texSeat = 0, texCrowd = 0;
GLuint texBoard = 0, texAd = 0, texConcrete = 0, texSky = 0, texWood = 0;
bool   gTexturesOn = true;

/* Small deterministic pseudo-random generator so the textures look the same
   every run (handy while demonstrating the project in a viva). */
static unsigned int seed = 12345u;
static float rnd()
{
    seed = seed * 1103515245u + 12345u;
    return (float)((seed >> 16) & 0x7fff) / 32767.0f;
}

/* Upload an RGB byte buffer as a 2D texture object and return its id. */
static GLuint makeTexture(int w, int h, unsigned char* data)
{
    GLuint id = 0;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);
    return id;
}

/* --- 1. Outfield grass: green noise + alternating mower stripes ---------- */
static GLuint buildGrass()
{
    const int N = 128;
    static unsigned char px[128 * 128 * 3];
    for (int y = 0; y < N; ++y) {
        for (int x = 0; x < N; ++x) {
            int   i     = (y * N + x) * 3;
            float noise = rnd() * 0.18f;
            /* a stripe every 16 texels gives the mown-outfield look */
            float band  = ((x / 16) % 2) ? 0.10f : 0.00f;
            float g     = 0.42f + band + noise;
            px[i + 0] = (unsigned char)(255 * (0.10f + band * 0.4f + noise * 0.3f));
            px[i + 1] = (unsigned char)(255 * clampf(g, 0.0f, 1.0f));
            px[i + 2] = (unsigned char)(255 * (0.12f + band * 0.3f + noise * 0.2f));
        }
    }
    return makeTexture(N, N, px);
}

/* --- 2. Pitch: dry compacted clay with faint crease-coloured speckle ----- */
static GLuint buildPitch()
{
    const int N = 64;
    static unsigned char px[64 * 64 * 3];
    for (int y = 0; y < N; ++y) {
        for (int x = 0; x < N; ++x) {
            int   i = (y * N + x) * 3;
            float n = rnd() * 0.15f;
            px[i + 0] = (unsigned char)(255 * clampf(0.70f + n, 0.f, 1.f));
            px[i + 1] = (unsigned char)(255 * clampf(0.60f + n, 0.f, 1.f));
            px[i + 2] = (unsigned char)(255 * clampf(0.42f + n, 0.f, 1.f));
        }
    }
    return makeTexture(N, N, px);
}

/* --- 3. Seats: rows of blue/orange bucket seats with dark gaps ----------- */
static GLuint buildSeat()
{
    const int N = 64;
    static unsigned char px[64 * 64 * 3];
    for (int y = 0; y < N; ++y) {
        for (int x = 0; x < N; ++x) {
            int i  = (y * N + x) * 3;
            int cx = x % 8, cy = y % 8;
            bool gap = (cx < 1 || cy < 1);
            if (gap) {                                   /* aisle / shadow */
                px[i] = 25; px[i + 1] = 25; px[i + 2] = 30;
            } else if (((y / 8) % 3) == 0) {             /* orange band    */
                px[i] = 225; px[i + 1] = 110; px[i + 2] = 30;
            } else {                                     /* blue seats     */
                px[i] = 30; px[i + 1] = 70; px[i + 2] = 175;
            }
        }
    }
    return makeTexture(N, N, px);
}

/* --- 4. Crowd: dense multicoloured speckle for the far tiers ------------- */
static GLuint buildCrowd()
{
    const int N = 64;
    static unsigned char px[64 * 64 * 3];
    for (int y = 0; y < N; ++y) {
        for (int x = 0; x < N; ++x) {
            int i = (y * N + x) * 3;
            px[i + 0] = (unsigned char)(60 + rnd() * 195);
            px[i + 1] = (unsigned char)(50 + rnd() * 180);
            px[i + 2] = (unsigned char)(50 + rnd() * 190);
        }
    }
    return makeTexture(N, N, px);
}

/* --- 5. Scoreboard panel: dark matrix board with faint pixel grid -------- */
static GLuint buildBoard()
{
    const int N = 64;
    static unsigned char px[64 * 64 * 3];
    for (int y = 0; y < N; ++y) {
        for (int x = 0; x < N; ++x) {
            int i = (y * N + x) * 3;
            bool dot = ((x % 4) == 0 || (y % 4) == 0);
            unsigned char v = dot ? 26 : 14;
            px[i] = v; px[i + 1] = v; px[i + 2] = (unsigned char)(v + 8);
        }
    }
    return makeTexture(N, N, px);
}

/* --- 6. Advertisement hoarding: bright horizontal colour bands ----------- */
static GLuint buildAd()
{
    const int N = 64;
    static unsigned char px[64 * 64 * 3];
    for (int y = 0; y < N; ++y) {
        for (int x = 0; x < N; ++x) {
            int i    = (y * N + x) * 3;
            int band = (x / 16) % 4;
            unsigned char r = 255, g = 255, b = 255;
            if (band == 0) { r = 210; g = 30;  b = 45;  }
            if (band == 1) { r = 245; g = 245; b = 245; }
            if (band == 2) { r = 20;  g = 90;  b = 190; }
            if (band == 3) { r = 250; g = 190; b = 25;  }
            if (y < 3 || y > N - 4) { r = g = b = 20; }   /* dark border */
            px[i] = r; px[i + 1] = g; px[i + 2] = b;
        }
    }
    return makeTexture(N, N, px);
}

/* --- 7. Concrete: light grey noise with panel joints --------------------- */
static GLuint buildConcrete()
{
    const int N = 64;
    static unsigned char px[64 * 64 * 3];
    for (int y = 0; y < N; ++y) {
        for (int x = 0; x < N; ++x) {
            int   i = (y * N + x) * 3;
            float n = 0.72f + rnd() * 0.12f;
            if (x % 32 == 0 || y % 32 == 0) n *= 0.80f;   /* joint line */
            unsigned char v = (unsigned char)(255 * clampf(n, 0.f, 1.f));
            px[i] = v; px[i + 1] = v; px[i + 2] = (unsigned char)(v * 0.98f);
        }
    }
    return makeTexture(N, N, px);
}

/* --- 8. Sky: vertical blue gradient (v = 0 horizon -> v = 1 zenith) ------ */
static GLuint buildSky()
{
    const int N = 64;
    static unsigned char px[64 * 64 * 3];
    for (int y = 0; y < N; ++y) {
        float t = (float)y / (N - 1);
        unsigned char r = (unsigned char)(255 * (0.62f - 0.45f * t));
        unsigned char g = (unsigned char)(255 * (0.78f - 0.38f * t));
        unsigned char b = (unsigned char)(255 * (0.95f - 0.15f * t));
        for (int x = 0; x < N; ++x) {
            int i = (y * N + x) * 3;
            px[i] = r; px[i + 1] = g; px[i + 2] = b;
        }
    }
    return makeTexture(N, N, px);
}

/* --- 9. Wood: willow grain for the bat and dugout benches ---------------- */
static GLuint buildWood()
{
    const int N = 64;
    static unsigned char px[64 * 64 * 3];
    for (int y = 0; y < N; ++y) {
        for (int x = 0; x < N; ++x) {
            int   i = (y * N + x) * 3;
            float g = 0.80f + 0.12f * sinf(x * 0.9f) + rnd() * 0.06f;
            px[i + 0] = (unsigned char)(255 * clampf(g,          0.f, 1.f));
            px[i + 1] = (unsigned char)(255 * clampf(g * 0.86f,  0.f, 1.f));
            px[i + 2] = (unsigned char)(255 * clampf(g * 0.62f,  0.f, 1.f));
        }
    }
    return makeTexture(N, N, px);
}

void loadAllTextures()
{
    texGrass    = buildGrass();
    texPitch    = buildPitch();
    texSeat     = buildSeat();
    texCrowd    = buildCrowd();
    texBoard    = buildBoard();
    texAd       = buildAd();
    texConcrete = buildConcrete();
    texSky      = buildSky();
    texWood     = buildWood();

    /* GL_MODULATE multiplies the texel by the lit surface colour, so the
       textures still respond to the day/night lighting. */
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

void texEnable(GLuint id)
{
    if (!gTexturesOn) return;
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, id);
}

void texDisable()
{
    glDisable(GL_TEXTURE_2D);
}
