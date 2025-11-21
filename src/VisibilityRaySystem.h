#pragma once
#include "include/raylib.h"
#include "ChunkPath.h"
#include <vector>
#include <unordered_set>

/**
 * VisibilityRaySystem implements frustum-based chunk visibility culling.
 * 
 * It casts rays from the camera position in multiple directions to determine
 * which chunks are potentially visible. This is used to:
 * - Avoid rendering/loading chunks that are outside the view
 * - Prioritize loading chunks directly in front of the camera
 * - Enable progressive level-of-detail loading
 * 
 * The system produces a set of "visible" chunk paths that the renderer
 * and chunk manager can use for rendering and loading decisions.
 */
class VisibilityRaySystem {
public:
    VisibilityRaySystem() = default;

    /**
     * Cast rays from camera and update visibility set.
     * 
     * Parameters:
     * - camera: The 3D camera to cast rays from
     * - chunkSize: Size of each chunk (e.g., 8 blocks)
     * - maxRayDistance: How far rays should travel before stopping (in world units)
     * - rayCount: Number of rays to cast in a grid pattern (should be odd, e.g., 5, 7, 9)
     * 
     * Returns the set of ChunkPath hashes that are visible
     */
    std::unordered_set<uint32_t> ComputeVisibleChunks(
        const Camera& camera,
        int chunkSize,
        float maxRayDistance,
        int rayCount = 5
    );

    /**
     * Get the most recently computed visible chunks
     */
    const std::unordered_set<uint32_t>& GetVisibleChunks() const { return visibleChunks; }

    /**
     * Clear visibility set (all chunks will be culled)
     */
    void Clear() { visibleChunks.clear(); }

    /**
     * Enable/disable debug visualization (draws rays and chunks as lines)
     */
    void SetDebugVisualization(bool enabled) { debugVisualization = enabled; }
    bool IsDebugVisualizationEnabled() const { return debugVisualization; }

    /**
     * Render debug rays and visible chunk bounding boxes
     */
    void RenderDebug(int chunkSize) const;

private:
    std::unordered_set<uint32_t> visibleChunks;
    bool debugVisualization = false;
    
    // Store debug rays for visualization
    std::vector<std::pair<Vector3, Vector3>> debugRays;
    std::vector<Vector3> debugIntersections;
    
    /**
     * Convert a world position to a chunk path (simplified for this prototype)
     * In the full system, this would navigate the hierarchical chunk tree
     */
    ChunkPath WorldToChunkPath(const Vector3& pos, int chunkSize) const;
};
