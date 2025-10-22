#pragma once
#include "WorldGenerator.h"
#include "include/raylib.h"
#include <vector>
#include <string>

class Chunk {
public:
    static const int SIZE = 8; // blocks per axis (reduced to avoid 16-bit index overflow in prototype)
    Chunk();
    ~Chunk();

    void Init(const ChunkCoord& coord);
    BlockId Get(int x, int y, int z) const;
    void Set(int x, int y, int z, BlockId id);
    const ChunkCoord& GetCoord() const { return coord; }

    // Mesh/model produced by meshing (may be empty)
    Model model;
    bool hasModel = false;

private:
    ChunkCoord coord;
    std::vector<BlockId> blocks; // SIZE^3
};
