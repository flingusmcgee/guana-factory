#pragma once

#include <vector>
#include <algorithm>
#include <memory>
#include <string>
#include <unordered_map>
#include "System.h"

class SystemManager {
public:
    static SystemManager& GetInstance();

    // Register a system instance. Ownership is transferred to the manager.
    void RegisterSystem(const std::string& id, std::unique_ptr<System> sys);
    // Unregister and destroy a system
    void UnregisterSystem(const std::string& id);

    // Lifecycle
    bool InitAll();
    void UpdateAll(float dt);
    void ShutdownAll();

    // Debug helpers
    bool HasSystem(const std::string& id) const;

private:
    SystemManager() {}
    std::unordered_map<std::string, std::unique_ptr<System>> systems;
    std::vector<std::string> order; // registration order
};
