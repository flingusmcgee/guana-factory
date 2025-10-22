#pragma once
#include "Chunk.h"
#include "include/raylib.h"
#include <functional>

namespace GreedyMesher {
    // Build a mesh for the given chunk. The function uploads the mesh and stores a Model in chunk->model
    // It returns true on success and false on failure.
    bool MeshChunk(Chunk& chunk);

    // Overload that accepts a sampler function for out-of-bounds/global queries.
    // The sampler is called with global block coordinates (gx,gy,gz) and should return BlockId (0 for air).
    bool MeshChunk(Chunk& chunk, std::function<BlockId(int,int,int)> neighborSampler);
}
