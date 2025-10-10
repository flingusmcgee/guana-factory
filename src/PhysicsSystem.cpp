#include "PhysicsSystem.h"

// Update position of entities based on their velocity
void PhysicsSystem::Update  (std::vector<Entity>& entities, float dt) {
    for (auto& entity : entities) {
        if (entity.is_active) {
            entity.position.x += entity.velocity.x * dt;
            entity.position.y += entity.velocity.y * dt;
            entity.position.z += entity.velocity.z * dt;
            
            // Update bounding box position
            entity.bounds.min.x = entity.position.x - 0.5f;
            entity.bounds.min.y = entity.position.y - 0.5f;
            entity.bounds.min.z = entity.position.z - 0.5f;
            entity.bounds.max.x = entity.position.x + 0.5f;
            entity.bounds.max.y = entity.position.y + 0.5f;
            entity.bounds.max.z = entity.position.z + 0.5f;
        }
    }
}  
