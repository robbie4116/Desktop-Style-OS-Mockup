#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <vector>
#include <cmath>
#include <cstdint>

int main() {
    const int W = 1280, H = 720;
    std::vector<uint8_t> img(W * H * 3);
    auto set = [&](int x, int y, int r, int g, int b) {
        int i = (y * W + x) * 3; img[i]=r; img[i+1]=g; img[i+2]=b;
    };
    for (int y = 0; y < H; ++y) {
        float t = (float)y / H;
        int sr = (int)(70  + t * 120);
        int sg = (int)(130 + t * 100);
        int sb = (int)(200 + t * 50);
        for (int x = 0; x < W; ++x) {
            float hy = 0.70f + 0.05f * std::sin(x * 0.006f) + 0.02f * std::sin(x * 0.02f);
            if (t < hy) set(x, y, sr, sg, sb);
            else {
                float g = (t - hy) / (1.0f - hy);
                set(x, y, (int)(60 + g*30), (int)(140 + g*60), (int)(50 + g*20));
            }
        }
    }
    stbi_write_png("assets/wallpaper.png", W, H, 3, img.data(), W * 3);
    return 0;
}
