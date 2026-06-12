#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <cstdio>
#include <filesystem>
#include <string>
#include "app_context.h"
#include "logic/boot_state.h"
#include "render/boot_screen.h"
#include "render/splash_screen.h"
#include "render/texture_loader.h"
#include "render/desktop.h"
#include "render/taskbar.h"
#include "render/windows/file_explorer.h"
#include "render/windows/settings.h"
#include "render/windows/task_manager.h"

int main(int argc, char** argv) {
    if (!glfwInit()) { std::fprintf(stderr, "glfwInit failed\n"); return 1; }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWmonitor* mon  = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = mon ? glfwGetVideoMode(mon) : nullptr;
    int w = mode ? static_cast<int>(mode->width  * 0.75f) : 1280;
    int h = mode ? static_cast<int>(mode->height * 0.75f) :  720;
    GLFWwindow* window = glfwCreateWindow(w, h, "CSOPESY Desktop OS Emulator", nullptr, nullptr);
    if (!window) { std::fprintf(stderr, "glfwCreateWindow failed\n"); glfwTerminate(); return 1; }
    glfwSetWindowSizeLimits(window, 800, 500, GLFW_DONT_CARE, GLFW_DONT_CARE);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // center in monitor work-area
    if (mode) {
        int mx = 0, my = 0;
        glfwGetMonitorPos(mon, &mx, &my);
        glfwSetWindowPos(window, mx + (mode->width - w) / 2, my + (mode->height - h) / 2);
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    csopesy::AppContext ctx;
    ctx.stateEnteredTime = glfwGetTime();

    {
        std::string exeDir;
        if (argc > 0) exeDir = std::filesystem::path(argv[0]).parent_path().string();
        csopesy::LoadedTexture wt = csopesy::loadWallpaperTexture(exeDir);
        ctx.wallpaperTex = wt.id;
        ctx.wallpaperLoaded = wt.ok;
    }

    while (!glfwWindowShouldClose(window) && !ctx.shouldShutdown) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // --- input: any click or key skips boot/splash ---
        // NOTE: v1.91.5 removed the legacy io.KeysDown[]/io.MouseClicked[] IO arrays.
        // Use the function-based event API exclusively.
        ctx.skipRequested = ImGui::IsMouseClicked(ImGuiMouseButton_Left) ||
                            ImGui::IsKeyPressed(ImGuiKey_Space, false) ||
                            ImGui::IsKeyPressed(ImGuiKey_Enter, false) ||
                            ImGui::IsKeyPressed(ImGuiKey_Escape, false);

        // --- advance state machine ---
        double now = glfwGetTime();
        double elapsed = now - ctx.stateEnteredTime;
        csopesy::AppState next = csopesy::advanceBootState(
            ctx.state, elapsed, ctx.skipRequested, ctx.timings);
        if (next != ctx.state) { ctx.state = next; ctx.stateEnteredTime = now; }

        // Elapsed time WITHIN the current state, valid after a possible transition
        // reset above (so a freshly-entered state starts at ~0.0 this frame).
        double stateElapsed = now - ctx.stateEnteredTime;

        // --- per-state rendering ---
        switch (ctx.state) {
            case csopesy::AppState::Bios:    csopesy::renderBootScreen(ctx, stateElapsed); break;
            case csopesy::AppState::Splash:  csopesy::renderSplash(ctx, stateElapsed); break;
            case csopesy::AppState::Desktop:
                csopesy::renderDesktopBackground(ctx);
                csopesy::renderTaskbar(ctx);
                csopesy::renderFileExplorer(ctx);
                csopesy::renderSettings(ctx);
                csopesy::renderTaskManager(ctx);
                break;
        }

        ImGui::Render();
        int w, h; glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);
        glClearColor(0.10f, 0.10f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    if (ctx.wallpaperTex) { GLuint t = ctx.wallpaperTex; glDeleteTextures(1, &t); }
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
