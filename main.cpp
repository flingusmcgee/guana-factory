#include "src/Game.h"
#include "src/Log.h"
#include "src/Config.h"
#include "src/ArchetypeManager.h"

// Entry point
int main() {
    Log::Init(); // Initialize the log
    // Log resolved log directory so it's easy to find runtime logs
    Config::Load("config.ini");
    std::string defaultLogDir = "build/log";
    std::string resolvedLogDir = Config::GetString("log.dir", defaultLogDir);
    Log::Info(std::string("Resolved log.dir: ") + resolvedLogDir);
    Log::Info("Engine starting up");
    Game::GetInstance().Init();
    Game::GetInstance().Run();
    Game::GetInstance().Shutdown();

    Log::Info("Engine shutting down");
    Log::Shutdown(); // Close the log file
    return 0;
}
