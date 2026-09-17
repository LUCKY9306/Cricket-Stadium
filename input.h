/* ==========================================================================
   input.h  -  Keyboard and mouse handling
   --------------------------------------------------------------------------
   Keys are latched into a state array by the down/up callbacks and consumed
   once per frame by inputUpdate(), which gives smooth movement instead of
   the jerky motion you get from the key-repeat rate.
   ========================================================================== */
#ifndef INPUT_H
#define INPUT_H

#include "common.h"

void inputInit();
void inputUpdate(float dt);           /* apply held keys to the camera      */

/* GLUT callbacks - registered in main.cpp */
void onKeyDown(unsigned char key, int x, int y);
void onKeyUp(unsigned char key, int x, int y);
void onSpecialDown(int key, int x, int y);
void onSpecialUp(int key, int x, int y);
void onMouseButton(int button, int state, int x, int y);
void onMouseMotion(int x, int y);
void onMousePassive(int x, int y);
void onMouseWheel(int wheel, int dir, int x, int y);

#endif /* INPUT_H */
