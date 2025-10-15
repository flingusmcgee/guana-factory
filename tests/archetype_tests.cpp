#include <iostream>
#include "src/ArchetypeManager.h"
#include "src/Log.h"

int main() {
    Log::Init();
    Log::Info("Running archetype tests");

    ArchetypeManager::GetInstance().LoadArchetypesFromDirectory("res/archetypes");
    size_t count = ArchetypeManager::GetInstance().GetLoadedCount();
    std::cout << "Loaded archetypes: " << count << std::endl;
    if (count < 3) {
        Log::Error("Expected at least 3 archetypes for tests");
        return 2;
    }

    // Basic individual file load check
    bool ok = false;
    Archetype* a = ArchetypeManager::GetInstance().GetArchetype("cube_red");
    if (a && a->tag == "cube_red") ok = true;

    if (!ok) {
        Log::Error("cube_red archetype missing or malformed");
        return 2;
    }

    Log::Info("Archetype tests passed");
    Log::Shutdown();
    return 0;
}
