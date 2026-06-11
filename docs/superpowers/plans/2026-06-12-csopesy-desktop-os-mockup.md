# CSOPESY Desktop-Style OS Mockup Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build a C++20 desktop-OS mockup with Dear ImGui (GLFW+OpenGL3) that boots through a BIOS POST screen and a CSOPESY splash into an interactive desktop with a wallpaper, real-time clock, a bottom taskbar, and three app windows (File Explorer, Settings, Windows-style Task Manager), exiting only via a PWR button.

**Architecture:** A single GLFW window runs an ImGui render loop driven by a small state machine (Bios → Splash → Desktop). Pure, ImGui-free logic units (clock formatting, boot-state transitions, process value jitter, CPU history ring buffer, texture-path resolution) are unit-tested with doctest. Rendering units (boot, splash, desktop, taskbar, windows) read/write a single shared `AppContext` and are verified by compiling and running the app. Dependencies (ImGui, GLFW) are fetched by CMake via `FetchContent`; `stb_image`, `stb_image_write`, and `doctest` are vendored single headers.

**Tech Stack:** C++20, CMake ≥ 3.16 (x64), Dear ImGui (docking-free master), GLFW 3.4, OpenGL 3.3, stb_image, doctest.

---

## Testing Strategy (read first)

Immediate-mode GUI draw calls cannot be meaningfully asserted on, so this plan splits the work:

- **Pure logic** (`clock_format`, `boot_state`, `process_model`, `cpu_history`, `texture_paths`) is written ImGui-free and **unit-tested with doctest first (TDD)**. These compile into a `csopesy_tests` executable.
- **Rendering/UI** (`boot_screen`, `splash_screen`, `desktop`, `taskbar`, `windows/*`, `texture_loader`, `main`) is verified by: (a) the project compiles with zero warnings-as-errors, and (b) a **manual run check** with an explicit "what you should see" expectation. Where a rendering unit contains extractable logic, that logic lives in a tested pure unit.

`std::format` (C++20) is used for status/label strings with a `__cpp_lib_format`-guarded `snprintf` fallback. The clock uses `<ctime>`/`strftime` on a `std::tm` (portable and trivially testable).

---

## File Structure

```
Desktop-Style-OS-Mockup/
├── CMakeLists.txt                  # build, FetchContent deps, app + tests + wallpaper_gen targets
├── .gitignore                      # build/, *.user, etc.
├── README.md                       # documentation (Chunk 8)
├── third_party/
│   ├── stb_image.h                 # vendored single header
│   ├── stb_image_write.h           # vendored single header (wallpaper generator)
│   └── doctest.h                   # vendored single header (tests)
├── assets/
│   └── wallpaper.png               # generated bliss-style wallpaper (Chunk 5)
├── tools/
│   └── make_wallpaper.cpp          # standalone generator -> assets/wallpaper.png
├── src/
│   ├── main.cpp                    # init, main loop, state dispatch, shutdown
│   ├── app_context.h               # shared AppContext struct (+ enums)
│   ├── logic/                      # ImGui-free, unit-tested
│   │   ├── clock_format.h / .cpp
│   │   ├── boot_state.h / .cpp
│   │   ├── process_model.h / .cpp
│   │   ├── cpu_history.h           # header-only
│   │   └── texture_paths.h / .cpp
│   ├── render/
│   │   ├── boot_screen.h / .cpp
│   │   ├── splash_screen.h / .cpp
│   │   ├── desktop.h / .cpp
│   │   ├── taskbar.h / .cpp
│   │   ├── texture_loader.h / .cpp
│   │   └── windows/
│   │       ├── file_explorer.h / .cpp
│   │       ├── settings.h / .cpp
│   │       └── task_manager.h / .cpp
└── tests/
    ├── test_main.cpp               # #define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
    ├── test_clock_format.cpp
    ├── test_boot_state.cpp
    ├── test_process_model.cpp
    ├── test_cpu_history.cpp
    └── test_texture_paths.cpp
```

---

## Chunk 1: Project scaffolding & build system

**Outcome:** `cmake -B build -A x64 && cmake --build build` produces an app that opens a 1280×720 window titled "CSOPESY Desktop OS Emulator" showing a dark-gray cleared frame, and closes via the window control. A `csopesy_tests` target exists and runs (zero tests for now).

### Task 1.1: Vendored third-party headers

**Files:**
- Create: `third_party/stb_image.h`
- Create: `third_party/stb_image_write.h`
- Create: `third_party/doctest.h`

- [ ] **Step 1: Download the three single-header libraries**

Fetch pinned, reproducible versions (a fixed stb commit and a tagged doctest
release — never `master`, which is a moving target):
```bash
STB=f0569113c93ad095470c54bf34a17b36646bbbb1   # known-good stb commit
curl -L -o third_party/stb_image.h        https://raw.githubusercontent.com/nothings/stb/$STB/stb_image.h
curl -L -o third_party/stb_image_write.h  https://raw.githubusercontent.com/nothings/stb/$STB/stb_image_write.h
curl -L -o third_party/doctest.h          https://raw.githubusercontent.com/doctest/doctest/v2.4.11/doctest/doctest.h
```
Expected: three files exist, each > 10 KB. Verify with `ls -l third_party`.
> If a pinned URL 404s, fall back to the latest release tag and record the
> resolved version in a comment at the top of the file.

- [ ] **Step 2: Commit**
```bash
git add third_party
git commit -m "chore: vendor stb_image, stb_image_write, doctest headers"
```

### Task 1.2: .gitignore

**Files:**
- Create: `.gitignore`

- [ ] **Step 1: Write `.gitignore`**
```gitignore
/build/
/out/
*.obj
*.exe
*.pdb
*.user
.vs/
CMakeUserPresets.json
```

- [ ] **Step 2: Commit**
```bash
git add .gitignore && git commit -m "chore: add .gitignore"
```

### Task 1.3: CMakeLists.txt with dependencies and three targets

**Files:**
- Create: `CMakeLists.txt`
- Create: `src/main.cpp` (minimal, replaced in Chunk 3)

- [ ] **Step 1: Write `CMakeLists.txt`**
```cmake
cmake_minimum_required(VERSION 3.16)
project(csopesy_os LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

# --- GLFW ---
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4)
FetchContent_MakeAvailable(glfw)

# --- Dear ImGui (not a CMake project; build sources ourselves) ---
FetchContent_Declare(imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.5)
FetchContent_MakeAvailable(imgui)

add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp)
target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)
target_link_libraries(imgui PUBLIC glfw)

find_package(OpenGL REQUIRED)

# --- pure logic library (ImGui-free, unit-tested) ---
add_library(csopesy_logic STATIC
    src/logic/clock_format.cpp
    src/logic/boot_state.cpp
    src/logic/process_model.cpp
    src/logic/texture_paths.cpp)
target_include_directories(csopesy_logic PUBLIC src)
target_compile_features(csopesy_logic PUBLIC cxx_std_20)

# --- main application ---
add_executable(csopesy_os
    src/main.cpp
    src/render/boot_screen.cpp
    src/render/splash_screen.cpp
    src/render/desktop.cpp
    src/render/taskbar.cpp
    src/render/texture_loader.cpp
    src/render/windows/file_explorer.cpp
    src/render/windows/settings.cpp
    src/render/windows/task_manager.cpp)
target_include_directories(csopesy_os PRIVATE src third_party)
target_link_libraries(csopesy_os PRIVATE imgui csopesy_logic OpenGL::GL)

# --- tests ---
add_executable(csopesy_tests
    tests/test_main.cpp
    tests/test_clock_format.cpp
    tests/test_boot_state.cpp
    tests/test_process_model.cpp
    tests/test_cpu_history.cpp
    tests/test_texture_paths.cpp)
target_include_directories(csopesy_tests PRIVATE src third_party)
target_link_libraries(csopesy_tests PRIVATE csopesy_logic)
enable_testing()
add_test(NAME unit COMMAND csopesy_tests)

# --- wallpaper generator (standalone, run once) ---
add_executable(wallpaper_gen tools/make_wallpaper.cpp)
target_include_directories(wallpaper_gen PRIVATE third_party)
```

> Note: the source files referenced above are created in later chunks. To keep this chunk buildable on its own, Step 2 stubs them.

- [ ] **Step 2: Create minimal stubs so Chunk 1 compiles**

Create empty-but-valid placeholders for every non-`main` source referenced above so the configure/build succeeds before later chunks fill them in. Each `.cpp` for a logic/render unit gets a one-line comment body; `tools/make_wallpaper.cpp` and `tests/*` get the minimal content below. Create:
- `src/logic/clock_format.cpp`, `src/logic/boot_state.cpp`, `src/logic/process_model.cpp`, `src/logic/texture_paths.cpp` — each just `// implemented in Chunk 2`
- `src/render/boot_screen.cpp`, `splash_screen.cpp`, `desktop.cpp`, `taskbar.cpp`, `texture_loader.cpp`, `windows/file_explorer.cpp`, `windows/settings.cpp`, `windows/task_manager.cpp` — each just `// implemented in later chunks`
- `tools/make_wallpaper.cpp` → `int main(){return 0;}`
- `tests/test_main.cpp` → the real doctest entry point now (so the test target links in Chunk 1):
  ```cpp
  #define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
  #include "doctest.h"
  ```
  (Task 2.0 then becomes a no-op confirmation that this file exists.)
- `tests/test_clock_format.cpp`, `test_boot_state.cpp`, `test_process_model.cpp`, `test_cpu_history.cpp`, `test_texture_paths.cpp` — each empty (doctest discovers no tests yet)

- [ ] **Step 3: Write minimal `src/main.cpp` (blank window)**
```cpp
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <cstdio>

int main() {
    if (!glfwInit()) { std::fprintf(stderr, "glfwInit failed\n"); return 1; }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "CSOPESY Desktop OS Emulator", nullptr, nullptr);
    if (!window) { glfwTerminate(); return 1; }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // state-machine rendering added in Chunk 3

        ImGui::Render();
        int w, h; glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);
        glClearColor(0.10f, 0.10f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
```

- [ ] **Step 4: Configure and build**

Run:
```
cmake -B build -A x64
cmake --build build --config Release
```
Expected: configures (FetchContent clones glfw + imgui on first run), builds `csopesy_os`, `csopesy_tests`, `wallpaper_gen` with no errors.

> If the generator is MinGW instead of Visual Studio, use `cmake -B build -G "MinGW Makefiles"` and omit `-A x64` (MinGW is x64 by default). Document both in README.

- [ ] **Step 5: Manual run check**

Run `build/Release/csopesy_os.exe` (MSVC) or `build/csopesy_os.exe` (MinGW).
Expected: a 1280×720 window titled "CSOPESY Desktop OS Emulator" with a dark gray background appears and closes cleanly via the window's X.

- [ ] **Step 6: Commit**
```bash
git add CMakeLists.txt src tools tests
git commit -m "build: scaffold CMake project, deps, and blank ImGui window"
```

---

## Chunk 2: Pure logic units (TDD with doctest)

**Outcome:** `csopesy_tests` runs and all tests pass. Logic units are implemented ImGui-free.

### Task 2.0: confirm doctest entry point

`tests/test_main.cpp` was already created with the doctest main in Chunk 1
(Task 1.3, Step 2) so the test target links from the start.

- [ ] **Step 1: Verify** `tests/test_main.cpp` contains exactly:
```cpp
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
```
If missing or different, create it now. No separate commit needed.

### Task 2.1: clock_format

**Files:**
- Create: `src/logic/clock_format.h`
- Replace: `src/logic/clock_format.cpp`
- Replace: `tests/test_clock_format.cpp`

- [ ] **Step 1: Write the failing test**
```cpp
#include "doctest.h"
#include "logic/clock_format.h"
#include <ctime>

static std::tm makeTm(int year, int mon, int mday, int hour, int min, int wday) {
    std::tm t{};
    t.tm_year = year - 1900; t.tm_mon = mon; t.tm_mday = mday;
    t.tm_hour = hour; t.tm_min = min; t.tm_wday = wday;
    return t;
}

TEST_CASE("formatClock renders the reference style") {
    // Thursday 2026-04-30 17:25  (wday 4 = Thursday, mon 3 = April)
    std::tm t = makeTm(2026, 3, 30, 17, 25, 4);
    CHECK(csopesy::formatClock(t) == "Thursday, Apr 30, 2026 | 05:25 PM");
}

TEST_CASE("formatClock pads and uses AM correctly") {
    std::tm t = makeTm(2026, 0, 5, 9, 7, 1); // Monday Jan 05 09:07
    CHECK(csopesy::formatClock(t) == "Monday, Jan 05, 2026 | 09:07 AM");
}
```
- [ ] **Step 2: Run to verify it fails**

Run: `cmake --build build && ctest --test-dir build -R unit -V`
Expected: FAIL — `formatClock` undefined / link error.

- [ ] **Step 3: Implement**

`src/logic/clock_format.h`:
```cpp
#pragma once
#include <ctime>
#include <string>
namespace csopesy { std::string formatClock(const std::tm& t); }
```
`src/logic/clock_format.cpp`:
```cpp
#include "logic/clock_format.h"
namespace csopesy {
std::string formatClock(const std::tm& t) {
    char buf[64];
    // %A full weekday, %b abbrev month, %d zero-padded day, %Y year,
    // %I 12-hour zero-padded, %M minute, %p AM/PM
    std::strftime(buf, sizeof(buf), "%A, %b %d, %Y | %I:%M %p", &t);
    return std::string(buf);
}
}
```
> `strftime` relies on `tm_wday`/`tm_mon` being set; tests set them explicitly. At runtime `localtime` fills all fields.

- [ ] **Step 4: Run to verify it passes**

Run: `cmake --build build && ctest --test-dir build -R unit -V`
Expected: PASS.

- [ ] **Step 5: Commit**
```bash
git add src/logic/clock_format.* tests/test_clock_format.cpp
git commit -m "feat: clock_format with reference-style output (tested)"
```

### Task 2.2: boot_state

**Files:**
- Create: `src/logic/boot_state.h`
- Replace: `src/logic/boot_state.cpp`
- Replace: `tests/test_boot_state.cpp`

- [ ] **Step 1: Write the failing test**
```cpp
#include "doctest.h"
#include "logic/boot_state.h"
using namespace csopesy;

static BootTimings T{3.0, 2.5};

TEST_CASE("bios advances to splash on timeout") {
    CHECK(advanceBootState(AppState::Bios, 2.9, false, T) == AppState::Bios);
    CHECK(advanceBootState(AppState::Bios, 3.0, false, T) == AppState::Splash);
}
TEST_CASE("bios advances to splash on skip") {
    CHECK(advanceBootState(AppState::Bios, 0.1, true, T) == AppState::Splash);
}
TEST_CASE("splash advances to desktop on timeout or skip") {
    CHECK(advanceBootState(AppState::Splash, 2.4, false, T) == AppState::Splash);
    CHECK(advanceBootState(AppState::Splash, 2.5, false, T) == AppState::Desktop);
    CHECK(advanceBootState(AppState::Splash, 0.0, true, T) == AppState::Desktop);
}
TEST_CASE("desktop is terminal") {
    CHECK(advanceBootState(AppState::Desktop, 999.0, true, T) == AppState::Desktop);
}
```
- [ ] **Step 2: Run to verify it fails**

Run: `cmake --build build && ctest --test-dir build -R unit -V`
Expected: FAIL — undefined symbols.

- [ ] **Step 3: Implement**

`src/logic/boot_state.h`:
```cpp
#pragma once
namespace csopesy {
enum class AppState { Bios, Splash, Desktop };
struct BootTimings { double biosDuration; double splashDuration; };
AppState advanceBootState(AppState current, double elapsedInState,
                          bool skipRequested, const BootTimings& t);
}
```
`src/logic/boot_state.cpp`:
```cpp
#include "logic/boot_state.h"
namespace csopesy {
AppState advanceBootState(AppState current, double elapsedInState,
                          bool skipRequested, const BootTimings& t) {
    switch (current) {
        case AppState::Bios:
            return (skipRequested || elapsedInState >= t.biosDuration)
                       ? AppState::Splash : AppState::Bios;
        case AppState::Splash:
            return (skipRequested || elapsedInState >= t.splashDuration)
                       ? AppState::Desktop : AppState::Splash;
        case AppState::Desktop:
        default:
            return AppState::Desktop;
    }
}
}
```
- [ ] **Step 4: Run to verify it passes** — Expected: PASS.
- [ ] **Step 5: Commit**
```bash
git add src/logic/boot_state.* tests/test_boot_state.cpp
git commit -m "feat: boot_state transition logic (tested)"
```

### Task 2.3: process_model

**Files:**
- Create: `src/logic/process_model.h`
- Replace: `src/logic/process_model.cpp`
- Replace: `tests/test_process_model.cpp`

- [ ] **Step 1: Write the failing test**
```cpp
#include "doctest.h"
#include "logic/process_model.h"
using namespace csopesy;

TEST_CASE("LCG jitter is deterministic for a given seed") {
    unsigned a = 12345, b = 12345;
    float va = nextUnit(a);
    float vb = nextUnit(b);
    CHECK(va == vb);   // exact: same seed -> same value
    CHECK(a == b);     // states advanced identically
}
TEST_CASE("updateProcess keeps cpu and memory within clamped ranges") {
    ProcessRow row{"test.exe", 1000, 20.0f, 100.0f, 99u};
    for (int i = 0; i < 5000; ++i) {
        updateProcess(row);
        CHECK(row.cpu >= 0.0f);
        CHECK(row.cpu <= 100.0f);
        CHECK(row.memMB >= 1.0f);
        CHECK(row.memMB <= 4096.0f);
    }
}
TEST_CASE("aggregateCpu averages the rows") {
    std::vector<ProcessRow> rows{
        {"a", 1, 10.0f, 0.0f, 1u}, {"b", 2, 30.0f, 0.0f, 2u}};
    CHECK(aggregateCpu(rows) == doctest::Approx(20.0f));
}
TEST_CASE("makeDefaultProcesses returns a non-empty believable list") {
    auto v = makeDefaultProcesses();
    CHECK(v.size() >= 8);
    CHECK(std::string(v.front().name).size() > 0);
}
```
- [ ] **Step 2: Run to verify it fails** — Expected: FAIL.

- [ ] **Step 3: Implement**

`src/logic/process_model.h`:
```cpp
#pragma once
#include <vector>
namespace csopesy {

struct ProcessRow {
    const char* name;
    int   pid;
    float cpu;      // percent, 0..100
    float memMB;    // megabytes
    unsigned rng;   // per-row LCG state
};

// Advance LCG, return float in [0,1).
float nextUnit(unsigned& state);

// Apply a bounded random walk to cpu/memMB, clamped to plausible ranges.
void updateProcess(ProcessRow& row);

// Average CPU across rows (0 if empty).
float aggregateCpu(const std::vector<ProcessRow>& rows);

// Sum of memory across rows in MB.
float aggregateMemMB(const std::vector<ProcessRow>& rows);

// A believable static process list (csopesy.exe, explorer.exe, ...).
std::vector<ProcessRow> makeDefaultProcesses();
}
```
`src/logic/process_model.cpp`:
```cpp
#include "logic/process_model.h"
#include <algorithm>
namespace csopesy {

float nextUnit(unsigned& state) {
    state = state * 1664525u + 1013904223u;      // Numerical Recipes LCG
    return static_cast<float>(state >> 8) / 16777216.0f; // [0,1)
}

static float clampf(float v, float lo, float hi) {
    return std::max(lo, std::min(hi, v));
}

void updateProcess(ProcessRow& row) {
    float dCpu = (nextUnit(row.rng) - 0.5f) * 4.0f;   // +/- 2 %
    float dMem = (nextUnit(row.rng) - 0.5f) * 8.0f;   // +/- 4 MB
    row.cpu   = clampf(row.cpu + dCpu, 0.0f, 100.0f);
    row.memMB = clampf(row.memMB + dMem, 1.0f, 4096.0f);
}

float aggregateCpu(const std::vector<ProcessRow>& rows) {
    if (rows.empty()) return 0.0f;
    float s = 0.0f; for (auto& r : rows) s += r.cpu;
    return s / static_cast<float>(rows.size());
}

float aggregateMemMB(const std::vector<ProcessRow>& rows) {
    float s = 0.0f; for (auto& r : rows) s += r.memMB; return s;
}

std::vector<ProcessRow> makeDefaultProcesses() {
    return {
        {"csopesy.exe",       1024, 12.4f, 184.0f, 101u},
        {"explorer.exe",      812,   3.1f, 142.0f, 202u},
        {"dwm.exe",           640,   5.7f,  98.0f, 303u},
        {"System",            4,     1.2f,  24.0f, 404u},
        {"imgui_demo.exe",    2310,  8.9f,  64.0f, 505u},
        {"anito_engine.exe",  3001, 27.3f, 412.0f, 606u},
        {"audiodg.exe",       1190,  0.8f,  18.0f, 707u},
        {"svchost.exe",       980,   2.0f,  76.0f, 808u},
        {"taskmgr.exe",       2780,  4.5f,  52.0f, 909u},
        {"notepad.exe",       3140,  0.3f,  12.0f, 110u},
    };
}
}
```
- [ ] **Step 4: Run to verify it passes** — Expected: PASS.
- [ ] **Step 5: Commit**
```bash
git add src/logic/process_model.* tests/test_process_model.cpp
git commit -m "feat: process_model with deterministic jitter (tested)"
```

### Task 2.4: cpu_history (header-only ring buffer)

**Files:**
- Create: `src/logic/cpu_history.h`
- Replace: `tests/test_cpu_history.cpp`

- [ ] **Step 1: Write the failing test**
```cpp
#include "doctest.h"
#include "logic/cpu_history.h"
using namespace csopesy;

TEST_CASE("history never exceeds capacity and keeps newest order") {
    CpuHistory h(3);
    h.push(1.0f); h.push(2.0f); h.push(3.0f); h.push(4.0f);
    CHECK(h.size() == 3);
    const float* d = h.data();
    CHECK(d[0] == doctest::Approx(2.0f)); // oldest dropped
    CHECK(d[2] == doctest::Approx(4.0f)); // newest last
}
TEST_CASE("starts empty and fills") {
    CpuHistory h(4);
    CHECK(h.size() == 0);
    h.push(5.0f);
    CHECK(h.size() == 1);
    CHECK(h.data()[0] == doctest::Approx(5.0f));
}
```
- [ ] **Step 2: Run to verify it fails** — Expected: FAIL.

- [ ] **Step 3: Implement**

`src/logic/cpu_history.h`:
```cpp
#pragma once
#include <vector>
#include <cstddef>
namespace csopesy {

// Fixed-capacity FIFO of floats with contiguous storage for ImGui::PlotLines.
class CpuHistory {
public:
    explicit CpuHistory(std::size_t capacity) : cap_(capacity) {
        buf_.reserve(capacity);
    }
    void push(float v) {
        if (buf_.size() < cap_) buf_.push_back(v);
        else { buf_.erase(buf_.begin()); buf_.push_back(v); }
    }
    const float* data() const { return buf_.data(); }
    std::size_t size() const { return buf_.size(); }
    std::size_t capacity() const { return cap_; }
private:
    std::size_t cap_;
    std::vector<float> buf_;
};
}
```
- [ ] **Step 4: Run to verify it passes** — Expected: PASS.
- [ ] **Step 5: Commit**
```bash
git add src/logic/cpu_history.h tests/test_cpu_history.cpp
git commit -m "feat: cpu_history ring buffer (tested)"
```

### Task 2.5: texture_paths

**Files:**
- Create: `src/logic/texture_paths.h`
- Replace: `src/logic/texture_paths.cpp`
- Replace: `tests/test_texture_paths.cpp`

- [ ] **Step 1: Write the failing test**
```cpp
#include "doctest.h"
#include "logic/texture_paths.h"
using namespace csopesy;

TEST_CASE("candidate paths include exe-relative and cwd-relative options") {
    auto v = wallpaperCandidatePaths("C:/app/bin");
    CHECK(v.size() >= 3);
    CHECK(v.front() == "C:/app/bin/assets/wallpaper.png");
    bool hasCwd = false;
    for (auto& p : v) if (p == "assets/wallpaper.png") hasCwd = true;
    CHECK(hasCwd);
}
TEST_CASE("empty exe dir still yields cwd candidate") {
    auto v = wallpaperCandidatePaths("");
    bool hasCwd = false;
    for (auto& p : v) if (p == "assets/wallpaper.png") hasCwd = true;
    CHECK(hasCwd);
}
```
- [ ] **Step 2: Run to verify it fails** — Expected: FAIL.

- [ ] **Step 3: Implement**

`src/logic/texture_paths.h`:
```cpp
#pragma once
#include <string>
#include <vector>
namespace csopesy {
// Ordered list of places to look for the wallpaper, given the executable dir.
std::vector<std::string> wallpaperCandidatePaths(const std::string& exeDir);
}
```
`src/logic/texture_paths.cpp`:
```cpp
#include "logic/texture_paths.h"
namespace csopesy {
std::vector<std::string> wallpaperCandidatePaths(const std::string& exeDir) {
    std::vector<std::string> v;
    if (!exeDir.empty()) {
        v.push_back(exeDir + "/assets/wallpaper.png");
        v.push_back(exeDir + "/../assets/wallpaper.png");
        v.push_back(exeDir + "/../../assets/wallpaper.png");
    }
    v.push_back("assets/wallpaper.png");
    v.push_back("../assets/wallpaper.png");
    return v;
}
}
```
- [ ] **Step 4: Run to verify it passes** — Expected: PASS.
- [ ] **Step 5: Commit**
```bash
git add src/logic/texture_paths.* tests/test_texture_paths.cpp
git commit -m "feat: texture_paths candidate resolution (tested)"
```

---

## Chunk 3: AppContext + state-machine integration

**Outcome:** The app boots through placeholder Bios/Splash/Desktop states using the tested `boot_state` logic, advancing on timers and skipping on click/key. Each state shows a simple labeled full-screen panel (real rendering comes next). Shutdown flag plumbed.

### Task 3.1: AppContext

**Files:**
- Create: `src/app_context.h`

- [ ] **Step 1: Write `src/app_context.h`**
```cpp
#pragma once
#include "logic/boot_state.h"
#include "logic/cpu_history.h"
#include "logic/process_model.h"
#include <vector>

namespace csopesy {

enum class SettingsTab { Display, Sound, About };

struct AppContext {
    // boot state machine
    AppState state = AppState::Bios;
    double   stateEnteredTime = 0.0;   // glfwGetTime() when current state began
    bool     skipRequested = false;    // set by input handler each frame
    BootTimings timings{3.0, 2.5};

    // desktop / wallpaper
    unsigned wallpaperTex = 0;         // GLuint; 0 = none
    bool     wallpaperLoaded = false;

    // window open flags (taskbar is the only writer besides ImGui close)
    bool openFileExplorer = false;
    bool openSettings = false;
    bool openTaskManager = false;

    // settings tab routing (VOL button -> Sound tab)
    SettingsTab requestedSettingsTab = SettingsTab::Display;
    bool        settingsTabRequested = false;

    // misc state
    float volume = 0.5f;
    bool  shouldShutdown = false;

    // task manager data
    CpuHistory cpuHistory{90};
    std::vector<ProcessRow> processes = makeDefaultProcesses();
};
}
```
- [ ] **Step 2: Commit**
```bash
git add src/app_context.h && git commit -m "feat: AppContext shared state struct"
```

### Task 3.2: Wire state machine + input + shutdown into main loop

**Files:**
- Modify: `src/main.cpp`

- [ ] **Step 1: Replace the loop body in `src/main.cpp`**

Add includes near the top:
```cpp
#include "app_context.h"
#include "logic/boot_state.h"
```
Create the context before the loop:
```cpp
csopesy::AppContext ctx;
ctx.stateEnteredTime = glfwGetTime();
```
Replace the loop with state handling (placeholder panels; real renderers wired in later chunks):
```cpp
while (!glfwWindowShouldClose(window) && !ctx.shouldShutdown) {
    glfwPollEvents();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // --- input: any click or key skips boot/splash ---
    // NOTE: v1.91.5 removed the legacy io.KeysDown[]/io.MouseClicked[] IO arrays.
    // Use the function-based event API exclusively.
    ctx.skipRequested = ImGui::IsMouseClicked(ImGuiMouseButton_Left) ||
                        ImGui::IsKeyPressed(ImGuiKey_Space, false) ||
                        ImGui::IsKeyPressed(ImGuiKey_Enter, false) ||
                        ImGui::IsKeyPressed(ImGuiKey_Escape, false);

    // --- advance state machine ---
    double now = glfwGetTime();
    double elapsed = now - ctx.stateEnteredTime;
    csopesy::AppState next = csopesy::advanceBootState(
        ctx.state, elapsed, ctx.skipRequested, ctx.timings);
    if (next != ctx.state) { ctx.state = next; ctx.stateEnteredTime = now; }

    // --- placeholder rendering (replaced in later chunks) ---
    switch (ctx.state) {
        case csopesy::AppState::Bios:    /* renderBootScreen(ctx, elapsed) */ break;
        case csopesy::AppState::Splash:  /* renderSplash(ctx, elapsed) */ break;
        case csopesy::AppState::Desktop: /* renderDesktop / taskbar / windows */ break;
    }
    {
        ImGui::SetNextWindowPos(ImVec2(20, 20));
        ImGui::Begin("state", nullptr, ImGuiWindowFlags_NoResize|ImGuiWindowFlags_AlwaysAutoResize);
        const char* names[] = {"BIOS", "SPLASH", "DESKTOP"};
        ImGui::Text("state: %s  elapsed: %.1f", names[(int)ctx.state], elapsed);
        if (ImGui::Button("PWR (quit)")) ctx.shouldShutdown = true;
        ImGui::End();
    }

    ImGui::Render();
    int w, h; glfwGetFramebufferSize(window, &w, &h);
    glViewport(0, 0, w, h);
    glClearColor(0.10f, 0.10f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
}
```
> Do NOT use `io.KeysDown[]` or `io.MouseClicked[]` — both were removed in the ImGui keyboard rework and are absent in the pinned v1.91.5. The function-based API (`ImGui::IsMouseClicked`, `ImGui::IsKeyPressed`) shown above is the only correct form.

- [ ] **Step 2: Build** — Run `cmake --build build`. Expected: builds clean.

- [ ] **Step 3: Manual run check**

Run the app. Expected: shows "state: BIOS" for ~3s, auto-advances to "SPLASH" (~2.5s), then "DESKTOP" and stays. Clicking anywhere during BIOS/SPLASH skips immediately. The "PWR (quit)" button closes the app cleanly (no force exit).

- [ ] **Step 4: Commit**
```bash
git add src/main.cpp
git commit -m "feat: integrate boot state machine, input skip, and PWR shutdown"
```

---

## Chunk 4: Boot & Splash screens

**Outcome:** BIOS POST and CSOPESY splash render like the reference, replacing the placeholders.

### Task 4.1: Boot screen

**Files:**
- Create: `src/render/boot_screen.h`
- Replace: `src/render/boot_screen.cpp`
- Modify: `src/main.cpp`

- [ ] **Step 1: Write `src/render/boot_screen.h`**
```cpp
#pragma once
namespace csopesy { struct AppContext;
void renderBootScreen(const AppContext& ctx, double elapsed); }
```
- [ ] **Step 2: Implement `src/render/boot_screen.cpp`**

Render a full-screen, undecorated black window with monospaced POST text. Reveal lines progressively (one every ~0.18s) for authenticity. Content mirrors the reference screenshot.
```cpp
#include "render/boot_screen.h"
#include "app_context.h"
#include "imgui.h"

namespace csopesy {

static const char* kLines[] = {
    "CSOPESY Megatrends",
    "Released: 12/01/94",
    "DLSU GAME Lab (C)1994 CSOPESY Megatrends Inc.",
    "",
    "Memory Test:",
    "Checking RAM : 64000K OK",
    "",
    "CPU Type: Pentium III",
    "BIOS Version: BCN SIT 1989-1994 Special CSOPESY",
    "Main Processor: Pentium III",
    "Numeric Processor: Built-in",
    "",
    "Primary Master:   420 MB WDCAC2420H",
    "Primary Slave:    None",
    "Secondary Master: CD-ROM LTN-305A",
    "Secondary Slave:  None",
    "",
    "Fun Fact: The CPU specs are a tribute to the Bemani Python 2 Engine,",
    "the OS that powers up Dance Dance Revolution SuperNova series.",
    "It was essentially a retail Sony PlayStation 2 inside a metal box.",
};

void renderBootScreen(const AppContext&, double elapsed) {
    const ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->Pos);
    ImGui::SetNextWindowSize(vp->Size);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0,0,0,1));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::Begin("##bios", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus);

    // ~0.12s per line so all 20 lines reveal within the 3.0s BIOS duration.
    int reveal = static_cast<int>(elapsed / 0.12) + 1;
    int total = (int)(sizeof(kLines)/sizeof(kLines[0]));
    if (reveal > total) reveal = total;

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.85f, 0.85f, 1.0f));
    for (int i = 0; i < reveal; ++i) ImGui::TextUnformatted(kLines[i]);
    ImGui::PopStyleColor();

    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}
}
```
> Uses the default font (monospace-ish in fixed layout). If a monospace look is desired, this is acceptable for the mockup; a custom font is out of scope (§11 of spec).

- [ ] **Step 3: Wire into `main.cpp`**

Add `#include "render/boot_screen.h"` and replace the Bios case:
```cpp
case csopesy::AppState::Bios: csopesy::renderBootScreen(ctx, elapsed); break;
```
Remove the temporary "state" debug window's relevance to BIOS (keep PWR button for now; it is replaced by the taskbar in Chunk 6 — leave it gated to Desktop only):
```cpp
if (ctx.state == csopesy::AppState::Desktop) { /* keep temp PWR window */ }
```
- [ ] **Step 4: Build** — Expected: clean.
- [ ] **Step 5: Manual run check** — Expected: black screen, POST lines appear progressively, then advances to splash. Clicking skips.
- [ ] **Step 6: Commit**
```bash
git add src/render/boot_screen.* src/main.cpp
git commit -m "feat: BIOS POST boot screen"
```

### Task 4.2: Splash screen

**Files:**
- Create: `src/render/splash_screen.h`
- Replace: `src/render/splash_screen.cpp`
- Modify: `src/main.cpp`

- [ ] **Step 1: Write `src/render/splash_screen.h`**
```cpp
#pragma once
namespace csopesy { struct AppContext;
void renderSplash(const AppContext& ctx, double elapsed); }
```
- [ ] **Step 2: Implement `src/render/splash_screen.cpp`**
```cpp
#include "render/splash_screen.h"
#include "app_context.h"
#include "imgui.h"

namespace csopesy {
void renderSplash(const AppContext&, double elapsed) {
    const ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->Pos);
    ImGui::SetNextWindowSize(vp->Size);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0,0,0,1));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::Begin("##splash", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus);

    auto center = [&](const char* txt, float scale, ImVec4 col, float yFrac) {
        ImGui::SetWindowFontScale(scale);
        ImVec2 ts = ImGui::CalcTextSize(txt);
        ImGui::SetCursorPos(ImVec2((vp->Size.x - ts.x) * 0.5f, vp->Size.y * yFrac));
        ImGui::PushStyleColor(ImGuiCol_Text, col);
        ImGui::TextUnformatted(txt);
        ImGui::PopStyleColor();
        ImGui::SetWindowFontScale(1.0f);
    };

    center("CSOPESY", 4.0f, ImVec4(0.30f,0.45f,0.70f,1.0f), 0.34f);
    center("CSOPESY Operating System Emulator v1.0", 1.1f, ImVec4(0.8f,0.8f,0.8f,1), 0.52f);
    center("Created By: Dr. Neil Patrick Del Gallego", 1.0f, ImVec4(0.7f,0.7f,0.7f,1), 0.57f);
    center("Part of Project Anito", 1.0f, ImVec4(0.7f,0.7f,0.7f,1), 0.64f);

    int dots = (static_cast<int>(elapsed / 0.4) % 4);
    char loading[16] = "Loading";
    for (int i = 0; i < dots; ++i) loading[7 + i] = '.';
    loading[7 + dots] = '\0';
    center(loading, 1.1f, ImVec4(0.4f,0.8f,0.4f,1.0f), 0.80f);

    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}
}
```
- [ ] **Step 3: Wire into `main.cpp`** — `#include "render/splash_screen.h"`; Splash case → `csopesy::renderSplash(ctx, elapsed);`
- [ ] **Step 4: Build** — Expected: clean.
- [ ] **Step 5: Manual run check** — Expected: centered blue "CSOPESY", subtitle/credits, animated "Loading…" dots; advances to desktop. Clicking skips.
- [ ] **Step 6: Commit**
```bash
git add src/render/splash_screen.* src/main.cpp
git commit -m "feat: CSOPESY loading splash screen"
```

---

## Chunk 5: Desktop — wallpaper, clock, version tag (+ asset generation)

**Outcome:** The Desktop state shows a full-screen wallpaper image (loaded from `assets/wallpaper.png`, with a drawn gradient fallback), a boxed real-time clock top-right, and a green version tag lower-left.

### Task 5.1: Wallpaper asset generator

**Files:**
- Replace: `tools/make_wallpaper.cpp`
- Create (output, committed): `assets/wallpaper.png`

- [ ] **Step 1: Implement the generator**

A standalone program that synthesizes a 1280×720 XP-Bliss-style scene (sky gradient + rolling green hill) and writes a PNG via `stb_image_write`.
```cpp
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <vector>
#include <cmath>
#include <cstdint>

int main() {
    const int W = 1280, H = 720;
    std::vector<uint8_t> img(W * H * 3);
    auto set = [&](int x, int y, int r, int g, int b) {
        int i = (y * W + x) * 3; img[i]=r; img[i+1]=g; img[i+2]=b;
    };
    for (int y = 0; y < H; ++y) {
        float t = (float)y / H;
        // sky: deep blue -> light blue
        int sr = (int)(70  + t * 120);
        int sg = (int)(130 + t * 100);
        int sb = (int)(200 + t * 50);
        // hill curve
        float hill = 0.72f + 0.06f * std::sin((float)0 * 0.0f);
        for (int x = 0; x < W; ++x) {
            float hy = 0.70f + 0.05f * std::sin(x * 0.006f) + 0.02f * std::sin(x * 0.02f);
            if (t < hy) set(x, y, sr, sg, sb);
            else {
                float g = (t - hy) / (1.0f - hy);
                set(x, y, (int)(60 + g*30), (int)(140 + g*60), (int)(50 + g*20));
            }
        }
    }
    stbi_write_png("assets/wallpaper.png", W, H, 3, img.data(), W * 3);
    return 0;
}
```
- [ ] **Step 2: Build and run the generator**

Run:
```
cmake --build build --target wallpaper_gen
build/Release/wallpaper_gen.exe   # run from repo root so assets/ resolves
```
Expected: `assets/wallpaper.png` created (~ a few hundred KB). Open it to confirm a sky-over-hills image.

> If the run CWD is the build dir, run from the repo root or pass an absolute output path. Verify the file lands at `assets/wallpaper.png`.

- [ ] **Step 3: Commit**
```bash
git add tools/make_wallpaper.cpp assets/wallpaper.png
git commit -m "feat: generate bliss-style wallpaper asset"
```

### Task 5.2: texture_loader

**Files:**
- Create: `src/render/texture_loader.h`
- Replace: `src/render/texture_loader.cpp`

- [ ] **Step 1: Write `src/render/texture_loader.h`**
```cpp
#pragma once
#include <string>
namespace csopesy {
struct LoadedTexture { unsigned id = 0; int width = 0; int height = 0; bool ok = false; };
// Tries each candidate path (see logic/texture_paths). Returns ok=false if none load.
LoadedTexture loadWallpaperTexture(const std::string& exeDir);
}
```
- [ ] **Step 2: Implement `src/render/texture_loader.cpp`**
```cpp
#include "render/texture_loader.h"
#include "logic/texture_paths.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <GLFW/glfw3.h>   // pulls GL types + GL 1.1 texture functions

namespace csopesy {
LoadedTexture loadWallpaperTexture(const std::string& exeDir) {
    LoadedTexture out;
    for (const auto& path : wallpaperCandidatePaths(exeDir)) {
        int w, h, comp;
        unsigned char* px = stbi_load(path.c_str(), &w, &h, &comp, 4);
        if (!px) continue;
        GLuint tex = 0;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, px);
        stbi_image_free(px);
        out = {static_cast<unsigned>(tex), w, h, true};
        return out;
    }
    return out; // ok=false
}
}
```
> `STB_IMAGE_IMPLEMENTATION` is defined here only. `STB_IMAGE_WRITE_IMPLEMENTATION` is defined only in `tools/make_wallpaper.cpp`, so no ODR clash.

- [ ] **Step 3: Build** — Expected: clean (no link errors for `glGenTextures` etc.; they come from `OpenGL::GL` / opengl32).
- [ ] **Step 4: Commit**
```bash
git add src/render/texture_loader.*
git commit -m "feat: wallpaper texture loader (stb_image -> GL texture)"
```

### Task 5.3: Desktop rendering

**Files:**
- Create: `src/render/desktop.h`
- Replace: `src/render/desktop.cpp`
- Modify: `src/main.cpp`

- [ ] **Step 1: Write `src/render/desktop.h`**
```cpp
#pragma once
namespace csopesy { struct AppContext;
// Draws wallpaper (or gradient fallback), clock, and version tag.
void renderDesktopBackground(const AppContext& ctx); }
```
- [ ] **Step 2: Implement `src/render/desktop.cpp`**
```cpp
#include "render/desktop.h"
#include "app_context.h"
#include "logic/clock_format.h"
#include "imgui.h"
#include <ctime>

namespace csopesy {

static void drawGradientFallback(ImDrawList* dl, ImVec2 p0, ImVec2 p1) {
    ImU32 top = IM_COL32(70,130,200,255), bot = IM_COL32(150,200,235,255);
    float horizon = p0.y + (p1.y - p0.y) * 0.70f;
    dl->AddRectFilledMultiColor(p0, ImVec2(p1.x, horizon), top, top, bot, bot);
    dl->AddRectFilled(ImVec2(p0.x, horizon), p1, IM_COL32(80,150,60,255));
}

void renderDesktopBackground(const AppContext& ctx) {
    const ImGuiViewport* vp = ImGui::GetMainViewport();
    ImDrawList* bg = ImGui::GetBackgroundDrawList();
    ImVec2 p0 = vp->Pos;
    ImVec2 p1 = ImVec2(vp->Pos.x + vp->Size.x, vp->Pos.y + vp->Size.y);

    if (ctx.wallpaperLoaded && ctx.wallpaperTex)
        bg->AddImage((ImTextureID)(intptr_t)ctx.wallpaperTex, p0, p1);
    else
        drawGradientFallback(bg, p0, p1);

    // --- clock, boxed, top-right ---
    std::time_t now = std::time(nullptr);
    std::tm local{};
#if defined(_WIN32)
    localtime_s(&local, &now);
#else
    localtime_r(&now, &local);
#endif
    std::string clock = formatClock(local);
    ImDrawList* fg = ImGui::GetForegroundDrawList();
    ImVec2 ts = ImGui::CalcTextSize(clock.c_str());
    ImVec2 pad(10, 6);
    ImVec2 boxMax(p1.x - 12, p0.y + 12 + ts.y + pad.y * 2);
    ImVec2 boxMin(boxMax.x - ts.x - pad.x * 2, p0.y + 12);
    fg->AddRectFilled(boxMin, boxMax, IM_COL32(20,20,30,210), 4.0f);
    fg->AddText(ImVec2(boxMin.x + pad.x, boxMin.y + pad.y), IM_COL32(230,230,235,255), clock.c_str());

    // --- version tag, lower-left ---
    fg->AddText(ImVec2(p0.x + 16, p1.y - 110),
                IM_COL32(80,230,120,255), "CSOPESY OS v1.0 - System Online");
}
}
```
- [ ] **Step 3: Load the wallpaper at startup in `main.cpp`**

After ImGui init, before the loop:
```cpp
#include "render/texture_loader.h"
#include "render/desktop.h"
// ...
{
    csopesy::LoadedTexture wt = csopesy::loadWallpaperTexture(""); // cwd candidates
    ctx.wallpaperTex = wt.id;
    ctx.wallpaperLoaded = wt.ok;
}
```
In the Desktop case:
```cpp
case csopesy::AppState::Desktop:
    csopesy::renderDesktopBackground(ctx);
    break;
```
- [ ] **Step 4: Build** — Expected: clean.
- [ ] **Step 5: Manual run check**

Run from repo root. Expected: after boot/splash, the desktop shows the bliss wallpaper filling the window, a boxed live clock top-right updating each second, and a green version tag lower-left. Rename `assets/wallpaper.png` temporarily and re-run to confirm the gradient fallback draws; restore the file.

- [ ] **Step 6: Commit**
```bash
git add src/render/desktop.* src/main.cpp
git commit -m "feat: desktop wallpaper, live clock, version tag with gradient fallback"
```

---

## Chunk 6: Taskbar + window flag wiring + PWR

**Outcome:** A fixed bottom taskbar with Start/folder, File Explorer, Settings, Task Manager buttons (left) and VOL / NET / PWR (right). Buttons toggle `AppContext` flags; PWR shuts down. The temporary debug window is removed.

### Task 6.1: Taskbar

**Files:**
- Create: `src/render/taskbar.h`
- Replace: `src/render/taskbar.cpp`
- Modify: `src/main.cpp`

- [ ] **Step 1: Write `src/render/taskbar.h`**
```cpp
#pragma once
namespace csopesy { struct AppContext;
void renderTaskbar(AppContext& ctx); }
```
- [ ] **Step 2: Implement `src/render/taskbar.cpp`**
```cpp
#include "render/taskbar.h"
#include "app_context.h"
#include "imgui.h"

namespace csopesy {

static bool tbButton(const char* label, const ImVec4& col) {
    ImGui::PushStyleColor(ImGuiCol_Button, col);
    bool clicked = ImGui::Button(label, ImVec2(0, 40));
    ImGui::PopStyleColor();
    return clicked;
}

void renderTaskbar(AppContext& ctx) {
    const ImGuiViewport* vp = ImGui::GetMainViewport();
    const float barH = 56.0f;
    ImGui::SetNextWindowPos(ImVec2(vp->Pos.x, vp->Pos.y + vp->Size.y - barH));
    ImGui::SetNextWindowSize(ImVec2(vp->Size.x, barH));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.07f,0.07f,0.10f,0.96f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::Begin("##taskbar", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus);

    if (tbButton("[#] Start", ImVec4(0.20f,0.45f,0.20f,1))) {} // decorative
    ImGui::SameLine();
    if (tbButton("Files", ImVec4(0.18f,0.30f,0.50f,1))) ctx.openFileExplorer = !ctx.openFileExplorer;
    ImGui::SameLine();
    if (tbButton("Settings", ImVec4(0.30f,0.30f,0.40f,1))) ctx.openSettings = !ctx.openSettings;
    ImGui::SameLine();
    if (tbButton("Task Mgr", ImVec4(0.45f,0.30f,0.20f,1))) ctx.openTaskManager = !ctx.openTaskManager;

    // right-aligned cluster: VOL  NET  PWR
    // SameLine() takes an offset within the window content region, so align
    // against GetWindowContentRegionMax().x (accounts for WindowPadding), not
    // the raw viewport width.
    const float btnW = 64.0f, spacing = ImGui::GetStyle().ItemSpacing.x;
    float regionRight = ImGui::GetWindowContentRegionMax().x;
    float rightX = regionRight - (btnW * 3 + spacing * 2);
    ImGui::SameLine(rightX);
    if (ImGui::Button("VOL", ImVec2(btnW, 40))) {
        ctx.openSettings = true;
        ctx.requestedSettingsTab = SettingsTab::Sound;
        ctx.settingsTabRequested = true;
    }
    ImGui::SameLine();
    ImGui::Button("NET", ImVec2(btnW, 40)); // decorative
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f,0.15f,0.15f,1));
    if (ImGui::Button("PWR", ImVec2(btnW, 40))) ctx.shouldShutdown = true;
    ImGui::PopStyleColor();

    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}
}
```
- [ ] **Step 3: Wire into `main.cpp`**

`#include "render/taskbar.h"`. In the Desktop case, after `renderDesktopBackground(ctx)`:
```cpp
csopesy::renderTaskbar(ctx);
```
Remove the temporary "state"/PWR debug window entirely.

- [ ] **Step 4: Build** — Expected: clean.
- [ ] **Step 5: Manual run check** — Expected: bottom taskbar with the five+ buttons; Files/Settings/Task Mgr toggle flags (no windows yet — verify via no crash and button highlight), VOL sets the Settings+Sound request, PWR quits. The debug window is gone.
- [ ] **Step 6: Commit**
```bash
git add src/render/taskbar.* src/main.cpp
git commit -m "feat: bottom taskbar with app buttons, VOL/NET, and PWR shutdown"
```

---

## Chunk 7: App windows — File Explorer, Settings, Task Manager

**Outcome:** All three windows render when their taskbar buttons are toggled, are draggable/closable, and contain the specified placeholder content. Task Manager animates.

### Task 7.1: File Explorer

**Files:**
- Create: `src/render/windows/file_explorer.h`
- Replace: `src/render/windows/file_explorer.cpp`
- Modify: `src/main.cpp`

- [ ] **Step 1: Write `src/render/windows/file_explorer.h`**
```cpp
#pragma once
namespace csopesy { struct AppContext;
void renderFileExplorer(AppContext& ctx); }
```
- [ ] **Step 2: Implement `src/render/windows/file_explorer.cpp`**
```cpp
#include "render/windows/file_explorer.h"
#include "app_context.h"
#include "imgui.h"

namespace csopesy {

struct FileEntry { const char* name; const char* type; const char* size; };

void renderFileExplorer(AppContext& ctx) {
    if (!ctx.openFileExplorer) return;
    ImGui::SetNextWindowSize(ImVec2(560, 360), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("File Explorer", &ctx.openFileExplorer)) { ImGui::End(); return; }

    static int selected = 0;
    const char* folders[] = {"Desktop", "Documents", "Downloads", "Games", "System"};

    // v1.91.5: BeginChild's 3rd arg is ImGuiChildFlags, not bool. Use the
    // Borders flag (passing `true`/1 would NOT set the border bit, which is 1<<1).
    ImGui::BeginChild("tree", ImVec2(160, 0), ImGuiChildFlags_Borders);
    for (int i = 0; i < 5; ++i)
        if (ImGui::Selectable(folders[i], selected == i)) selected = i;
    ImGui::EndChild();

    ImGui::SameLine();
    ImGui::BeginChild("files", ImVec2(0, 0), ImGuiChildFlags_Borders);
    ImGui::Text("This PC > %s", folders[selected]);
    ImGui::Separator();
    static const FileEntry sets[5][3] = {
        {{"readme.txt","Text","2 KB"},{"shortcut.lnk","Shortcut","1 KB"},{"notes.md","Markdown","4 KB"}},
        {{"resume.docx","Document","18 KB"},{"budget.xlsx","Spreadsheet","22 KB"},{"thesis.pdf","PDF","1.2 MB"}},
        {{"setup.exe","Application","44 MB"},{"image.png","Image","820 KB"},{"song.mp3","Audio","5.1 MB"}},
        {{"anito.exe","Application","210 MB"},{"ddr_sim.exe","Application","88 MB"},{"save01.dat","Data","64 KB"}},
        {{"kernel32.dll","System","1.1 MB"},{"config.sys","System","1 KB"},{"hosts","System","1 KB"}},
    };
    if (ImGui::BeginTable("filelist", 3,
            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable)) {
        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupColumn("Type");
        ImGui::TableSetupColumn("Size");
        ImGui::TableHeadersRow();
        for (int i = 0; i < 3; ++i) {
            const FileEntry& e = sets[selected][i];
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted(e.name);
            ImGui::TableSetColumnIndex(1); ImGui::TextUnformatted(e.type);
            ImGui::TableSetColumnIndex(2); ImGui::TextUnformatted(e.size);
        }
        ImGui::EndTable();
    }
    ImGui::EndChild();
    ImGui::End();
}
}
```
- [ ] **Step 3: Wire into `main.cpp`** — `#include "render/windows/file_explorer.h"`; in Desktop case after taskbar: `csopesy::renderFileExplorer(ctx);`
- [ ] **Step 4: Build** — Expected: clean.
- [ ] **Step 5: Manual run check** — Expected: clicking "Files" opens a two-pane explorer; selecting a folder changes the file table; the window's X closes it (button toggles it back open).
- [ ] **Step 6: Commit**
```bash
git add src/render/windows/file_explorer.* src/main.cpp
git commit -m "feat: File Explorer window"
```

### Task 7.2: Settings

**Files:**
- Create: `src/render/windows/settings.h`
- Replace: `src/render/windows/settings.cpp`
- Modify: `src/main.cpp`

- [ ] **Step 1: Write `src/render/windows/settings.h`**
```cpp
#pragma once
namespace csopesy { struct AppContext;
void renderSettings(AppContext& ctx); }
```
- [ ] **Step 2: Implement `src/render/windows/settings.cpp`**
```cpp
#include "render/windows/settings.h"
#include "app_context.h"
#include "imgui.h"

namespace csopesy {

static ImGuiTabItemFlags tabFlag(AppContext& ctx, SettingsTab tab) {
    if (ctx.settingsTabRequested && ctx.requestedSettingsTab == tab)
        return ImGuiTabItemFlags_SetSelected;
    return 0;
}

void renderSettings(AppContext& ctx) {
    if (!ctx.openSettings) return;
    ImGui::SetNextWindowSize(ImVec2(460, 320), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Settings", &ctx.openSettings)) { ImGui::End(); return; }

    if (ImGui::BeginTabBar("settabs")) {
        if (ImGui::BeginTabItem("Display", nullptr, tabFlag(ctx, SettingsTab::Display))) {
            static bool darkAccent = true;
            static int resIdx = 1;
            const char* res[] = {"1024 x 768", "1280 x 720", "1920 x 1080"};
            ImGui::Checkbox("Dark accent", &darkAccent);
            ImGui::Combo("Resolution", &resIdx, res, IM_ARRAYSIZE(res));
            ImGui::TextDisabled("(cosmetic placeholders)");
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Sound", nullptr, tabFlag(ctx, SettingsTab::Sound))) {
            ImGui::SliderFloat("Master Volume", &ctx.volume, 0.0f, 1.0f, "%.2f");
            ImGui::ProgressBar(ctx.volume, ImVec2(-1, 0));
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("About", nullptr, tabFlag(ctx, SettingsTab::About))) {
            ImGui::Text("CSOPESY Operating System Emulator v1.0");
            ImGui::Separator();
            ImGui::BulletText("CPU: Pentium III");
            ImGui::BulletText("BIOS: BCN SIT 1989-1994 Special CSOPESY");
            ImGui::BulletText("RAM: 64000K");
            ImGui::BulletText("Primary Master: 420 MB WDCAC2420H");
            ImGui::BulletText("Optical: CD-ROM LTN-305A");
            ImGui::Spacing();
            ImGui::TextDisabled("Created by Dr. Neil Patrick Del Gallego");
            ImGui::TextDisabled("Part of Project Anito");
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ctx.settingsTabRequested = false; // consume the one-shot request
    ImGui::End();
}
}
```
- [ ] **Step 3: Wire into `main.cpp`** — `#include "render/windows/settings.h"`; Desktop case: `csopesy::renderSettings(ctx);`
- [ ] **Step 4: Build** — Expected: clean.
- [ ] **Step 5: Manual run check** — Expected: "Settings" opens a tabbed window; the volume slider works; clicking taskbar **VOL** opens Settings focused on the **Sound** tab. About shows the BIOS-style specs.
- [ ] **Step 6: Commit**
```bash
git add src/render/windows/settings.* src/main.cpp
git commit -m "feat: Settings window with Display/Sound/About tabs and VOL routing"
```

### Task 7.3: Task Manager

**Files:**
- Create: `src/render/windows/task_manager.h`
- Replace: `src/render/windows/task_manager.cpp`
- Modify: `src/main.cpp`

- [ ] **Step 1: Write `src/render/windows/task_manager.h`**
```cpp
#pragma once
namespace csopesy { struct AppContext;
void renderTaskManager(AppContext& ctx); }
```
- [ ] **Step 2: Implement `src/render/windows/task_manager.cpp`**

Status string uses `std::format` with a guarded `snprintf` fallback.
```cpp
#include "render/windows/task_manager.h"
#include "app_context.h"
#include "logic/process_model.h"
#include "imgui.h"
#include <cstdio>
#if defined(__cpp_lib_format)
  #include <format>
#endif
#include <string>

namespace csopesy {

static std::string fmtStatus(float cpu, float memPct) {
#if defined(__cpp_lib_format)
    return std::format("CPU {:.0f}%   -   Memory {:.0f}%", cpu, memPct);
#else
    char b[64]; std::snprintf(b, sizeof(b), "CPU %.0f%%   -   Memory %.0f%%", cpu, memPct);
    return std::string(b);
#endif
}

void renderTaskManager(AppContext& ctx) {
    if (!ctx.openTaskManager) return;

    // advance the dummy values every frame the window is open
    for (auto& p : ctx.processes) updateProcess(p);
    float cpu = aggregateCpu(ctx.processes);
    float memTotal = 8192.0f; // pretend 8 GB
    float memPct = (aggregateMemMB(ctx.processes) / memTotal) * 100.0f;
    ctx.cpuHistory.push(cpu);

    ImGui::SetNextWindowSize(ImVec2(540, 460), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Task Manager", &ctx.openTaskManager)) { ImGui::End(); return; }

    ImGui::TextUnformatted(fmtStatus(cpu, memPct).c_str());
    ImGui::PlotLines("##cpu", ctx.cpuHistory.data(), (int)ctx.cpuHistory.size(),
                     0, "CPU history", 0.0f, 100.0f, ImVec2(-1, 80));
    ImGui::Separator();

    if (ImGui::BeginTable("procs", 4,
            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
            ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY,
            ImVec2(0, 260))) {
        ImGui::TableSetupColumn("Process");
        ImGui::TableSetupColumn("PID");
        ImGui::TableSetupColumn("CPU %");
        ImGui::TableSetupColumn("Memory (MB)");
        ImGui::TableHeadersRow();
        for (const auto& p : ctx.processes) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted(p.name);
            ImGui::TableSetColumnIndex(1); ImGui::Text("%d", p.pid);
            ImGui::TableSetColumnIndex(2); ImGui::Text("%.1f", p.cpu);
            ImGui::TableSetColumnIndex(3); ImGui::Text("%.0f", p.memMB);
        }
        ImGui::EndTable();
    }

    ImGui::TextDisabled("Values are simulated placeholders.");
    ImGui::End();
}
}
```
- [ ] **Step 3: Wire into `main.cpp`** — `#include "render/windows/task_manager.h"`; Desktop case: `csopesy::renderTaskManager(ctx);`
- [ ] **Step 4: Build** — Expected: clean.
- [ ] **Step 5: Manual run check** — Expected: "Task Mgr" opens a Windows-like window with a status line, a moving CPU history graph, and a bordered process table whose CPU/Memory values jitter every frame; closable via X.
- [ ] **Step 6: Commit**
```bash
git add src/render/windows/task_manager.* src/main.cpp
git commit -m "feat: Task Manager window with animated process table and CPU graph"
```

### Task 7.4: Full integration smoke test

- [ ] **Step 1: Build the whole project and run tests**

Run:
```
cmake --build build --config Release
ctest --test-dir build -R unit -V
```
Expected: build clean; all unit tests PASS.

- [ ] **Step 2: Full manual run check**

Run from repo root. Verify end-to-end: BIOS → splash → desktop; wallpaper + live clock; open all three windows simultaneously, drag/resize/close them; VOL→Sound tab; PWR exits cleanly. No crashes, no console errors.

- [ ] **Step 3: Commit (if any fixes were needed)**
```bash
git add -A && git commit -m "test: end-to-end integration verified"
```

---

## Chunk 8: Documentation & final polish

**Outcome:** A complete `README.md` satisfying the Documentation rubric, with screenshots, architecture diagrams, build/run instructions, and code snippets.

### Task 8.1: Capture screenshots

**Files:**
- Create: `docs/images/bios.png`, `splash.png`, `desktop.png`, `taskmgr.png`, `explorer.png`, `settings.png`

- [ ] **Step 1: Run the app and capture each screen**

Launch the app and capture each screen using the **Windows-MCP `Screenshot`
tool** (or the OS Snipping Tool if unavailable). Save with these exact names
under `docs/images/`: `bios.png`, `splash.png`, `desktop.png`, `explorer.png`,
`settings.png`, `taskmgr.png`.
Expected: six PNGs exist under `docs/images/`.

- [ ] **Step 2: Commit**
```bash
git add docs/images && git commit -m "docs: add application screenshots"
```

### Task 8.2: README

**Files:**
- Create: `README.md`

- [ ] **Step 1: Write `README.md`** with these sections:

1. **Title + one-paragraph overview** and a hero screenshot (`docs/images/desktop.png`).
2. **Features** — bulleted list mapped to the three rubric components + boot sequence.
3. **Screenshots** — embed the six images with captions.
4. **Architecture** — a Mermaid state diagram and a component/data-flow diagram:
   ````markdown
   ```mermaid
   stateDiagram-v2
     [*] --> BIOS
     BIOS --> SPLASH: 3s / click
     SPLASH --> DESKTOP: 2.5s / click
     DESKTOP --> [*]: PWR
   ```
   ````
   ````markdown
   ```mermaid
   flowchart TD
     main[main.cpp loop] --> ctx[AppContext]
     main --> boot[boot_screen]
     main --> splash[splash_screen]
     main --> desk[desktop]
     main --> bar[taskbar]
     bar --> ctx
     desk --> tex[texture_loader]
     main --> fe[file_explorer]
     main --> set[settings]
     main --> tm[task_manager]
     tm --> logic[process_model / cpu_history]
     desk --> clk[clock_format]
   ```
   ````
5. **Project structure** — the file tree from this plan with one-line descriptions.
6. **Build & Run** — exact commands for MSVC and MinGW (x64), prerequisites (CMake ≥3.16, a C++20 compiler — VS 2022 or GCC 13+, network access for FetchContent on first configure), and "run from the repo root so `assets/` resolves".
7. **Controls** — click/Esc/Enter/Space to skip boot; taskbar buttons; PWR to quit.
8. **Code walkthrough** — 3–4 short annotated snippets: the state-machine dispatch (`main.cpp`), `advanceBootState`, the wallpaper draw + fallback, and the animated Task Manager table. Each snippet ≤25 lines with a sentence explaining it.
9. **Testing** — how to run `ctest`, what the logic units cover.
10. **Credits** — Dr. Neil Patrick Del Gallego; Project Anito; Dear ImGui, GLFW, stb, doctest.

- [ ] **Step 2: Verify Mermaid renders (required)**

Confirm both Mermaid blocks render correctly (broken diagrams visibly hurt the
Documentation score). Check in a Markdown preview and, after pushing, on the
GitHub rendered view. Fix any syntax errors before considering this task done.

- [ ] **Step 3: Commit**
```bash
git add README.md && git commit -m "docs: comprehensive README with diagrams, snippets, and screenshots"
```

### Task 8.3: Repo finalization

- [ ] **Step 1: Set the remote and push**

Run:
```
git remote add origin https://github.com/robbie4116/Desktop-Style-OS-Mockup.git
git push -u origin main
```
> Confirm with the user before pushing (first publish to the remote).

- [ ] **Step 2: Final verification**

Fresh clone in a temp dir, configure/build/run/test to confirm the documented commands work from scratch.
Expected: clean build, tests pass, app runs.

---

## Definition of Done

- [ ] App boots BIOS → splash → desktop and exits only via PWR.
- [ ] Desktop shows a loaded wallpaper image (gradient fallback verified) and a live clock.
- [ ] Taskbar has ≥3 working buttons; two open unique windows; the third opens Task Manager.
- [ ] Task Manager resembles Windows', with a process table (Process/PID/CPU%/Memory) of animated dummy values.
- [ ] All doctest unit tests pass via `ctest`.
- [ ] README covers overview, screenshots, architecture diagrams, build/run, walkthrough with snippets, testing, credits.
- [ ] Builds clean on x64 with the documented commands.
