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
