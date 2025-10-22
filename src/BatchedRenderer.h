#pragma once
#include "include/raylib.h"

namespace BatchedRenderer {
    // Initialize renderer (optional)
    void Init();
    // Draw visible entities by grouping models and using DODStorage as the source of positions/colors
    void RenderAll(const Camera& cam);
}
