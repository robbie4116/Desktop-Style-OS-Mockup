#include "render/windows/file_explorer.h"
#include "app_context.h"
#include "imgui.h"

namespace csopesy {

struct FileEntry { const char* name; const char* type; const char* size; };

void renderFileExplorer(AppContext& ctx) {
    if (!ctx.openFileExplorer) return;
    ImGui::SetNextWindowSize(ImVec2(560, 360), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("File Explorer", &ctx.openFileExplorer)) { ImGui::End(); return; }

    static int selected = 0;
    const char* folders[] = {"Desktop", "Documents", "Downloads", "Games", "System"};

    // v1.91.5: BeginChild's 3rd arg is ImGuiChildFlags, not bool. Use the
    // Borders flag (passing `true`/1 would NOT set the border bit, which is 1<<1).
    ImGui::BeginChild("tree", ImVec2(160, 0), ImGuiChildFlags_Borders);
    for (int i = 0; i < 5; ++i)
        if (ImGui::Selectable(folders[i], selected == i)) selected = i;
    ImGui::EndChild();

    ImGui::SameLine();
    ImGui::BeginChild("files", ImVec2(0, 0), ImGuiChildFlags_Borders);
    ImGui::Text("This PC > %s", folders[selected]);
    ImGui::Separator();
    static const FileEntry sets[5][3] = {
        {{"readme.txt","Text","2 KB"},{"shortcut.lnk","Shortcut","1 KB"},{"notes.md","Markdown","4 KB"}},
        {{"resume.docx","Document","18 KB"},{"budget.xlsx","Spreadsheet","22 KB"},{"thesis.pdf","PDF","1.2 MB"}},
        {{"setup.exe","Application","44 MB"},{"image.png","Image","820 KB"},{"song.mp3","Audio","5.1 MB"}},
        {{"anito.exe","Application","210 MB"},{"ddr_sim.exe","Application","88 MB"},{"save01.dat","Data","64 KB"}},
        {{"kernel32.dll","System","1.1 MB"},{"config.sys","System","1 KB"},{"hosts","System","1 KB"}},
    };
    if (ImGui::BeginTable("filelist", 3,
            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable)) {
        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupColumn("Type");
        ImGui::TableSetupColumn("Size");
        ImGui::TableHeadersRow();
        for (int i = 0; i < 3; ++i) {
            const FileEntry& e = sets[selected][i];
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted(e.name);
            ImGui::TableSetColumnIndex(1); ImGui::TextUnformatted(e.type);
            ImGui::TableSetColumnIndex(2); ImGui::TextUnformatted(e.size);
        }
        ImGui::EndTable();
    }
    ImGui::EndChild();
    ImGui::End();
}
}
