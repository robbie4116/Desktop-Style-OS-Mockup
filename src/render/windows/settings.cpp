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
    const float s = ctx.uiScale;
    ImGui::SetNextWindowSize(ImVec2(460.0f * s, 320.0f * s), ImGuiCond_Once);
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
            ImGui::TextDisabled("Created by S09 Group 2");
            ImGui::TextDisabled("Cumti, Dulatre, Hong, Pineda");
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ctx.settingsTabRequested = false; // consume the one-shot request
    ImGui::End();
}
}
