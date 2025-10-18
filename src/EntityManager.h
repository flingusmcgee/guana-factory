#pragma once
#include "Entity.h"
#include "PhysicsSystem.h"
#include "CollisionSystem.h"
#include <vector>
#include <string>

// Maximum number of entities allowed in the world
constexpr int MAX_ENTITIES = 1000; // CHANGE ME OR I WILL FIND YOU

class EntityManager {
public:
    static EntityManager& GetInstance();
    void Init();
    Entity* CreateEntity();
    Entity* CreateEntityFromArchetype(const std::string& name, Vector3 position); // The Factory
    void UpdateAll(float dt);
    void RenderAll();
    int GetActiveCount() const;

    Entity* FindEntityByID(unsigned int id);
    std::vector<Entity*> FindEntitiesWithTag(const std::string& tag);

private:
    EntityManager() {}
    std::vector<Entity> entity_pool;

    // Instances of the core gameplay systems
    PhysicsSystem physicsSystem;
    CollisionSystem collisionSystem;
};