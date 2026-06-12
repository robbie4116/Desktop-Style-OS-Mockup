#pragma once
#include <string>
namespace csopesy {
struct LoadedTexture { unsigned id = 0; int width = 0; int height = 0; bool ok = false; };
// Tries each candidate path (see logic/texture_paths). Returns ok=false if none load.
LoadedTexture loadWallpaperTexture(const std::string& exeDir);
}
