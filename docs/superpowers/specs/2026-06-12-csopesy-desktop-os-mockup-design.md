# CSOPESY Desktop-Style OS Mockup — Design Spec

**Date:** 2026-06-12
**Status:** Approved (design), pending implementation plan
**Repo:** https://github.com/robbie4116/Desktop-Style-OS-Mockup.git

## 1. Purpose

Build a desktop-style operating-system mockup in C++ using the Dear ImGui
immediate-mode GUI library. The application recreates the look and feel of the
"CSOPESY Desktop OS Emulator" reference: a BIOS boot screen, a loading splash,
and a full desktop environment with a wallpaper, real-time clock, a taskbar, and
openable application windows (File Explorer, Settings, and a Windows-style Task
Manager).

This satisfies a course assignment graded on four rubric criteria, each worth 10
points: Desktop Composition, Taskbar, Task Manager, and Documentation.

## 2. Requirements (from the assignment)

### Component 1 — The Desktop
- Full-screen background, drawn first each frame.
- Wallpaper via a loaded image texture (XP-Bliss style).
- Real-time clock, updated every frame, fixed in a corner.
- A **PWR** button that shuts the app down. The application must close via this
  button, not via force exit.

### Component 2 — The Taskbar
- Fixed panel (bottom of screen).
- At least three clickable icon buttons.
- Two buttons each open a unique UI screen with placeholder content.
- The third button opens the Task Manager.

### Component 3 — The Task Manager
- A window that closely resembles the Windows Task Manager.
- A placeholder table of "Processes" with CPU and memory usage (dummy values).

### Documentation
- Comprehensive README: walkthrough, architecture diagram, code snippets, build
  and usage instructions, screenshots.

## 3. Tech Stack & Build

- **Language:** C++20 ("use latest" per syllabus; C++20 is the universally
  supported latest standard across current MSVC and GCC 13+).
- **GUI:** Dear ImGui with the **GLFW + OpenGL3** backend.
- **Image loading:** `stb_image.h` (single-header, vendored).
- **Build system:** CMake (>= 3.16) using `FetchContent` to pull Dear ImGui and
  GLFW automatically — no manual SDK installation required.
- **Platform/architecture:** **x64**. Visual Studio generator invoked with
  `-A x64`; MinGW toolchains are x64 by default.
- **Window:** a single resizable GLFW window, initial size 1280×720, titled
  "CSOPESY Desktop OS Emulator". All rendering targets the live framebuffer size,
  so resizing reflows the desktop/taskbar automatically.
- **Build commands (documented in README):**
  - MSVC: `cmake -B build -A x64` then `cmake --build build --config Release`
  - MinGW: `cmake -B build -G "MinGW Makefiles"` then `cmake --build build`

### C++20 features used
- `std::format` for the clock and Task Manager value strings.
- Designated initializers for the `AppContext` struct.
- `<chrono>` calendar types for clock formatting.

**Compiler caveat:** `<format>` requires MSVC (VS 2019 16.10+ / VS 2022) or
GCC 13+. The clock/value formatting will use a `snprintf` fallback guarded by a
`__cpp_lib_format` feature-test macro so the project still builds on older GCC.
This is documented in the README build requirements.

## 4. Application State Machine

The main loop renders exactly one state per frame. States advance on a timer
(`glfwGetTime()`) and are skippable with a mouse click or key press.

```
BIOS (POST screen, ~3s) → SPLASH (CSOPESY loading, ~2.5s) → DESKTOP → [PWR] → exit
```

- **BIOS:** POST-style text (vendor banner, RAM check, CPU type Pentium III,
  BIOS version, detected drives, fun fact) rendered as monospace text on black.
- **SPLASH:** Centered CSOPESY logo text + animated "Loading…" dots, credit line.
- **DESKTOP:** The interactive environment (see §5).
- **Exit:** PWR sets `should_shutdown`; the main loop then calls
  `glfwSetWindowShouldClose` and terminates cleanly.

## 5. Components

Each unit has one responsibility, communicates through `AppContext`, and can be
read/tested in isolation. Header + source pairs unless noted.

| Unit | Responsibility |
|------|----------------|
| `main.cpp` | GLFW/OpenGL/ImGui init, main loop, state dispatch, clean shutdown |
| `app_context.h` | Shared struct: state, timers, wallpaper texture id, window-open flags, volume, `requested_settings_tab`, CPU-history buffer |
| `boot_screen.{h,cpp}` | Renders the BIOS POST screen |
| `splash_screen.{h,cpp}` | Renders the CSOPESY loading splash |
| `desktop.{h,cpp}` | Full-screen wallpaper texture, real-time clock (top-right), "CSOPESY OS v1.0 — System Online" tag |
| `taskbar.{h,cpp}` | Fixed bottom panel; buttons toggle window flags and shutdown |
| `windows/file_explorer.{h,cpp}` | Two-pane faux explorer: folder tree + file list with placeholder entries |
| `windows/settings.{h,cpp}` | Tabbed: Display (theme toggle, resolution combo), Sound (volume slider bound to VOL), About (BIOS-style system specs) |
| `windows/task_manager.{h,cpp}` | Windows-style process table + animated dummy CPU/memory values + CPU history line graph |
| `texture_loader.{h,cpp}` | Loads an image via stb_image into an OpenGL texture; returns failure so the desktop can fall back to a drawn gradient |
| `assets/wallpaper.png` | Bundled, generated XP-Bliss-style wallpaper image |

## 6. Component Details

### 6.1 Desktop
- Draws the wallpaper texture stretched to the full framebuffer **first** each
  frame (via a borderless full-screen ImGui window or a background draw list).
- If `texture_loader` reports no texture, draws a vertical sky-to-grass gradient
  with a hill silhouette using `ImDrawList` primitives (robust fallback).
- Real-time clock fixed top-right, formatted like
  `Thursday, Apr 30, 2026 | 05:25 PM`, recomputed every frame.
- A small green "CSOPESY OS v1.0 — System Online" label near the lower-left.

### 6.2 Taskbar
- Fixed full-width panel docked to the bottom, fixed height (~64 px).
- Left: a Start/folder icon button.
- Center-left: **File Explorer**, **Settings**, **Task Manager** buttons (these
  three satisfy the ≥3 requirement; File Explorer and Settings are the two
  "unique UI screens", Task Manager is the required third).
- Right-aligned: **VOL**, **NET**, **PWR** buttons.
- Clicking an app button toggles its `open_*` flag in `AppContext`. PWR sets
  `should_shutdown`.

### 6.3 File Explorer (app window 1)
- Resizable/draggable/closable ImGui window.
- Left pane: a tree of placeholder folders (Desktop, Documents, Downloads,
  Games, System).
- Right pane: a table/list of placeholder files (name, type, size) for the
  selected folder. All static placeholder data.

### 6.4 Settings (app window 2)
- Tabbed window (`ImGui::BeginTabBar`):
  - **Display:** dark/light accent toggle, resolution combo box (cosmetic).
  - **Sound:** master volume slider bound to `AppContext::volume`; the taskbar
    VOL button sets `open_settings = true` and `requested_settings_tab = Sound`,
    and the Settings window selects that tab once via `ImGuiTabItemFlags_SetSelected`
    then clears the request.
  - **About:** read-only system specs echoing the BIOS screen (OS name/version,
    CPU Pentium III, BIOS version, RAM, drives, credit to Dr. Neil Patrick Del
    Gallego).

### 6.5 Task Manager (rubric-critical)
- Bordered window styled to resemble Windows Task Manager.
- Header status line: e.g. `CPU 23%  ·  Memory 41%` (recomputed from the dummy
  data each frame).
- A small CPU-usage line graph (`ImGui::PlotLines`) fed by a rolling history
  buffer in `AppContext`.
- A process table (`ImGui::BeginTable` with borders + headers): columns
  **Process | PID | CPU % | Memory (MB)**, ~10 fake rows
  (csopesy.exe, explorer.exe, dwm.exe, ImGui_demo.exe, etc.).
- **Animated dummy values:** each row's CPU/memory jitters by a small bounded
  random delta per frame so the table feels live. Jitter uses a per-row seed and
  is clamped to plausible ranges.

## 7. Data Flow

- `main` owns a single `AppContext` instance (no globals).
- Boot/splash/desktop render functions receive `AppContext&`.
- The taskbar is the only writer of `open_file_explorer`, `open_settings`,
  `open_task_manager`, `volume`, and `should_shutdown`.
- Each window renders only when its `open_*` flag is true; ImGui's built-in
  close button writes the flag back to false.
- Task Manager appends to the CPU-history buffer each frame it is open. The
  buffer is a fixed-size ring (e.g. 90 samples) living in `AppContext`, so it
  persists across close/reopen rather than resetting.

## 8. Wallpaper Asset Strategy

The user chose a loaded image texture. To avoid depending on a copyrighted
asset, a stylized XP-Bliss-like wallpaper (`assets/wallpaper.png`) is generated
and committed to the repo. `texture_loader` loads it at startup. The path is
resolved relative to the executable, with the source `assets/` dir as a fallback
during development. If loading fails, the drawn-gradient fallback (§6.1) keeps
the desktop functional.

## 9. Shutdown Behavior

PWR is the designed exit path. Pressing it sets `should_shutdown = true`; the
main loop detects this, breaks, and performs ordered cleanup (ImGui backend
shutdown, GLFW window/context destruction). The native window close button
remains functional as an OS-level affordance, but the intended and documented
shutdown is PWR, satisfying "must be closed using this button, not by force
exit."

## 10. Documentation Plan (rubric item)

`README.md` will include:
- Project overview and screenshots (BIOS, splash, desktop, each window).
- Build requirements (compiler versions, CMake) and exact x64 build/run commands.
- **Architecture diagram:** a Mermaid state-machine diagram of the boot flow and
  a component/data-flow diagram.
- A per-component walkthrough with representative **code snippets**.
- Controls/usage (how to skip boot, open windows, shut down).
- Credits.

## 11. Out of Scope (YAGNI)

- Real process enumeration or real system metrics.
- Persistent settings / file I/O for the explorer.
- Multiple wallpapers, themes beyond a cosmetic accent toggle.
- Window snapping, multi-monitor handling, audio playback for VOL.
- Networking for the NET button (decorative only).

## 12. Risks

- **`<format>` availability** on older GCC — mitigated by the `snprintf`
  fallback (§3).
- **Asset path resolution** across build dirs — mitigated by checking multiple
  candidate paths plus the gradient fallback.
- **FetchContent network access** at configure time — documented as a build
  prerequisite; alternatively GLFW/ImGui can be vendored if offline builds are
  needed (noted in README).
