# 3D Cricket Stadium Simulation

**Computer Graphics & Multimedia (CGM) — RTU B.Tech, 4th Semester**
Written in C++ with OpenGL and FreeGLUT only. No engine, no external image
files, no third-party libraries.

---

## 1. What the program shows

A complete circular cricket stadium rendered in real time:

| Element | Details |
|---|---|
| Playing area | Textured grass outfield with mower stripes, boundary rope, 30-yard circle |
| Pitch | 22-yard strip with popping, bowling and return creases |
| Wickets | Three stumps and two bails at each end |
| Stands | 12 tiers of seating all the way round, textured seats, concrete risers, outer wall |
| Crowd | ~330 animated spectators performing a Mexican wave |
| Pavilion | Three-storey building with balconies, glass front, clock tower and sign |
| Dugouts | Two team dugouts with benches and seated reserves |
| Scoreboard | Giant LED board on pillars, updated live after every delivery |
| Floodlights | Four lattice towers with 18-lamp heads and visible light cones |
| Flags | 16 flags on the roof rim, animated with a travelling sine wave |
| Ad boards | Ring of 40 hoardings around the boundary |
| Players | Bowler, striker, non-striker, keeper, 2 umpires, 9 fielders |
| Sky | Textured sky dome, drifting clouds, stars at night |

### Animations
- Bowler run-up (translation) → full 360° arm rotation (rotation) → release
- Ball flight with gravity, pitch bounce and seam deviation
- Bat swing driven by a sine curve, batsman body lean
- Six / four / single / dot / wicket outcomes, chosen per delivery
- Fireworks particle burst after a six
- Crowd wave that speeds up and grows after a boundary
- Waving flags, drifting clouds, spinning ball with a ground shadow
- Live scoreboard update with a blinking event banner

### Weather / time of day (5 modes)
Sunny · Sunset · Night (floodlights on) · Rain (1400 particle streaks) · Fog

---

## 2. Files

```
CricketStadium3D/
├── main.cpp          window, render loop, HUD, animation timer
├── camera.cpp/.h     free-fly camera, projections, bird's eye, auto tour
├── stadium.cpp/.h    all geometry: ground, stands, players, scoreboard …
├── animation.cpp/.h  bowling state machine, ball physics, particle systems
├── lighting.cpp/.h   sun/moon, floodlights, materials, fog, weather colours
├── texture.cpp/.h    nine procedurally generated textures
├── input.cpp/.h      keyboard and mouse handling
├── common.h          shared constants and global state
├── Makefile          Linux / macOS / MinGW build
├── build_windows.bat one-click Windows build
├── README.md         this file
└── DOCUMENTATION.md  CGM concept write-up + viva questions
```

---

## 3. Compilation

### Linux / Ubuntu / WSL

```bash
sudo apt update
sudo apt install build-essential freeglut3-dev
cd CricketStadium3D
make
./CricketStadium
```

Manual command (equivalent):

```bash
g++ -O2 -o CricketStadium main.cpp camera.cpp stadium.cpp animation.cpp \
    lighting.cpp texture.cpp input.cpp -lglut -lGLU -lGL -lm
```

### Windows (MinGW-w64 + freeglut)

1. Install MinGW-w64 and add its `bin` folder to `PATH`.
2. Download the freeglut MinGW package; copy
   `include\GL\*` into MinGW's `include\GL\`,
   `lib\*.a` into MinGW's `lib\`, and `bin\freeglut.dll` next to the exe.
3. Build:

```bat
g++ -O2 -o CricketStadium.exe main.cpp camera.cpp stadium.cpp animation.cpp lighting.cpp texture.cpp input.cpp -lfreeglut -lopengl32 -lglu32
```

or just run `build_windows.bat`.

### Windows (Code::Blocks / Dev-C++)

Create an empty project, add all `.cpp` and `.h` files, then in
*Project → Build options → Linker settings* add:
`freeglut`, `opengl32`, `glu32` (in that order).

### macOS

```bash
make            # uses -framework OpenGL -framework GLUT
./CricketStadium
```

---

## 4. Keyboard & mouse controls

### Camera
| Key | Action |
|---|---|
| `W` / `S` | Move forward / backward |
| `A` / `D` | Strafe left / right |
| `Q` / `E` | Rise / descend |
| `Z` (hold) | Sprint (3× speed) |
| `↑ ↓ ← →` | Look up / down / left / right |
| Left-drag | Look around with the mouse |
| `M` | Toggle free mouse-look (pointer is captured) |
| Mouse wheel, `+` / `-`, `PgUp` / `PgDn` | Zoom in / out (changes FOV) |
| `B` | Bird's-eye (top-down) view |
| `O` | Automatic stadium tour |
| `P` | Switch perspective ↔ orthographic projection |
| `1` – `5` | Preset views: bowler's end, batsman's end, square leg, stands, scoreboard |

### Match & scene
| Key | Action |
|---|---|
| `SPACE` | Bowl a delivery now |
| `K` | Toggle auto-play (keeps bowling by itself) |
| `I` | Reset the scoreboard |
| `N` | Toggle day ↔ night |
| `C` | Cycle all five weather modes |
| `F1`–`F5` | Sunny / Sunset / Night / Rain / Fog |
| `L` | Toggle lighting on/off (to show the difference) |
| `T` | Toggle texture mapping on/off |
| `X` | Toggle wireframe mode |
| `V` | Show/hide the HUD |
| `Esc` | Quit |

> Tip for the viva: `L`, `T` and `X` let you show the examiner the same
> scene with lighting off, textures off and as pure wireframe — an easy way
> to demonstrate that each concept is really implemented.

---

## 5. Notes

- All nine textures are **generated in code** (noise, stripes, gradients), so
  the project runs anywhere with no missing-image problems.
- The simulation is driven by `glutTimerFunc` at ~60 FPS; all movement is
  multiplied by the real frame time (`dt`), so speed is machine-independent.
- Tested against `g++ -Wall -Wextra` with zero warnings.
- If the frame rate is low on integrated graphics, reduce `CSEG`/`CROWS` in
  `drawStands()` (stadium.cpp) or `NRAIN` in `animation.cpp`.

See **DOCUMENTATION.md** for the mapping of every CGM syllabus topic onto the
exact function that implements it.
