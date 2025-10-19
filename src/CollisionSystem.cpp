#include "CollisionSystem.h"
#include "EventManager.h"
#include <cstdint>
#include <cmath>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// Spatial hash broad-phase parameters
static const float CELL_SIZE = 1.0f; // world units per cell; tune as needed

// Helper to hash 2D cell coordinates into a single int64 key
static inline int64_t CellKey(int x, int y) {
    return (static_cast<int64_t>(x) << 32) ^ static_cast<uint32_t>(y);
}

// Check all active entities for collisions and fire events using spatial hashing
void CollisionSystem::CheckCollisions(std::vector<Entity>& entities) {
    static std::unordered_map<int64_t, std::vector<size_t>> buckets;
    buckets.clear();
    buckets.reserve(entities.size() * 2);

    // Track active collision pairs across frames to avoid spamming the same
    // collision event while two entities remain in contact.
    static std::unordered_set<uint64_t> activePairs;
    std::unordered_set<uint64_t> currentFramePairs;

    // Insert entities into buckets based on their AABB centers
    for (size_t i = 0; i < entities.size(); ++i) {
        if (!entities[i].is_active) continue;
        float cx = (entities[i].bounds.min.x + entities[i].bounds.max.x) * 0.5f;
        float cz = (entities[i].bounds.min.z + entities[i].bounds.max.z) * 0.5f;
        int cellX = static_cast<int>(std::floor(cx / CELL_SIZE));
        int cellZ = static_cast<int>(std::floor(cz / CELL_SIZE));
        int64_t key = CellKey(cellX, cellZ);
        auto &vec = buckets[key];
        if (vec.capacity() == 0) vec.reserve(4);
        vec.push_back(i);
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
                            // Build a symmetric pair key (min<<32 | max)
                            uint32_t a = static_cast<uint32_t>(entities[i].id);
                            uint32_t b = static_cast<uint32_t>(entities[j].id);
                            uint32_t lo = (a < b) ? a : b;
                            uint32_t hi = (a < b) ? b : a;
                            uint64_t pairKey = (static_cast<uint64_t>(lo) << 32) | hi;
                            currentFramePairs.insert(pairKey);

                            // Only fire event if this pair was not colliding last frame
                            if (activePairs.find(pairKey) == activePairs.end()) {
                                CollisionEvent event;
                                event.entityA = &entities[i];
                                event.entityB = &entities[j];
                                EventManager::GetInstance().FireEvent(event);
                                activePairs.insert(pairKey);
                            }
                        }
                    }
                }
            }
        }
    }

    // Remove pairs that are no longer colliding
    for (auto it = activePairs.begin(); it != activePairs.end(); ) {
        if (currentFramePairs.find(*it) == currentFramePairs.end()) {
            it = activePairs.erase(it);
        } else {
            ++it;
        }
    }
}
