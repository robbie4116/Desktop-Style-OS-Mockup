#include "render/texture_loader.h"
#include "logic/texture_paths.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <GLFW/glfw3.h>   // pulls GL types + GL 1.1 texture functions

// GL_CLAMP_TO_EDGE is GL 1.2; not exported by opengl32 on Windows, define manually.
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

namespace csopesy {
LoadedTexture loadWallpaperTexture(const std::string& exeDir) {
    LoadedTexture out;
    for (const auto& path : wallpaperCandidatePaths(exeDir)) {
        int w, h, comp;
        unsigned char* px = stbi_load(path.c_str(), &w, &h, &comp, 4);
        if (!px) continue;
        GLuint tex = 0;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, px);
        stbi_image_free(px);
        out = {static_cast<unsigned>(tex), w, h, true};
        return out;
    }
    return out; // ok=false
}
}
