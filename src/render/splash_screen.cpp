#include "render/splash_screen.h"
#include "app_context.h"
#include "imgui.h"
#include <algorithm>

namespace csopesy {
void renderSplash(const AppContext& ctx, double elapsed) {
    const ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->Pos);
    ImGui::SetNextWindowSize(vp->Size);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0,0,0,1));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
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
    center("Created By: S09 Group 2 - Cumti, Dulatre, Hong, Pineda", 1.0f, ImVec4(0.7f,0.7f,0.7f,1), 0.57f);

    int dots = (static_cast<int>(elapsed / 0.4) % 4);
    char loading[16] = "Loading";
    for (int i = 0; i < dots; ++i) loading[7 + i] = '.';
    loading[7 + dots] = '\0';
    center(loading, 1.1f, ImVec4(0.4f,0.8f,0.4f,1.0f), 0.80f);

    // Progress bar — fills from left to right over splashDuration seconds
    float progress = std::clamp(
        static_cast<float>(elapsed / ctx.timings.splashDuration), 0.0f, 1.0f);
    const float barW = 260.0f, barH = 3.0f;
    ImVec2 barMin(vp->Pos.x + (vp->Size.x - barW) * 0.5f,
                  vp->Pos.y + vp->Size.y * 0.87f);
    ImVec2 barMax(barMin.x + barW, barMin.y + barH);
    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddRectFilled(barMin, barMax, IM_COL32(40, 40, 40, 255));
    dl->AddRectFilled(barMin,
                      ImVec2(barMin.x + barW * progress, barMax.y),
                      IM_COL32(80, 230, 120, 255));

    ImGui::End();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();
}
}
