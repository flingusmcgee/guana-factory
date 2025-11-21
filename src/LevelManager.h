#pragma once
#include "ChunkPath.h"
#include <vector>
#include <cstdint>

/**
 * LODLevel defines parameters for a specific level in the chunk hierarchy.
 * 
 * Each level represents a different scale of chunk organization:
 * - Level 0: Base chunks (8x8x8 blocks) - always visible nearby
 * - Level 1: Chunks of 8 level-0 chunks (64x64x64 blocks effective)
 * - Level 2: Chunks of 8 level-1 chunks (512x512x512 blocks effective)
 * etc.
 * 
 * The loadRadius determines how many chunks away from the player's current
 * chunk at this level should be loaded/active.
 */
struct LODLevel {
    int level;                  // 0, 1, 2, ... (0 is base)
    int chunksPerAxis;          // How many base chunks per axis (8 for level 1, 64 for level 2, etc)
    int blockScale;             // Effective block size (1 for level 0, 8 for level 1, 64 for level 2, etc)
    int loadRadius;             // How many chunks away to load at this level
    int unloadRadius;           // When to unload (should be > loadRadius)
    bool expandChildren;        // Should children be expanded/loaded?
    bool collapseParents;       // Should parent be collapsed/unloaded?
};

/**
 * LevelManager defines and manages a hierarchical chunk level system.
 * 
 * It provides:
 * - Level parameters (chunk scales, load radii)
 * - Expansion/collapse logic (converting parent chunks into child chunks)
 * - Queries about chunk relationships and visibility at different levels
 */
class LevelManager {
public:
    LevelManager();

    /**
     * Get a specific LOD level by index
     */
    const LODLevel& GetLevel(int levelIndex) const;
    
    /**
     * Get the number of defined LOD levels
     */
    int GetLevelCount() const { return levels.size(); }

    /**
     * Get the base (level 0) chunk size in blocks
     */
    int GetBaseChunkSize() const { return 8; }  // Hard-coded 8x8x8 blocks

    /**
     * Convert a chunk path to world coordinates at a specific level
     */
    void GetChunkBounds(const ChunkPath& path, int levelIndex, 
                        int& minX, int& minY, int& minZ,
                        int& maxX, int& maxY, int& maxZ) const;

    /**
     * Get all child paths from a parent path at the next level
     */
    std::vector<ChunkPath> GetChildren(const ChunkPath& parent) const;

    /**
     * Get the parent path (one level up)
     */
    ChunkPath GetParent(const ChunkPath& path) const;

    /**
     * Check if a chunk path should be loaded based on distance from player at a given level
     */
    bool ShouldLoadAtLevel(const ChunkPath& path, const ChunkPath& playerChunk, int levelIndex) const;

    /**
     * Check if a chunk path should be unloaded
     */
    bool ShouldUnloadAtLevel(const ChunkPath& path, const ChunkPath& playerChunk, int levelIndex) const;

    /**
     * Convert world position to chunk path at a specific level
     */
    ChunkPath WorldToChunkPath(float worldX, float worldY, float worldZ, int levelIndex) const;

    /**
     * Set load radius for a specific level (for tuning)
     */
    void SetLoadRadius(int levelIndex, int radius);

private:
    std::vector<LODLevel> levels;

    /**
     * Initialize default level parameters
     */
    void InitializeLevels();

    /**
     * Calculate Manhattan distance between two chunk paths at the same level
     */
    int CalculatePathDistance(const ChunkPath& a, const ChunkPath& b, int levelIndex) const;
};
