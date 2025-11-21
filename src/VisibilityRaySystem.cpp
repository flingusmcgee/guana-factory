#include "VisibilityRaySystem.h"
#include <cmath>

// Helper function to normalize a vector
static Vector3 V3Normalize(Vector3 v) {
    float len = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    if (len > 0.0001f) {
        return {v.x / len, v.y / len, v.z / len};
    }
    return v;
}

// Helper function to subtract two vectors
static Vector3 V3Subtract(Vector3 a, Vector3 b) {
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}

// Helper function to add two vectors
static Vector3 V3Add(Vector3 a, Vector3 b) {
    return {a.x + b.x, a.y + b.y, a.z + b.z};
}

// Helper function to scale a vector
static Vector3 V3Scale(Vector3 v, float s) {
    return {v.x * s, v.y * s, v.z * s};
}

// Helper function to compute cross product
static Vector3 V3Cross(Vector3 a, Vector3 b) {
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

std::unordered_set<uint32_t> VisibilityRaySystem::ComputeVisibleChunks(
    const Camera& camera,
    int chunkSize,
    float maxRayDistance,
    int rayCount
) {
    visibleChunks.clear();
    debugRays.clear();
    debugIntersections.clear();

    // Ensure rayCount is odd for symmetry
    if (rayCount % 2 == 0) rayCount++;
    if (rayCount < 3) rayCount = 3;

    int centerIdx = rayCount / 2;
    float angleStep = 60.0f / (rayCount - 1);  // 60 degree FOV split into steps

    // Compute camera forward direction
    Vector3 forward = V3Normalize(V3Subtract(camera.target, camera.position));
    Vector3 right = V3Normalize(V3Cross(forward, camera.up));
    (void)V3Normalize(V3Cross(right, forward));  // Would be actualUp, currently unused

    // Cast rays in a grid pattern
    for (int i = 0; i < rayCount; ++i) {
        for (int j = 0; j < rayCount; ++j) {
            // Calculate ray direction within view frustum
            float horzAngle = (i - centerIdx) * angleStep;
            float vertAngle = (j - centerIdx) * angleStep;

            // Convert to radians
            float horzRad = horzAngle * 3.14159265f / 180.0f;
            float vertRad = vertAngle * 3.14159265f / 180.0f;

            // Simple rotation: scale forward by angle factor
            // (This is a simplified approximation)
            Vector3 rayDir = forward;
            rayDir.x += sinf(horzRad) * right.x;
            rayDir.z += sinf(horzRad) * right.z;
            rayDir.y += sinf(vertRad) * 0.5f;  // Vertical component
            
            rayDir = V3Normalize(rayDir);

            Vector3 rayEnd = V3Add(camera.position, V3Scale(rayDir, maxRayDistance));
            debugRays.push_back({camera.position, rayEnd});

            // Step through the ray and collect chunks
            Vector3 rayPos = camera.position;
            float distTraveled = 0.0f;
            float stepSize = static_cast<float>(chunkSize);  // Full chunk size steps (was 0.5x)
            int maxSteps = static_cast<int>(maxRayDistance / stepSize) + 2;
            int step = 0;

            while (distTraveled < maxRayDistance && step < maxSteps) {
                // Convert current ray position to chunk path
                ChunkPath chunkPath = WorldToChunkPath(rayPos, chunkSize);
                uint32_t hash = chunkPath.Hash();
                visibleChunks.insert(hash);

                debugIntersections.push_back(rayPos);

                // Step along ray
                rayPos = V3Add(rayPos, V3Scale(rayDir, stepSize));
                distTraveled += stepSize;
                ++step;
            }
        }
    }

    return visibleChunks;
}

ChunkPath VisibilityRaySystem::WorldToChunkPath(const Vector3& pos, int chunkSize) const {
    // Simplified: just create root path for now
    // In a full hierarchical system, this would navigate the chunk tree
    ChunkPath path;
    (void)pos; // currently unused placeholder
    (void)chunkSize; // currently unused placeholder
    
    // For now, just return root - in reality you'd select a child based on position
    // This is a placeholder that will be extended with proper spatial partitioning
    
    return path;
}

void VisibilityRaySystem::RenderDebug(int chunkSize) const {
    if (!debugVisualization) return;
    (void)chunkSize;  // Unused in placeholder implementation

    // Draw rays
    for (const auto& ray : debugRays) {
        DrawLine3D(ray.first, ray.second, BLUE);
    }

    // Draw intersections
    for (const auto& intersection : debugIntersections) {
        DrawSphere(intersection, 0.2f, GREEN);
    }

    // Draw chunk boundaries for visible chunks (placeholder)
    for (const auto& hash : visibleChunks) {
        (void)hash;  // Unused in placeholder
        // In a full implementation, we'd draw bounding boxes for each visible chunk
        // For now, this is just a placeholder
    }
}
