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

    // Detailed checks for archetypes
    {
        Archetype* a = ArchetypeManager::GetInstance().GetArchetype("cube");
        if (!a) { Log::Error("cube archetype missing"); return 2; }
        // Diagnostic: log actual values
        Log::Info("cube -> tag='" + a->tag + "' model_id='" + a->model_id + "'");
        Log::Info("cube -> color=(" + std::to_string((int)a->color.r) + "," + std::to_string((int)a->color.g) + "," + std::to_string((int)a->color.b) + ")");
        if (a->tag != "cube") { Log::Error("cube.tag mismatch"); return 2; }
        if (a->model_id != "cube") { Log::Error("cube.model_id mismatch"); return 2; }
        if (a->color.r != 128 || a->color.g != 128 || a->color.b != 128) { Log::Error("cube.color mismatch"); return 2; }
    }

    {
        Archetype* a = ArchetypeManager::GetInstance().GetArchetype("cube_red");
        if (!a) { Log::Error("cube_red archetype missing"); return 2; }
        Log::Info("cube_red -> color=(" + std::to_string((int)a->color.r) + "," + std::to_string((int)a->color.g) + "," + std::to_string((int)a->color.b) + ") vel.x=" + std::to_string(a->velocity.x));
        if (a->color.r != 255 || a->color.g != 0 || a->color.b != 0) { Log::Error("cube_red.color mismatch"); return 2; }
        if (a->velocity.x != 1.0f) { Log::Error("cube_red.velocity.x mismatch"); return 2; }
    }

    {
        Archetype* a = ArchetypeManager::GetInstance().GetArchetype("cube_blue");
        if (!a) { Log::Error("cube_blue archetype missing"); return 2; }
        Log::Info("cube_blue -> color=(" + std::to_string((int)a->color.r) + "," + std::to_string((int)a->color.g) + "," + std::to_string((int)a->color.b) + ") vel.x=" + std::to_string(a->velocity.x));
        if (a->color.r != 0 || a->color.g != 0 || a->color.b != 255) { Log::Error("cube_blue.color mismatch"); return 2; }
        if (a->velocity.x != -1.0f) { Log::Error("cube_blue.velocity.x mismatch"); return 2; }
    }

    Log::Info("Archetype tests passed");
    Log::Shutdown();
    return 0;
}
