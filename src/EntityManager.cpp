#include "EntityManager.h"
#include "ArchetypeManager.h"
#include "AssetManager.h"
#include "Log.h"

EntityManager& EntityManager::GetInstance() {
    static EntityManager instance;
    return instance;
}

// Prepare the entity pool for use
void EntityManager::Init() {
    entity_pool.resize(MAX_ENTITIES);
}

// Find an inactive entity, activate it and return a pointer
Entity* EntityManager::CreateEntity() {
    for (int i = 0; i < MAX_ENTITIES; ++i) {
        if (!entity_pool[i].is_active) {
            // Reset the entity to a clean state before use
            entity_pool[i] = Entity(); 
            entity_pool[i].is_active = true;
            entity_pool[i].id = i;
            return &entity_pool[i];
        }
    }
    return nullptr; // No inactive entities found
}

// Find a single, active entity by its unique ID
Entity* EntityManager::FindEntityByID(unsigned int id) {
    if (id < entity_pool.size() && entity_pool[id].is_active) {
        return &entity_pool[id];
    }
    return nullptr; // Return null if ID is out of bounds or entity is not active
}

// Find all active entities that have a specific tag
std::vector<Entity*> EntityManager::FindEntitiesWithTag(const std::string& tag) {
    std::vector<Entity*> tagged_entities;
    for (auto& entity : entity_pool) {
        if (entity.is_active && entity.tag == tag) {
            tagged_entities.push_back(&entity);
        }
    }
    return tagged_entities;
}

// Update all core systems and active entities
void EntityManager::UpdateAll(float dt) {
    // Run systems that operate on all entities
    physicsSystem.Update(entity_pool, dt);
    collisionSystem.CheckCollisions(entity_pool);
    
    // Loop for individual entity logic
    for (auto& entity : entity_pool) {
        if (entity.is_active) {
            // Behavior logic will be called here
        }
    }

    // A second loop
    // This deactivates any entities that were marked for death during the update phase
    for (size_t i = 0; i < entity_pool.size(); ++i) {
        auto& entity = entity_pool[i];
        if (entity.is_active && entity.needs_to_die) {
            entity = Entity();
            entity.id = static_cast<unsigned int>(i);
        }
    }
}

// Function that builds an entity from an archetype
Entity* EntityManager::CreateEntityFromArchetype(const std::string& name, Vector3 position) {
    Archetype* arch = ArchetypeManager::GetInstance().GetArchetype(name);
    if (!arch) {
        Log::Error("Failed to create entity: Archetype '" + name + "' not found.");
        return nullptr;
    }

    Entity* newEntity = CreateEntity();
    if (newEntity) {
        newEntity->position = position;
        newEntity->tag = arch->tag;
        newEntity->color = arch->color;
        newEntity->velocity = arch->velocity;

        // Paranoid check for the model
        if (AssetManager::ModelExists(arch->model_id)) {
             newEntity->model = AssetManager::GetModel(arch->model_id);
        } else {
            Log::Error("Model '" + arch->model_id + "' not found for archetype '" + name + "'. Using default thing.");
            newEntity->model = AssetManager::GetModel("cube"); // Fallback
        }
    }
    return newEntity;
}


// RENDERER MUST BE MODIFIED TO USE THE NEW COLOR
void EntityManager::RenderAll() {
    for (const auto& entity : entity_pool) {
        if (entity.is_active) {
            DrawModel(entity.model, entity.position, 1.0f, entity.color); // Use entity's colour now
        }
    }
}
