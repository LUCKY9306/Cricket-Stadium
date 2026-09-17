/* ==========================================================================
   animation.cpp  -  Bowling sequence, ball physics and particle effects
   ========================================================================== */
#include "animation.h"
#include "lighting.h"

int   gPhase        = BP_READY;
Vec3  gBallPos      = { 0.0f, 1.6f, -PITCH_HALF };
Vec3  gBallVel      = { 0.0f, 0.0f, 0.0f };
bool  gBallVisible  = true;

float gBowlerZ      = -PITCH_HALF - 12.0f;
float gBowlerArm    = 0.0f;
float gBatSwing     = 0.0f;
float gBatsmanLean  = 0.0f;

float gCrowdPhase   = 0.0f;
float gCrowdExcite  = 0.0f;
float gFlagPhase    = 0.0f;
float gCloudShift   = 0.0f;
float gAutoBowlTimer= 2.0f;
bool  gAutoPlay     = true;

static float phaseT   = 0.0f;   /* seconds spent in the current phase */
static int   outcome  = 0;      /* 0 dot, 1 single, 4 four, 6 six, 9 wicket */
static bool  bounced  = false;
static bool  scored   = false;

/* ------------------------------------------------------------------ */
/* Particle systems                                                    */
/* ------------------------------------------------------------------ */
struct Particle {
    Vec3  p, v;
    float life, r, g, b;
};

static const int NFIRE = 320;
static Particle  fire[NFIRE];
static bool      fireActive = false;
static float     fireTimer  = 0.0f;

static const int NRAIN = 1400;
static Particle  rain[NRAIN];

static float frand() { return (float)rand() / (float)RAND_MAX; }

static void spawnFireworks()
{
    fireActive = true;
    fireTimer  = 0.0f;
    for (int i = 0; i < NFIRE; ++i) {
        /* three bursts at different places above the stands */
        int   b  = i % 3;
        float bx = (b == 0) ? -55.0f : (b == 1) ? 0.0f : 55.0f;
        float by = 40.0f + b * 6.0f;
        float bz = (b == 1) ? -70.0f : 30.0f;

        /* random point on a sphere -> spherical explosion */
        float th = frand() * 2.0f * PI;
        float ph = acosf(2.0f * frand() - 1.0f);
        float sp = 9.0f + frand() * 11.0f;

        fire[i].p = vec3(bx, by, bz);
        fire[i].v = vec3(sp * sinf(ph) * cosf(th),
                         sp * cosf(ph),
                         sp * sinf(ph) * sinf(th));
        fire[i].life = 1.4f + frand() * 1.1f;
        fire[i].r = 0.6f + frand() * 0.4f;
        fire[i].g = 0.3f + frand() * 0.7f;
        fire[i].b = 0.2f + frand() * 0.8f;
    }
}

static void initRain()
{
    for (int i = 0; i < NRAIN; ++i) {
        rain[i].p = vec3((frand() - 0.5f) * 220.0f,
                         frand() * 70.0f,
                         (frand() - 0.5f) * 220.0f);
        rain[i].v = vec3(-2.5f, -(38.0f + frand() * 18.0f), 1.5f);
        rain[i].life = 1.0f;
    }
}

void animInit()
{
    srand(2024);
    initRain();
    animResetMatch();
}

void animResetMatch()
{
    gScore.runs    = 0;
    gScore.wickets = 0;
    gScore.balls   = 0;
    gScore.overs   = 0;
    gScore.target  = 186;
    strcpy(gScore.event, "MATCH START");
    gPhase       = BP_READY;
    phaseT       = 0.0f;
    gBowlerZ     = -PITCH_HALF - 12.0f;
    gBowlerArm   = 0.0f;
    gBatSwing    = 0.0f;
    gBallVisible = true;
    gBallPos     = vec3(0.0f, 1.6f, -PITCH_HALF);
}

void animBowl()
{
    if (gPhase != BP_READY) return;
    gPhase   = BP_RUNUP;
    phaseT   = 0.0f;
    bounced  = false;
    scored   = false;

    /* Pick the outcome of this delivery up front (weighted random). */
    float r = frand();
    if      (r < 0.34f) outcome = 6;
    else if (r < 0.56f) outcome = 4;
    else if (r < 0.76f) outcome = 1;
    else if (r < 0.92f) outcome = 0;
    else                outcome = 9;      /* wicket */
}

/* Register the runs / wicket on the scoreboard once per delivery. */
static void applyOutcome()
{
    if (scored) return;
    scored = true;

    if (outcome == 9) {
        gScore.wickets++;
        strcpy(gScore.event, "WICKET!");
    } else {
        gScore.runs += outcome;
        if      (outcome == 6) strcpy(gScore.event, "SIX!");
        else if (outcome == 4) strcpy(gScore.event, "FOUR!");
        else if (outcome == 1) strcpy(gScore.event, "1 RUN");
        else                   strcpy(gScore.event, "DOT BALL");
    }

    gScore.balls++;
    if (gScore.balls >= 6) { gScore.balls = 0; gScore.overs++; }

    if (outcome == 6) { spawnFireworks(); gCrowdExcite = 1.0f; }
    if (outcome == 4) { gCrowdExcite = 0.75f; }
    if (gScore.wickets >= 10) animResetMatch();
}

/* ------------------------------------------------------------------ */
/* Main per-frame update                                               */
/* ------------------------------------------------------------------ */
void animUpdate(float dt)
{
    if (dt > 0.1f) dt = 0.1f;             /* guard against long stalls */

    /* --- ambient / looping animations -------------------------------- */
    gCrowdPhase  += dt * (1.4f + 3.0f * gCrowdExcite);
    gFlagPhase   += dt * 3.2f;
    gCloudShift  += dt * 0.55f;
    if (gCloudShift > 400.0f) gCloudShift -= 400.0f;
    gCrowdExcite -= dt * 0.22f;
    if (gCrowdExcite < 0.0f) gCrowdExcite = 0.0f;

    /* --- rain particles ---------------------------------------------- */
    if (gWeather == W_RAIN) {
        for (int i = 0; i < NRAIN; ++i) {
            rain[i].p.x += rain[i].v.x * dt;
            rain[i].p.y += rain[i].v.y * dt;
            rain[i].p.z += rain[i].v.z * dt;
            if (rain[i].p.y < 0.0f) {                 /* recycle the drop */
                rain[i].p.x = (frand() - 0.5f) * 220.0f;
                rain[i].p.y = 60.0f + frand() * 20.0f;
                rain[i].p.z = (frand() - 0.5f) * 220.0f;
            }
        }
    }

    /* --- fireworks ---------------------------------------------------- */
    if (fireActive) {
        fireTimer += dt;
        bool alive = false;
        for (int i = 0; i < NFIRE; ++i) {
            if (fire[i].life <= 0.0f) continue;
            alive = true;
            fire[i].life -= dt;
            fire[i].v.y  -= 6.5f * dt;                /* gravity          */
            fire[i].v.x  *= 0.985f;                   /* air drag         */
            fire[i].v.z  *= 0.985f;
            fire[i].p.x  += fire[i].v.x * dt;
            fire[i].p.y  += fire[i].v.y * dt;
            fire[i].p.z  += fire[i].v.z * dt;
        }
        if (!alive || fireTimer > 4.0f) fireActive = false;
    }

    /* --- delivery state machine --------------------------------------- */
    phaseT += dt;

    switch (gPhase) {

    case BP_READY:
        gBowlerZ   = -PITCH_HALF - 12.0f;
        gBowlerArm = 0.0f;
        gBatSwing  += (0.0f - gBatSwing) * 6.0f * dt;      /* ease back   */
        gBatsmanLean += (0.0f - gBatsmanLean) * 6.0f * dt;
        gBallPos   = vec3(0.0f, 1.6f, gBowlerZ + 0.6f);
        gBallVisible = true;
        if (gAutoPlay) {
            gAutoBowlTimer -= dt;
            if (gAutoBowlTimer <= 0.0f) { gAutoBowlTimer = 6.5f; animBowl(); }
        }
        break;

    case BP_RUNUP: {
        /* linear interpolation of the bowler's position (TRANSLATION) */
        float t = clampf(phaseT / 1.30f, 0.0f, 1.0f);
        gBowlerZ = (-PITCH_HALF - 12.0f) + t * 10.6f;
        gBallPos = vec3(0.45f, 1.5f + 0.25f * sinf(phaseT * 18.0f), gBowlerZ + 0.5f);
        if (t >= 1.0f) { gPhase = BP_DELIVERY; phaseT = 0.0f; }
        break;
    }

    case BP_DELIVERY: {
        /* the bowling arm sweeps a full circle (ROTATION) */
        float t = clampf(phaseT / 0.55f, 0.0f, 1.0f);
        gBowlerArm = t * 360.0f;
        if (t < 0.62f) {
            /* ball still in the hand - follow the hand position */
            float a = (gBowlerArm - 90.0f) * DEG2RAD;
            gBallPos = vec3(0.45f + 0.15f * cosf(a),
                            1.55f + 1.15f * (1.0f + cosf(a)) * 0.5f + 0.4f,
                            gBowlerZ + 0.35f + 0.9f * sinf(a) * 0.3f);
        } else {
            /* RELEASE: give the ball its initial velocity */
            gBallVel = vec3((frand() - 0.5f) * 0.9f, 1.2f, 30.0f);
            gPhase   = BP_TRAVEL;
            phaseT   = 0.0f;
            bounced  = false;
        }
        break;
    }

    case BP_TRAVEL: {
        gBallVel.y -= 9.81f * dt;                       /* gravity        */
        gBallPos.x += gBallVel.x * dt;
        gBallPos.y += gBallVel.y * dt;
        gBallPos.z += gBallVel.z * dt;

        /* pitch bounce, with a little seam deviation */
        if (gBallPos.y <= BALL_R && !bounced) {
            gBallPos.y = BALL_R;
            gBallVel.y = -gBallVel.y * 0.55f;
            gBallVel.x += (frand() - 0.5f) * 1.5f;
            bounced = true;
        }

        gBowlerArm += dt * 220.0f;                      /* follow through */
        if (gBowlerArm > 360.0f) gBowlerArm = 360.0f;

        /* the batsman plays as the ball arrives */
        if (gBallPos.z > PITCH_HALF - 2.2f) {
            gPhase = BP_STRUCK;
            phaseT = 0.0f;

            if (outcome == 9) {                         /* bowled         */
                gBallVel = vec3(0.6f, 1.5f, 6.0f);
            } else if (outcome == 6) {                  /* lofted six     */
                float dir = (frand() - 0.5f) * 1.6f;
                gBallVel = vec3(26.0f * dir, 27.0f, 30.0f);
            } else if (outcome == 4) {                  /* along the deck */
                float dir = (frand() - 0.5f) * 2.2f;
                gBallVel = vec3(30.0f * dir, 6.0f, 34.0f);
            } else if (outcome == 1) {
                gBallVel = vec3(9.0f, 3.0f, 11.0f);
            } else {                                    /* defended       */
                gBallVel = vec3(1.5f, 1.0f, 3.0f);
            }
        }
        break;
    }

    case BP_STRUCK: {
        /* BAT SWING: fast down-swing then a slow return */
        float t = clampf(phaseT / 0.42f, 0.0f, 1.0f);
        gBatSwing    = 150.0f * sinf(t * PI);
        gBatsmanLean = 18.0f  * sinf(t * PI);

        gBallVel.y -= 9.81f * dt;
        gBallPos.x += gBallVel.x * dt;
        gBallPos.y += gBallVel.y * dt;
        gBallPos.z += gBallVel.z * dt;

        if (gBallPos.y < BALL_R) {                     /* bounce in field */
            gBallPos.y = BALL_R;
            gBallVel.y = -gBallVel.y * 0.45f;
            gBallVel.x *= 0.8f;
            gBallVel.z *= 0.8f;
        }

        float d = sqrtf(gBallPos.x * gBallPos.x + gBallPos.z * gBallPos.z);
        if (d > BOUNDARY_R || phaseT > 2.6f) {
            applyOutcome();
            gPhase = BP_RESULT;
            phaseT = 0.0f;
        }
        break;
    }

    case BP_RESULT:
        applyOutcome();
        gBallVel.y -= 9.81f * dt;
        gBallPos.x += gBallVel.x * dt;
        gBallPos.y += gBallVel.y * dt;
        gBallPos.z += gBallVel.z * dt;
        if (gBallPos.y < BALL_R) { gBallPos.y = BALL_R; gBallVel = vec3(0,0,0); }
        gBatSwing    += (0.0f - gBatSwing)    * 4.0f * dt;
        gBatsmanLean += (0.0f - gBatsmanLean) * 4.0f * dt;
        if (phaseT > 2.4f) {
            gPhase   = BP_READY;
            phaseT   = 0.0f;
            gBowlerArm = 0.0f;
            gAutoBowlTimer = 2.2f;
        }
        break;
    }
}

/* ------------------------------------------------------------------ */
/* Particle rendering                                                  */
/* ------------------------------------------------------------------ */
void drawFireworks()
{
    if (!fireActive) return;

    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glDepthMask(GL_FALSE);                 /* additive sparks, no z-writes */
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glPointSize(3.5f);

    glBegin(GL_POINTS);
    for (int i = 0; i < NFIRE; ++i) {
        if (fire[i].life <= 0.0f) continue;
        float a = clampf(fire[i].life / 2.0f, 0.0f, 1.0f);
        glColor4f(fire[i].r, fire[i].g, fire[i].b, a);
        glVertex3f(fire[i].p.x, fire[i].p.y, fire[i].p.z);
    }
    glEnd();

    glDepthMask(GL_TRUE);
    glPopAttrib();
}

void drawRain()
{
    if (gWeather != W_RAIN) return;

    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_LINE_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glLineWidth(1.0f);
    glColor4f(0.72f, 0.80f, 0.92f, 0.55f);

    glBegin(GL_LINES);
    for (int i = 0; i < NRAIN; ++i) {
        glVertex3f(rain[i].p.x, rain[i].p.y, rain[i].p.z);
        glVertex3f(rain[i].p.x + 0.06f, rain[i].p.y - 1.1f, rain[i].p.z);
    }
    glEnd();

    glPopAttrib();
}
