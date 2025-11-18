#include "LevelManager.h"
#include "Log.h"
#include <algorithm>
#include <cmath>
#include <vector>

LevelManager::LevelManager() {
    InitializeLevels();
}

void LevelManager::InitializeLevels() {
    levels.clear();

    // Level 0: Base chunks (8x8x8 blocks)
    levels.push_back({
        .level = 0,
        .chunksPerAxis = 1,
        .blockScale = 1,
        .loadRadius = 1,          // Load chunks immediately adjacent
        .unloadRadius = 3,
        .expandChildren = false,
        .collapseParents = false
    });

    // Level 1: Mid-level chunks (64x64x64 blocks effective)
    levels.push_back({
        .level = 1,
        .chunksPerAxis = 8,
        .blockScale = 8,
        .loadRadius = 2,          // Load 2 chunks away (16x16x16 blocks)
        .unloadRadius = 4,
        .expandChildren = true,
        .collapseParents = false
    });

    // Level 2: Large chunks (512x512x512 blocks effective)
    levels.push_back({
        .level = 2,
        .chunksPerAxis = 64,
        .blockScale = 64,
        .loadRadius = 1,          // Load only the player's chunk and immediate neighbors
        .unloadRadius = 3,
        .expandChildren = true,
        .collapseParents = true
    });

    Log::Info("LevelManager::InitializeLevels - Initialized " + std::to_string(levels.size()) + " LOD levels");
}

const LODLevel& LevelManager::GetLevel(int levelIndex) const {
    if (levelIndex < 0 || levelIndex >= static_cast<int>(levels.size())) {
        Log::Error("LevelManager::GetLevel - Invalid level index: " + std::to_string(levelIndex));
        return levels[0];
    }
    return levels[levelIndex];
}

void LevelManager::GetChunkBounds(const ChunkPath& path, int levelIndex,
                                   int& minX, int& minY, int& minZ,
                                   int& maxX, int& maxY, int& maxZ) const {
    (void)path; // Simplified implementation does not use 'path' yet
    const LODLevel& level = GetLevel(levelIndex);
    int baseSize = GetBaseChunkSize();
    int chunkWorldSize = baseSize * level.blockScale;

    // Simplified: assume path encodes coordinates directly
    // In a full implementation, we'd properly decode the hierarchical path
    minX = 0;
    minY = 0;
    minZ = 0;
    maxX = chunkWorldSize;
    maxY = chunkWorldSize;
    maxZ = chunkWorldSize;
}

std::vector<ChunkPath> LevelManager::GetChildren(const ChunkPath& parent) const {
    std::vector<ChunkPath> children;
    
    // Generate 8 children (one for each octant)
    for (int x = 0; x < 2; ++x) {
        for (int y = 0; y < 2; ++y) {
            for (int z = 0; z < 2; ++z) {
                ChunkPath child = parent;
                child.Append(x * 4, y * 4, z * 4);  // Use corners of octant
                children.push_back(child);
            }
        }
    }

    return children;
}

ChunkPath LevelManager::GetParent(const ChunkPath& path) const {
    return path.GetParent();
}

bool LevelManager::ShouldLoadAtLevel(const ChunkPath& path, const ChunkPath& playerChunk, int levelIndex) const {
    if (levelIndex < 0 || levelIndex >= static_cast<int>(levels.size())) {
        return false;
    }

    const LODLevel& level = levels[levelIndex];
    int distance = CalculatePathDistance(path, playerChunk, levelIndex);
    
    return distance <= level.loadRadius;
}

bool LevelManager::ShouldUnloadAtLevel(const ChunkPath& path, const ChunkPath& playerChunk, int levelIndex) const {
    if (levelIndex < 0 || levelIndex >= static_cast<int>(levels.size())) {
        return true;  // Unload if level is invalid
    }

    const LODLevel& level = levels[levelIndex];
    int distance = CalculatePathDistance(path, playerChunk, levelIndex);
    
    return distance > level.unloadRadius;
}

ChunkPath LevelManager::WorldToChunkPath(float worldX, float worldY, float worldZ, int levelIndex) const {
    if (levelIndex < 0 || levelIndex >= static_cast<int>(levels.size())) {
        return ChunkPath();  // Root
    }

    const LODLevel& level = levels[levelIndex];
    int baseSize = GetBaseChunkSize();
    (void)baseSize;  // Unused in simplified implementation
    (void)worldX;   // Unused in simplified implementation
    (void)worldY;
    (void)worldZ;
    (void)level.blockScale;

    // Simplified path generation
    ChunkPath path;
    
    // For now, just return root path
    // A full implementation would properly navigate the hierarchy
    
    return path;
}

void LevelManager::SetLoadRadius(int levelIndex, int radius) {
    if (levelIndex >= 0 && levelIndex < static_cast<int>(levels.size())) {
        levels[levelIndex].loadRadius = radius;
        Log::Info("LevelManager::SetLoadRadius - Set radius for level " + std::to_string(levelIndex) + " to " + std::to_string(radius));
    }
}

int LevelManager::CalculatePathDistance(const ChunkPath& a, const ChunkPath& b, int levelIndex) const {
    (void)levelIndex; // levelIndex unused in simplified calculation
    // Simplified: if paths are equal, distance is 0; otherwise return 1
    // A full implementation would properly calculate Manhattan distance in the hierarchy
    return (a == b) ? 0 : 1;
}
