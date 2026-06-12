# Adaptive UI Scaling Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Apply a shared, monitor-aware UI scale to every screen and OS utility.

**Architecture:** A pure `ui_scale` logic unit calculates a clamped scale from
the initial client size. `main.cpp` stores it in `AppContext` and configures
ImGui globally; renderers multiply custom geometry and explicit dimensions by
the same factor.

**Tech Stack:** C++20, Dear ImGui, GLFW, doctest, CMake

---

## Chunk 1: Scale Model

### Task 1: Add tested scale calculation

**Files:**
- Create: `src/logic/ui_scale.h`
- Create: `src/logic/ui_scale.cpp`
- Create: `tests/test_ui_scale.cpp`
- Modify: `CMakeLists.txt`
- Modify: `src/app_context.h`

- [x] Write tests for baseline, minimum, maximum, and limiting dimension.
- [x] Run tests and verify they fail because `calculateUiScale` is missing.
- [x] Implement `calculateUiScale` and store `uiScale` in `AppContext`.
- [x] Run tests and verify they pass.

## Chunk 2: Global and Custom Rendering

### Task 2: Apply scale across all renderers

**Files:**
- Modify: `src/main.cpp`
- Modify: `src/render/boot_screen.cpp`
- Modify: `src/render/splash_screen.cpp`
- Modify: `src/render/desktop.cpp`
- Modify: `src/render/taskbar.cpp`
- Modify: `src/render/windows/file_explorer.cpp`
- Modify: `src/render/windows/settings.cpp`
- Modify: `src/render/windows/task_manager.cpp`

- [x] Configure ImGui font and style scale once during startup.
- [x] Scale BIOS and splash custom dimensions.
- [x] Scale desktop clock and taskbar custom dimensions.
- [x] Scale explicit utility-window, pane, graph, and table dimensions.
- [x] Build and run all tests.
- [x] Launch and inspect all states and utilities.
