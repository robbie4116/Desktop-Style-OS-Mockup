#include "render/boot_screen.h"
#include "app_context.h"
#include "imgui.h"

namespace csopesy {

static const char* kLines[] = {
    "CSOPESY Megatrends",
    "Released: 12/01/94",
    "DLSU GAME Lab (C)1994 CSOPESY Megatrends Inc.",
    "",
    "Memory Test:",
    "Checking RAM : 64000K OK",
    "",
    "CPU Type: Pentium III",
    "BIOS Version: BCN SIT 1989-1994 Special CSOPESY",
    "Main Processor: Pentium III",
    "Numeric Processor: Built-in",
    "",
    "Primary Master:   420 MB WDCAC2420H",
    "Primary Slave:    None",
    "Secondary Master: CD-ROM LTN-305A",
    "Secondary Slave:  None",
    "",
    "Fun Fact: The CPU specs are a tribute to the Bemani Python 2 Engine,",
    "the OS that powers up Dance Dance Revolution SuperNova series.",
    "It was essentially a retail Sony PlayStation 2 inside a metal box.",
};

void renderBootScreen(const AppContext&, double elapsed) {
    const ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->Pos);
    ImGui::SetNextWindowSize(vp->Size);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0,0,0,1));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::Begin("##bios", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus);

    // ~0.12s per line so all 20 lines reveal within the 3.0s BIOS duration.
    int reveal = static_cast<int>(elapsed / 0.12) + 1;
    int total = (int)(sizeof(kLines)/sizeof(kLines[0]));
    if (reveal > total) reveal = total;

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.85f, 0.85f, 1.0f));
    for (int i = 0; i < reveal; ++i) ImGui::TextUnformatted(kLines[i]);
    ImGui::PopStyleColor();

    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}
}
