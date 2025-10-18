#include "CollisionSystem.h"
#include "EventManager.h"
#include <string>
#include <cstdint>
#include <cmath>
#include <unordered_map>
#include <vector>

// Spatial hash broad-phase parameters
static const float CELL_SIZE = 1.0f; // world units per cell; tune as needed

// Helper to hash 2D cell coordinates into a single int64 key
static inline int64_t CellKey(int x, int y) {
    return (static_cast<int64_t>(x) << 32) ^ static_cast<uint32_t>(y);
}

// Check all active entities for collisions and fire events using spatial hashing
void CollisionSystem::CheckCollisions(std::vector<Entity>& entities) {
    std::unordered_map<int64_t, std::vector<size_t>> buckets;
    buckets.reserve(entities.size() * 2);

    // Insert entities into buckets based on their AABB centers
    for (size_t i = 0; i < entities.size(); ++i) {
        if (!entities[i].is_active) continue;
        Vector3 c = {
            (entities[i].bounds.min.x + entities[i].bounds.max.x) * 0.5f,
            (entities[i].bounds.min.y + entities[i].bounds.max.y) * 0.5f,
            (entities[i].bounds.min.z + entities[i].bounds.max.z) * 0.5f
        };
        int cellX = static_cast<int>(std::floor(c.x / CELL_SIZE));
        int cellZ = static_cast<int>(std::floor(c.z / CELL_SIZE));
        int64_t key = CellKey(cellX, cellZ);
        buckets[key].push_back(i);
    }

    // For each occupied cell, test entities within the cell and neighboring cells
    for (const auto &entry : buckets) {
        int64_t key = entry.first;
        int cellX = static_cast<int>(key >> 32);
        int cellZ = static_cast<int>(static_cast<uint32_t>(key & 0xffffffff));

        for (int dx = -1; dx <= 1; ++dx) {
            for (int dz = -1; dz <= 1; ++dz) {
                int64_t neighborKey = CellKey(cellX + dx, cellZ + dz);
                auto it = buckets.find(neighborKey);
                if (it == buckets.end()) continue;

                const std::vector<size_t>& a = entry.second;
                const std::vector<size_t>& b = it->second;

                for (size_t ia = 0; ia < a.size(); ++ia) {
                    for (size_t ib = 0; ib < b.size(); ++ib) {
                        size_t i = a[ia];
                        size_t j = b[ib];
                        if (i >= j) continue; // avoid duplicate checks and self
                        if (!entities[i].is_active || !entities[j].is_active) continue;
                        if (CheckCollisionBoxes(entities[i].bounds, entities[j].bounds)) {
                            CollisionEvent event;
                            event.entityA = &entities[i];
                            event.entityB = &entities[j];
                            EventManager::GetInstance().FireEvent(event);
                        }
                    }
                }
            }
        }
    }
}
