/* ==========================================================================
   texture.h  -  Procedural texture generation & binding helpers
   --------------------------------------------------------------------------
   CGM concept: TEXTURE MAPPING.
   All textures are generated in code (no external image files) so that the
   project compiles and runs anywhere without extra assets.
   ========================================================================== */
#ifndef TEXTURE_H
#define TEXTURE_H

#include "common.h"

extern GLuint texGrass;     /* outfield grass with mower stripes            */
extern GLuint texPitch;     /* dry brown pitch strip                        */
extern GLuint texSeat;      /* coloured stadium seating                     */
extern GLuint texCrowd;     /* speckled crowd for the upper tiers           */
extern GLuint texBoard;     /* dark scoreboard panel                        */
extern GLuint texAd;        /* advertisement hoarding stripes               */
extern GLuint texConcrete;  /* pavilion / tower concrete                    */
extern GLuint texSky;       /* sky gradient used on the sky dome            */
extern GLuint texWood;      /* bat / dugout wood                            */

void  loadAllTextures();                 /* build every texture once at init */
void  texEnable(GLuint id);              /* bind + enable GL_TEXTURE_2D      */
void  texDisable();                      /* disable texturing                */
extern bool gTexturesOn;                 /* toggled with the 'T' key         */

#endif /* TEXTURE_H */
