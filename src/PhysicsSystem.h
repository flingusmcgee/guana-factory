#pragma once
#include <vector>
#include "Entity.h"

// Movin' 
class PhysicsSystem {
public:
    void Update(std::vector<Entity>& entities, float dt);
};
