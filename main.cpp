#include "src/Game.h"
#include "src/Log.h"
#include "src/ArchetypeManager.h"

// Entry point
int main() {
    Log::Init(); // Initialize the log
    Log::Info("Engine starting up");
    // Load archetypes before initializing the game (smoke test)
    ArchetypeManager::GetInstance().LoadArchetypesFromDirectory("res/archetypes");
    Log::Info("Loaded archetypes: " + std::to_string(ArchetypeManager::GetInstance().GetLoadedCount()));

    Game::GetInstance().Init();
    Game::GetInstance().Run();
    Game::GetInstance().Shutdown();

    Log::Info("Engine shutting down");
    Log::Shutdown(); // Close the log file
    return 0;
}
