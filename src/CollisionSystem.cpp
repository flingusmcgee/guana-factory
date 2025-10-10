#include "CollisionSystem.h"
#include "EventManager.h"
#include <string>

// Check all active entities for collisions and fire venets
void CollisionSystem::CheckCollisions(std::vector<Entity>& entities) {
    for (size_t i = 0; i < entities.size(); ++i) {
        if (!entities[i].is_active) continue;

        for (size_t j = i + 1; j < entities.size(); ++j) {
            if (!entities[j].is_active) continue;

            if (CheckCollisionBoxes(entities[i].bounds, entities[j].bounds)) {
                CollisionEvent event;
                event.entityA = &entities[i];
                event.entityB = &entities[j];
                static const std::string COLLISION_EVENT_NAME = "Collision";
                EventManager::GetInstance().FireEvent(event);
            }
        }
    }
};
