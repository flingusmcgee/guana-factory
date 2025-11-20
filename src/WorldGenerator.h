#pragma once

// Minimal world generator interface used by Chunk/ChunkManager/HeightmapGenerator

// Small integer type for blocks; 0 == air
using BlockId = unsigned char;

struct ChunkCoord {
    int x;
    int y;
    int z;
};

class WorldGenerator {
public:
    virtual ~WorldGenerator() {}
    // Fill outBlocks (chunkSize^3 entries) for the chunk at coord
    virtual void GenerateChunk(const ChunkCoord& coord, BlockId* outBlocks, int chunkSize) = 0;
};
