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

    // settings tab routing (VOL button -> Sound tab).
    // settingsTabRequested is a one-shot: renderSettings() applies the requested
    // tab once and then clears it back to false, so it must not be left set.
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
