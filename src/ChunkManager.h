#pragma once
#include "include/raylib.h"
#include <vector>

namespace ChunkManager {
    void Init();
    void Shutdown();
    void Render(const Camera& cam);
}
