#pragma once
#include <vector>
#include "Entity.h"

// Detecting interactions between entities
class CollisionSystem {
public:
    void CheckCollisions(std::vector<Entity>& entities);
};
