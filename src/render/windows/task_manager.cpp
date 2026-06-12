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

    // advance the dummy values every frame the Task Manager is toggled open
    // (continues even when the window is collapsed to its title bar)
    for (auto& p : ctx.processes) updateProcess(p);
    float cpu = aggregateCpu(ctx.processes);
    float memTotal = 8192.0f; // pretend 8 GB
    float memPct = (aggregateMemMB(ctx.processes) / memTotal) * 100.0f;
    if (memPct > 100.0f) memPct = 100.0f;   // keep the readout plausible
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
