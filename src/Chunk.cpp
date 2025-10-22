#include "Chunk.h"
#include <cstring>

Chunk::Chunk() : blocks(SIZE * SIZE * SIZE, 0) {
    coord = {0,0,0};
    hasModel = false;
    model = { 0 };
}

Chunk::~Chunk() {
    if (hasModel) {
        UnloadModel(model);
        hasModel = false;
    }
}

void Chunk::Init(const ChunkCoord& c) {
    coord = c;
    std::fill(blocks.begin(), blocks.end(), 0);
}

BlockId Chunk::Get(int x, int y, int z) const {
    if (x < 0 || x >= SIZE || y < 0 || y >= SIZE || z < 0 || z >= SIZE) return 0;
    int idx = x + z * SIZE + y * SIZE * SIZE;
    return blocks[idx];
}

void Chunk::Set(int x, int y, int z, BlockId id) {
    if (x < 0 || x >= SIZE || y < 0 || y >= SIZE || z < 0 || z >= SIZE) return;
    int idx = x + z * SIZE + y * SIZE * SIZE;
    blocks[idx] = id;
}
