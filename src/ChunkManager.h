#pragma once
#include "include/raylib.h"
#include <vector>
#include "ChunkRegistry.h"

namespace ChunkManager {
    void Init();
    void Shutdown();
    void Render(const Camera& cam);
    ChunkRegistry& GetRegistry();
}