#include "BatchedRenderer.h"
#include "DODStorage.h"
#include "EntityManager.h"
#include "DebugHud.h"
#include <unordered_map>
#include <vector>

namespace BatchedRenderer {
    void Init() {
        // currently no initialization required
    }

    void RenderAll(const Camera& cam) {
        int maxEntities = 1000; // fallback
        // try to infer from entity manager
        // NOTE: using EntityManager's pool size is not exposed; instead use MAX_ENTITIES
#ifdef MAX_ENTITIES
        maxEntities = MAX_ENTITIES;
#endif

        std::unordered_map<Model*, std::vector<int>> groups;
        groups.reserve(64);

        for (int i = 0; i < maxEntities; ++i) {
            if (!DODStorage::IsActive(i)) continue;
            Model* m = DODStorage::GetModelPtr(i);
            if (!m) continue;
            groups[m].push_back(i);
        }

        // Draw grouped models
        for (auto &g : groups) {
            Model* m = g.first;
            for (int id : g.second) {
                Vector3 pos = DODStorage::GetPosition(id);
                Color c = DODStorage::GetColor(id);
                DrawModel(*m, pos, 1.0f, c);
            }
        }
    }
}
