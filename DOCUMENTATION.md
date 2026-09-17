# CGM Concepts Used — Technical Documentation

Project: **3D Cricket Stadium Simulation** (C++ / OpenGL / FreeGLUT)
Course: Computer Graphics & Multimedia, RTU B.Tech IV Semester

---

## 1. Geometric (modelling) transformations

| Transformation | OpenGL call | Where to find it |
|---|---|---|
| Translation | `glTranslatef` | `drawWicket()`, bowler run-up in `drawPlayers()`, clouds, floodlight towers |
| Rotation | `glRotatef` | Bowling arm (`gBowlerArm`), bat swing (`gBatSwing`), spinning ball, lamp heads tilted at the pitch |
| Scaling | `glScalef` | Flattened cloud spheres, crouching wicket-keeper (`glScalef(1,0.78,1)`), bat blade from a unit cube |

OpenGL post-multiplies matrices, so the **last** transformation written is the
first applied to the vertices. Example from the bat:

```cpp
glTranslatef(0, 0.30f, 0);       // then move it to the hand
glRotatef(batAngle, 1, 0, 0);    // first rotate the bat about the wrist
```

`glEnable(GL_NORMALIZE)` is switched on in `lightingInit()` because
`glScalef` would otherwise leave non-unit normals and break the lighting.

---

## 2. Hierarchical modelling

Implemented in `drawPlayer()` (stadium.cpp). The body is a tree:

```
player
 ├── legs      (rotate about the hip)
 └── torso
      ├── head / helmet
      ├── left arm   → forearm
      └── right arm  → forearm → BAT
```

Each level is wrapped in `glPushMatrix()` / `glPopMatrix()`, so the current
transformation matrix of a parent is inherited by its children. Rotating the
shoulder automatically carries the forearm and the bat with it — that is the
whole point of hierarchical (articulated) modelling. The matrix stack acts as
a save/restore mechanism so a child's transform never leaks to its sibling.

---

## 3. Projection transformations

Implemented in `cameraApplyProjection()` (camera.cpp), toggled with **P**.

- **Perspective**: `gluPerspective(fov, aspect, 0.1, 800)` — a viewing
  frustum; distant objects shrink, parallel lines converge. Changing `fov`
  is how the zoom works.
- **Orthographic**: `glOrtho(-w, w, -h, h, 0.1, 800)` — a rectangular
  parallelepiped view volume, no foreshortening. Useful to show the examiner
  the difference: in ortho mode the far stands are the same size as the near
  ones.

`glViewport()` in `reshape()` performs the final window (viewport)
transformation, and the aspect ratio is recomputed so the scene never
stretches when the window is resized.

---

## 4. Camera / viewing transformation

`cameraApplyView()` builds the view matrix with `gluLookAt(eye, centre, up)`.

The look direction is stored as yaw/pitch angles and converted to a Cartesian
forward vector (spherical → rectangular):

```
fx = cos(pitch)·cos(yaw)
fy = sin(pitch)
fz = cos(pitch)·sin(yaw)
```

Pitch is clamped to ±89° to avoid gimbal flip. Three viewing modes exist:

1. **Free-fly** — WASD + mouse look.
2. **Bird's-eye** — camera at (0, 150, 0) looking straight down; the up
   vector is set to +Z because +Y would be degenerate.
3. **Automatic tour** — `cameraUpdate()` sweeps the eye around a circular
   path whose radius and height are modulated by sine waves, always aiming
   back at the pitch with `atan2`.

---

## 5. Lighting and material properties

`lighting.cpp`. The Phong-style OpenGL lighting model is
`colour = emission + ambient + diffuse·(N·L) + specular·(R·V)^shininess`.

| Light | Type | Purpose |
|---|---|---|
| `GL_LIGHT0` | Directional (`w = 0`) | Sun by day, moon at night; colour and direction change per weather mode |
| `GL_LIGHT1..4` | Positional spotlights (`w = 1`) | The four floodlight towers, enabled in night/rain/fog |

Spotlights use `GL_SPOT_CUTOFF` (52°), `GL_SPOT_EXPONENT` (beam falloff) and
constant/linear/quadratic **attenuation** `1/(kc + kl·d + kq·d²)`.

Materials are set by `setMaterial(r, g, b, specular, shininess)`:

- `glEnable(GL_COLOR_MATERIAL)` maps `glColor3f` to ambient + diffuse.
- `glMaterialfv(..., GL_SPECULAR, ...)` and `GL_SHININESS` are explicit — the
  cricket ball uses specular 0.95 / shininess 110 for shiny new leather, while
  the grass uses 0.05 / 6 so it stays matte.
- `setEmissive()` makes the floodlight bulbs and the scoreboard text glow
  without them acting as real light sources.

`GL_LIGHT_MODEL_LOCAL_VIEWER` is enabled so specular highlights are computed
from the true eye position. `glShadeModel(GL_SMOOTH)` gives Gouraud shading
(colour interpolated across each polygon).

---

## 6. Texture mapping

`texture.cpp` builds **nine** textures procedurally as RGB byte arrays and
uploads them with `glTexImage2D`:

grass · pitch · seats · crowd · scoreboard panel · ad hoardings · concrete ·
sky gradient · wood.

Key parameters:

- `GL_TEXTURE_WRAP_S/T = GL_REPEAT` — one 128×128 grass tile is repeated 14×
  across the whole outfield by giving texture coordinates > 1.0.
- `GL_LINEAR` min/mag filter — bilinear interpolation between texels.
- `GL_MODULATE` texture environment — the texel colour is multiplied by the
  lit surface colour, so textures still darken correctly at night.

Texture coordinates are supplied per vertex with `glTexCoord2f`, e.g. in
`drawBox()` and in the triangle fan of `drawGround()`.

---

## 7. Timer-based animation

`timerCallback()` in main.cpp is registered with `glutTimerFunc(16, …)` and
re-registers itself, giving a steady ~60 Hz update. Every callback:

1. measures the elapsed time `dt` with `glutGet(GLUT_ELAPSED_TIME)`,
2. updates input, camera and animation state using `dt`,
3. calls `glutPostRedisplay()`.

Because every motion is multiplied by `dt`, the animation runs at the same
real-world speed on a fast or a slow machine. `glutIdleFunc` was deliberately
**not** used: it burns 100% CPU and gives an unbounded frame rate.

The delivery itself is a **finite state machine** in `animUpdate()`:

```
READY → RUNUP → DELIVERY → TRAVEL → STRUCK → RESULT → READY
```

- RUNUP uses linear interpolation of the bowler's z position.
- DELIVERY rotates the arm through 360° over 0.55 s and releases the ball at
  62% of the swing.
- TRAVEL and STRUCK integrate simple projectile physics
  (`v.y -= g·dt`, `p += v·dt`) with a damped bounce (`v.y = -v.y · 0.55`).
- RESULT updates the scoreboard, triggers fireworks on a six, and resets.

**Particle systems**: fireworks (320 particles with a spherical velocity
distribution, gravity, drag and a fading life) and rain (1400 recycled drops
drawn as `GL_LINES`).

---

## 8. Double buffering and depth testing

- `glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_MULTISAMPLE)`
  requests a back buffer, a z-buffer and multisample anti-aliasing.
- Each frame is drawn into the back buffer and shown with `glutSwapBuffers()`,
  which eliminates flicker and tearing.
- `glEnable(GL_DEPTH_TEST)` with `glDepthFunc(GL_LEQUAL)` performs
  hidden-surface removal per fragment (the z-buffer algorithm).
- `glDepthMask(GL_FALSE)` is used for the sky dome, clouds, fireworks and the
  floodlight beams so that transparent/background geometry is depth-tested but
  does not write depth — the standard way of rendering blended objects.
- The pitch is drawn 2 cm above the grass to avoid **z-fighting** between two
  coplanar surfaces.

---

## 9. Other graphics techniques present

| Technique | Where |
|---|---|
| Atmospheric fog (`GL_EXP2`) | `applyWeather()` — density varies per weather mode |
| Alpha blending | Clouds, rain, fireworks, ball shadow, HUD panel |
| Additive blending (`GL_SRC_ALPHA, GL_ONE`) | Fireworks and floodlight cones |
| Quadric surfaces (`gluCylinder`, `gluDisk`, `gluSphere`) | Stumps, limbs, towers, shadows |
| Parametric surfaces | Sky dome built from rings of `GL_QUAD_STRIP` |
| Vector (stroke) text in 3D | Scoreboard and hoardings via `glutStrokeCharacter` |
| Bitmap text in a 2D ortho overlay | HUD via `glutBitmapCharacter` |
| Fake soft shadow | Blended `gluDisk` under the ball, scaled by its height |
| Wireframe rendering | `glPolygonMode(GL_FRONT_AND_BACK, GL_LINE)` — key **X** |

---

## 10. Likely viva questions

**Q. Why `glPushMatrix`/`glPopMatrix`?**
They save and restore the current transformation matrix on a stack so a
child object's transform does not affect its parent or siblings — essential
for hierarchical models like the players.

**Q. Difference between `gluPerspective` and `glOrtho`?**
`gluPerspective` defines a frustum with perspective foreshortening (realistic
depth); `glOrtho` defines a box, preserving parallel lines and sizes (used for
CAD-style views and, here, for the 2D HUD).

**Q. Why is `glLightfv(GL_LIGHT0, GL_POSITION, …)` called after `gluLookAt`?**
Light positions are transformed by the current modelview matrix. Setting them
after the view matrix places them in world space; setting them before would
fix them to the camera.

**Q. What does `w` in the light position mean?**
`w = 0` → a directional light (the sun, rays parallel, position ignored).
`w = 1` → a positional light (the floodlights, with attenuation).

**Q. What is double buffering?**
Rendering into an off-screen back buffer and swapping it with the front
buffer in one operation, so the user never sees a partially drawn frame.

**Q. What is z-fighting and how did you fix it?**
Two coplanar surfaces produce flickering because their depth values are
almost identical. Fixed here by offsetting the pitch and the painted creases
slightly above the grass.

**Q. Which shading model is used?**
Gouraud shading (`GL_SMOOTH`): lighting is evaluated per vertex and the
resulting colours are interpolated across the polygon.

**Q. How is the animation made frame-rate independent?**
All state changes are multiplied by the measured frame time `dt`, so an
object moving at 30 units/second covers the same distance per second on any
machine.
