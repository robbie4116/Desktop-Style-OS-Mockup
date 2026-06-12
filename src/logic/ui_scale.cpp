#include "logic/ui_scale.h"
#include <algorithm>

namespace csopesy {

float calculateUiScale(int width, int height) {
    const float widthScale = static_cast<float>(width) / 1280.0f;
    const float heightScale = static_cast<float>(height) / 720.0f;
    return std::clamp(std::min(widthScale, heightScale), 1.0f, 1.5f);
}

float calculateTaskbarIconSize(float uiScale) {
    return 11.0f * uiScale;
}

}
