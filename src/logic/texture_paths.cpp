#include "logic/texture_paths.h"
namespace csopesy {
std::vector<std::string> wallpaperCandidatePaths(const std::string& exeDir) {
    std::vector<std::string> v;
    if (!exeDir.empty()) {
        v.push_back(exeDir + "/assets/wallpaper.png");
        v.push_back(exeDir + "/../assets/wallpaper.png");
        v.push_back(exeDir + "/../../assets/wallpaper.png");
    }
    v.push_back("assets/wallpaper.png");
    v.push_back("../assets/wallpaper.png");
    return v;
}
}
