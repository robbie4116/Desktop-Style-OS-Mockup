# UI Polish — Design Spec
**Date:** 2026-06-12
**Status:** Approved

## Overview

Five targeted visual improvements to the CSOPESY OS Mockup: icon-only taskbar buttons, status label relocation, longer splash with a loading bar, and a monitor-adaptive window size.

---

## 1. Taskbar Icon Buttons

### What changes
`src/render/taskbar.cpp`

Replace the current `tbButton()` helper (colored rectangle + text label) with two new helpers:

- `drawIcon(ImDrawList* dl, IconType type, ImVec2 center, float size, ImU32 color)` — stateless function that draws one icon using ImDrawList primitives (AddLine, AddRect, AddCircle, AddPolyline). Takes a center point and uniform size so icons scale cleanly.
- `tbIconButton(const char* id, IconType type, ImVec4 bg, float width)` — renders an `ImGui::InvisibleButton` for hit-testing, then uses the draw list to fill the background rect and draw the icon on top. Returns `bool` (clicked), same contract as the old helper.

**Icon set** — all 1.5px stroke. Note: `ImDrawList` does not support round line caps; all stroked lines render with flat/square endpoints. Where a rounder look is desirable (e.g. the power arc endpoints), a small filled circle (`AddCircleFilled`, radius = 0.75px) can be drawn at each endpoint as a best-effort approximation.

| Button | IconType | Geometry |
|--------|----------|----------|
| Start | `Icon_Start` | 2×2 grid of four equal squares with a small gap |
| Files | `Icon_Files` | Folder: bottom rect + top trapezoid tab |
| Settings | `Icon_Settings` | Circle (center hole) + 8 evenly-spaced radial spokes |
| Task Mgr | `Icon_TaskMgr` | Three vertical bars of increasing height (left-shortest, right-tallest) |
| VOL | `Icon_Vol` | Speaker triangle body + one curved arc |
| NET | `Icon_Net` | Three vertical signal bars of increasing height |
| PWR | `Icon_Pwr` | Circle arc (gap at top) + short vertical stem at gap |

Button background colors remain unchanged from the current palette. Button dimensions: 44×40px for left cluster, **56×40px for right cluster (VOL/NET/PWR) — down from the current 64px**; this is a deliberate reduction to better match the icon-only proportions.

### What does NOT change
- Button colors
- Click behaviour / `AppContext` flags each button sets
- Taskbar height (56px) or background color

---

## 2. Status Label Relocation

### What changes
- `src/render/desktop.cpp` — remove the `AddText` call at line 46 that draws `"CSOPESY OS v1.0 - System Online"` on the desktop canvas.
- `src/render/taskbar.cpp` — after rendering the left button group, push a dim green color (`IM_COL32(80, 200, 100, 180)`) via `ImGui::PushStyleColor(ImGuiCol_Text, ...)`, render `ImGui::Text("CSOPESY OS v1.0 - System Online")`, then `ImGui::PopStyleColor()`. This places the label naturally in the center gap between the left buttons and the right cluster.

The label string uses an **ASCII hyphen-minus** (`-`) throughout — matching the existing string in `desktop.cpp:46` exactly. It will be slightly dimmer than the current desktop rendering (alpha 180 vs 255) to feel like secondary information within the taskbar rather than a headline.

### What does NOT change
- The label string
- The same string in `splash_screen.cpp:28` and `settings.cpp:34` (those are separate and correct)

---

## 3. Splash Screen — Duration & Loading Bar

### What changes
**`src/app_context.h:16`**
```cpp
// Before
BootTimings timings{3.0, 2.5};
// After
BootTimings timings{3.0, 3.0};
```

**`src/render/splash_screen.cpp`**

After the existing "Loading..." animated dots, add a progress bar:

1. Add `#include <algorithm>` to `splash_screen.cpp` (required for `std::clamp`).
2. The `renderSplash` function signature is `renderSplash(const AppContext& ctx, double elapsed)`. Ensure the `ctx` parameter is named in the `.cpp` definition (the current implementation leaves it unnamed). Use `ctx.timings.splashDuration` as the duration value.
3. Compute `float progress = std::clamp(static_cast<float>(elapsed / ctx.timings.splashDuration), 0.0f, 1.0f)`.
4. Determine bar geometry:
   - Width: 260px, height: 3px
   - Horizontally centered in the window
   - Vertically: 16px below the bottom of the dots text
5. Draw via ImDrawList:
   - Track: `AddRectFilled(trackMin, trackMax, IM_COL32(40, 40, 40, 255))`
   - Fill: `AddRectFilled(trackMin, ImVec2(trackMin.x + 260.0f * progress, trackMax.y), IM_COL32(80, 230, 120, 255))`
   - No rounded corners (rect, not rounded rect) — matches the crisp aesthetic.

The bar reaches 100% exactly as `elapsed == duration` and the state machine transitions to Desktop. The existing skip-on-keypress behaviour is unchanged — if the user skips early the bar simply won't be full.

### What does NOT change
- BIOS screen duration (still 3.0s)
- Splash title, subtitle, animated dots
- Skip behaviour (mouse click, Space, Enter, Escape)

---

## 4. Monitor-Adaptive Window Size

### What changes
`src/main.cpp`, before the `glfwCreateWindow` call:

```cpp
GLFWmonitor* mon  = glfwGetPrimaryMonitor();
const GLFWvidmode* mode = glfwGetVideoMode(mon);
int w = static_cast<int>(mode->width  * 0.75f);
int h = static_cast<int>(mode->height * 0.75f);
```

Pass `w` and `h` to `glfwCreateWindow` instead of the hardcoded `1280, 720`.

After window creation, center the window on the monitor:
```cpp
int mx, my;
glfwGetMonitorPos(mon, &mx, &my);
glfwSetWindowPos(window,
    mx + (mode->width  - w) / 2,
    my + (mode->height - h) / 2);
```

**Resulting sizes on common displays:**

| Monitor | Window |
|---------|--------|
| 2560×1440 (user's) | 1920×1080 |
| 1920×1080 | 1440×810 |
| 1366×768 | 1024×576 |
| 1280×800 | 960×600 |

All results are above the existing 800×500 minimum size constraint.

### What does NOT change
- `glfwSetWindowSizeLimits` (800×500 min, no max)
- Window title
- VSync / OpenGL context setup

---

## Files Affected

| File | Change |
|------|--------|
| `src/main.cpp` | Window creation size + centering |
| `src/app_context.h` | `splashDuration` 2.5 → 3.0 |
| `src/render/splash_screen.cpp` | Add progress bar |
| `src/render/taskbar.cpp` | Icon helpers, status label |
| `src/render/desktop.cpp` | Remove status label AddText call |

No new source files. No new asset files. No CMakeLists changes.

---

## Out of Scope

- BIOS screen duration (stays 3.0s, already matches splash)
- Clock widget on desktop
- Window contents / existing app windows (File Explorer, Settings, Task Manager)
- Any changes to `src/logic/` (pure logic layer untouched)
