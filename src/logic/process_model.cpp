#include "logic/process_model.h"
#include <algorithm>
namespace csopesy {

float nextUnit(unsigned& state) {
    state = state * 1664525u + 1013904223u;
    return static_cast<float>(state >> 8) / 16777216.0f;
}

static float clampf(float v, float lo, float hi) {
    return std::max(lo, std::min(hi, v));
}

void updateProcess(ProcessRow& row) {
    float dCpu = (nextUnit(row.rng) - 0.5f) * 4.0f;
    float dMem = (nextUnit(row.rng) - 0.5f) * 8.0f;
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
        {"game_engine.exe",   3001, 27.3f, 412.0f, 606u},
        {"audiodg.exe",       1190,  0.8f,  18.0f, 707u},
        {"svchost.exe",       980,   2.0f,  76.0f, 808u},
        {"taskmgr.exe",       2780,  4.5f,  52.0f, 909u},
        {"notepad.exe",       3140,  0.3f,  12.0f, 110u},
    };
}
}
