/* ==========================================================================
   lighting.h  -  Lights, materials, fog and sky colour per weather mode
   --------------------------------------------------------------------------
   CGM concepts: LIGHTING MODEL (ambient / diffuse / specular),
                 MATERIAL PROPERTIES, ATMOSPHERIC FOG.
   ========================================================================== */
#ifndef LIGHTING_H
#define LIGHTING_H

#include "common.h"

void lightingInit();                 /* one-time GL light state setup       */
void lightingUpdate();               /* per-frame: sun/moon + floodlights   */
void applyWeather();                 /* clear colour + fog for gWeather     */
void setMaterial(float r, float g, float b,
                 float specular, float shininess);
void setEmissive(float r, float g, float b);   /* glowing objects  */
void clearEmissive();

/* Positions of the four floodlight towers, shared with stadium.cpp so the
   lamp geometry and the actual GL lights always agree. */
extern const float FLOOD_TOWER_ANGLE[4];
extern const float FLOOD_TOWER_R;
extern const float FLOOD_TOWER_H;

#endif /* LIGHTING_H */
