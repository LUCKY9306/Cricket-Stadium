/* ==========================================================================
   stadium.h  -  All stadium geometry
   --------------------------------------------------------------------------
   CGM concepts: MODELLING TRANSFORMATIONS (translate / rotate / scale),
                 HIERARCHICAL MODELLING (players are built with nested
                 glPushMatrix()/glPopMatrix() blocks).
   ========================================================================== */
#ifndef STADIUM_H
#define STADIUM_H

#include "common.h"

void stadiumInit();        /* creates the GLU quadric used by the meshes    */
void drawScene();          /* draws the entire stadium for one frame        */

/* Individual parts - exposed so they can be demonstrated one by one.       */
void drawSkyDome();
void drawGround();
void drawPitch();
void drawWicket(float x, float z);
void drawStands();
void drawRoofAndFlags();
void drawPavilion();
void drawDugouts();
void drawScoreboard();
void drawFloodlights();
void drawAdBoards();
void drawClouds();
void drawPlayers();
void drawBall();

/* Small helpers reused by other modules */
void drawBox(float w, float h, float d);
void strokeText3D(const char* s, float scale, float thickness);
float strokeTextWidth(const char* s, float scale);

#endif /* STADIUM_H */
