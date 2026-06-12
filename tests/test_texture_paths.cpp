#include "doctest.h"
#include "logic/texture_paths.h"
using namespace csopesy;

TEST_CASE("candidate paths include exe-relative and cwd-relative options") {
    auto v = wallpaperCandidatePaths("C:/app/bin");
    CHECK(v.size() >= 3);
    CHECK(v.front() == "C:/app/bin/assets/wallpaper.png");
    bool hasCwd = false;
    for (auto& p : v) if (p == "assets/wallpaper.png") hasCwd = true;
    CHECK(hasCwd);
}
TEST_CASE("empty exe dir still yields cwd candidate") {
    auto v = wallpaperCandidatePaths("");
    bool hasCwd = false;
    for (auto& p : v) if (p == "assets/wallpaper.png") hasCwd = true;
    CHECK(hasCwd);
}
