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
