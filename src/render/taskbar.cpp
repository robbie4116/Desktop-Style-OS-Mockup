#include "render/taskbar.h"
#include "app_context.h"
#include "logic/ui_scale.h"
#include "imgui.h"
#include <cmath>

namespace csopesy {

enum class IconType { Start, Files, Settings, TaskMgr, Vol, Net, Pwr };

static void drawIcon(ImDrawList* dl, IconType type, ImVec2 c, float sz, ImU32 color) {
    const float lw = 1.5f * (sz / 11.0f);
    switch (type) {

    case IconType::Start: {
        float s = sz * 0.40f;
        float g = sz * 0.10f;
        for (int row = 0; row < 2; ++row)
        for (int col = 0; col < 2; ++col) {
            float x = c.x + (col == 0 ? -(s * 2.0f + g) : g);
            float y = c.y + (row == 0 ? -(s * 2.0f + g) : g);
            dl->AddRect(ImVec2(x, y), ImVec2(x + s * 2.0f, y + s * 2.0f),
                        color, 0.0f, 0, lw);
        }
        break;
    }

    case IconType::Files: {
        float hw = sz * 0.90f, hh = sz * 0.55f;
        float tw = sz * 0.42f, th = sz * 0.22f;
        ImVec2 bMin(c.x - hw, c.y - hh + th);
        ImVec2 bMax(c.x + hw, c.y + hh);
        dl->AddRect(bMin, bMax, color, 0.0f, 0, lw);
        ImVec2 tTL(c.x - hw,        c.y - hh);
        ImVec2 tTR(c.x - hw + tw,   c.y - hh);
        ImVec2 tBR(c.x - hw + tw,   c.y - hh + th);
        dl->AddLine(tTL, tTR, color, lw);
        dl->AddLine(tTL, ImVec2(tTL.x, tBR.y), color, lw);
        dl->AddLine(tTR, tBR, color, lw);
        break;
    }

    case IconType::Settings: {
        float rCirc  = sz * 0.28f;
        float rInner = sz * 0.45f;
        float rOuter = sz * 0.72f;
        dl->AddCircle(c, rCirc, color, 0, lw);
        for (int i = 0; i < 8; ++i) {
            float a = i * (3.14159265f * 0.25f);
            dl->AddLine(
                ImVec2(c.x + cosf(a) * rInner, c.y + sinf(a) * rInner),
                ImVec2(c.x + cosf(a) * rOuter, c.y + sinf(a) * rOuter),
                color, lw);
        }
        break;
    }

    case IconType::TaskMgr: {
        const float bw   = sz * 0.20f;
        const float base = c.y + sz * 0.58f;
        const float xOff[3]    = { -sz * 0.50f, 0.0f, sz * 0.50f };
        const float heights[3] = {  sz * 0.55f, sz * 0.85f, sz * 1.15f };
        for (int i = 0; i < 3; ++i)
            dl->AddRectFilled(
                ImVec2(c.x + xOff[i] - bw, base - heights[i]),
                ImVec2(c.x + xOff[i] + bw, base), color);
        break;
    }

    case IconType::Vol: {
        float bLeft = c.x - sz * 0.68f, bRight = c.x - sz * 0.30f;
        float bTop  = c.y - sz * 0.34f, bBot   = c.y + sz * 0.34f;
        float cTipX = c.x + sz * 0.18f;
        float cTopY = c.y - sz * 0.62f, cBotY  = c.y + sz * 0.62f;
        dl->AddRect(ImVec2(bLeft, bTop), ImVec2(bRight, bBot), color, 0.0f, 0, lw);
        dl->AddLine(ImVec2(bRight, bTop), ImVec2(cTipX, cTopY), color, lw);
        dl->AddLine(ImVec2(bRight, bBot), ImVec2(cTipX, cBotY), color, lw);
        dl->AddLine(ImVec2(cTipX, cTopY), ImVec2(cTipX, cBotY), color, lw);
        float ax = c.x + sz * 0.34f;
        dl->AddLine(ImVec2(ax, c.y - sz * 0.25f), ImVec2(ax + sz * 0.32f, c.y - sz * 0.52f), color, lw);
        dl->AddLine(ImVec2(ax, c.y + sz * 0.25f), ImVec2(ax + sz * 0.32f, c.y + sz * 0.52f), color, lw);
        break;
    }

    case IconType::Net: {
        const float bw   = sz * 0.20f;
        const float base = c.y + sz * 0.58f;
        const float xOff[3]    = { -sz * 0.50f, 0.0f, sz * 0.50f };
        const float heights[3] = {  sz * 0.42f, sz * 0.78f, sz * 1.15f };
        for (int i = 0; i < 3; ++i)
            dl->AddRectFilled(
                ImVec2(c.x + xOff[i] - bw, base - heights[i]),
                ImVec2(c.x + xOff[i] + bw, base), color);
        break;
    }

    case IconType::Pwr: {
        float r = sz * 0.68f;
        float gapA = 0.58f;
        float startA = -3.14159265f * 0.5f + gapA;
        float endA   =  3.14159265f * 1.5f - gapA;
        dl->PathArcTo(c, r, startA, endA, 32);
        dl->PathStroke(color, false, lw);
        dl->AddLine(ImVec2(c.x, c.y - sz * 0.82f),
                    ImVec2(c.x, c.y - r * 0.85f), color, lw);
        break;
    }

    } // switch
}

static bool tbIconButton(const char* id, IconType type, ImVec4 bg, float width, float scale,
                          ImU32 iconColor = IM_COL32(200, 215, 235, 220)) {
    ImVec2 pos  = ImGui::GetCursorScreenPos();
    ImVec2 size(width * scale, 40.0f * scale);
    bool clicked = ImGui::InvisibleButton(id, size);
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec4 tinted = bg;
    if      (ImGui::IsItemActive())   { tinted.x += 0.15f; tinted.y += 0.15f; tinted.z += 0.15f; }
    else if (ImGui::IsItemHovered())  { tinted.x += 0.08f; tinted.y += 0.08f; tinted.z += 0.08f; }
    dl->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y),
                      ImGui::ColorConvertFloat4ToU32(tinted));
    drawIcon(dl, type,
             ImVec2(pos.x + size.x * 0.5f, pos.y + size.y * 0.5f),
             calculateTaskbarIconSize(scale), iconColor);
    return clicked;
}

void renderTaskbar(AppContext& ctx) {
    const ImGuiViewport* vp = ImGui::GetMainViewport();
    const float s = ctx.uiScale;
    const float barH = 56.0f * s;
    ImGui::SetNextWindowPos(ImVec2(vp->Pos.x, vp->Pos.y + vp->Size.y - barH));
    ImGui::SetNextWindowSize(ImVec2(vp->Size.x, barH));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.07f,0.07f,0.10f,0.96f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::Begin("##taskbar", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus);

    tbIconButton("##start", IconType::Start, ImVec4(0.20f,0.45f,0.20f,1), 44.0f, s); // decorative
    ImGui::SameLine();
    if (tbIconButton("##files", IconType::Files, ImVec4(0.18f,0.30f,0.50f,1), 44.0f, s)) ctx.openFileExplorer = !ctx.openFileExplorer;
    ImGui::SameLine();
    if (tbIconButton("##set", IconType::Settings, ImVec4(0.30f,0.30f,0.40f,1), 44.0f, s)) ctx.openSettings = !ctx.openSettings;
    ImGui::SameLine();
    if (tbIconButton("##task", IconType::TaskMgr, ImVec4(0.45f,0.30f,0.20f,1), 44.0f, s)) ctx.openTaskManager = !ctx.openTaskManager;

    // Status label — centered in the gap between left cluster and right cluster
    ImGui::SameLine();
    ImGui::SetCursorPosY((barH - ImGui::GetTextLineHeight()) * 0.5f);
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(80, 200, 100, 180));
    ImGui::TextUnformatted("CSOPESY OS v1.0 - System Online");
    ImGui::PopStyleColor();

    // right-aligned cluster: VOL  NET  PWR  (56 px each — down from 64)
    const float rBtnW = 56.0f * s;
    const float spacing = ImGui::GetStyle().ItemSpacing.x;
    float regionRight = ImGui::GetWindowContentRegionMax().x;
    float rightX = regionRight - (rBtnW * 3.0f + spacing * 2.0f);
    ImGui::SameLine(rightX);
    // VOL/NET use dark blue-gray approximating the default ImGui dark-style button
    if (tbIconButton("##vol", IconType::Vol, ImVec4(0.18f,0.25f,0.40f,1), 56.0f, s)) {
        ctx.openSettings = true;
        ctx.requestedSettingsTab = SettingsTab::Sound;
        ctx.settingsTabRequested = true;
    }
    ImGui::SameLine();
    tbIconButton("##net", IconType::Net, ImVec4(0.18f,0.25f,0.40f,1), 56.0f, s);  // decorative
    ImGui::SameLine();
    if (tbIconButton("##pwr", IconType::Pwr, ImVec4(0.6f,0.15f,0.15f,1), 56.0f, s,
                     IM_COL32(255, 175, 175, 220)))
        ctx.shouldShutdown = true;

    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}
}
