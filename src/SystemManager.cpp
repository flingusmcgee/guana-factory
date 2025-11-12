#include "SystemManager.h"
#include "System.h"
#include "Log.h"

SystemManager& SystemManager::GetInstance() {
    static SystemManager instance;
    return instance;
}

void SystemManager::RegisterSystem(const std::string& id, std::unique_ptr<System> sys) {
    if (!sys) return;
    if (systems.find(id) != systems.end()) {
        Log::Warning(std::string("System already registered: ") + id);
        return;
    }
    order.push_back(id);
    systems[id] = std::move(sys);
}

void SystemManager::UnregisterSystem(const std::string& id) {
    auto it = systems.find(id);
    if (it == systems.end()) return;
    systems.erase(it);
    order.erase(std::remove(order.begin(), order.end(), id), order.end());
}

bool SystemManager::InitAll() {
    for (const auto& id : order) {
        auto& s = systems[id];
        if (s) {
            Log::Info(std::string("Init System: ") + id);
            if (!s->Init()) {
                Log::Warning(std::string("System failed to init: ") + id);
                return false;
            }
        }
    }
    return true;
}

void SystemManager::UpdateAll(float dt) {
    for (const auto& id : order) {
        auto& s = systems[id];
        if (s) s->Update(dt);
    }
}

void SystemManager::ShutdownAll() {
    // Shutdown in reverse order of init
    for (auto it = order.rbegin(); it != order.rend(); ++it) {
        const auto& id = *it;
        auto& s = systems[id];
        if (s) {
            Log::Info(std::string("Shutdown System: ") + id);
            s->Shutdown();
        }
    }
    systems.clear();
    order.clear();
}

bool SystemManager::HasSystem(const std::string& id) const {
    return systems.find(id) != systems.end();
}
