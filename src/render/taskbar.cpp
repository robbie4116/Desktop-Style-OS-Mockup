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
