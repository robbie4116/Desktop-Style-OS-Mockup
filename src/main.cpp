#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <cstdio>
#include "app_context.h"
#include "logic/boot_state.h"
#include "render/boot_screen.h"

int main() {
    if (!glfwInit()) { std::fprintf(stderr, "glfwInit failed\n"); return 1; }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "CSOPESY Desktop OS Emulator", nullptr, nullptr);
    if (!window) { std::fprintf(stderr, "glfwCreateWindow failed\n"); glfwTerminate(); return 1; }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    csopesy::AppContext ctx;
    ctx.stateEnteredTime = glfwGetTime();

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

        // --- placeholder rendering (replaced in later chunks) ---
        switch (ctx.state) {
            case csopesy::AppState::Bios:    csopesy::renderBootScreen(ctx, stateElapsed); break;
            case csopesy::AppState::Splash:  /* renderSplash(ctx, stateElapsed) */ break;
            case csopesy::AppState::Desktop: /* renderDesktop / taskbar / windows */ break;
        }
        if (ctx.state == csopesy::AppState::Desktop) {
            ImGui::SetNextWindowPos(ImVec2(20, 20));
            ImGui::Begin("state", nullptr, ImGuiWindowFlags_NoResize|ImGuiWindowFlags_AlwaysAutoResize);
            const char* names[] = {"BIOS", "SPLASH", "DESKTOP"};
            ImGui::Text("state: %s  elapsed: %.1f", names[(int)ctx.state], stateElapsed);
            if (ImGui::Button("PWR (quit)")) ctx.shouldShutdown = true;
            ImGui::End();
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
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
