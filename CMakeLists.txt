cmake_minimum_required(VERSION 3.16)
project(csopesy_os LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

# --- GLFW ---
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(glfw)

# --- Dear ImGui (not a CMake project; build sources ourselves) ---
FetchContent_Declare(imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.5
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(imgui)

add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp)
target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)
target_link_libraries(imgui PUBLIC glfw)

find_package(OpenGL REQUIRED)

# --- pure logic library (ImGui-free, unit-tested) ---
add_library(csopesy_logic STATIC
    src/logic/clock_format.cpp
    src/logic/boot_state.cpp
    src/logic/process_model.cpp
    src/logic/texture_paths.cpp)
target_include_directories(csopesy_logic PUBLIC src)
target_compile_features(csopesy_logic PUBLIC cxx_std_20)

# --- main application ---
add_executable(csopesy_os
    src/main.cpp
    src/render/boot_screen.cpp
    src/render/splash_screen.cpp
    src/render/desktop.cpp
    src/render/taskbar.cpp
    src/render/texture_loader.cpp
    src/render/windows/file_explorer.cpp
    src/render/windows/settings.cpp
    src/render/windows/task_manager.cpp)
target_include_directories(csopesy_os PRIVATE src third_party)
target_link_libraries(csopesy_os PRIVATE imgui csopesy_logic OpenGL::GL)

# --- tests ---
add_executable(csopesy_tests
    tests/test_main.cpp
    tests/test_clock_format.cpp
    tests/test_boot_state.cpp
    tests/test_process_model.cpp
    tests/test_cpu_history.cpp
    tests/test_texture_paths.cpp)
target_include_directories(csopesy_tests PRIVATE src third_party)
target_link_libraries(csopesy_tests PRIVATE csopesy_logic)
enable_testing()
add_test(NAME unit COMMAND csopesy_tests)

# --- wallpaper generator (standalone, run once) ---
add_executable(wallpaper_gen tools/make_wallpaper.cpp)
target_include_directories(wallpaper_gen PRIVATE third_party)
