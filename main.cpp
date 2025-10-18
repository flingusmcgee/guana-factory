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
    // Avoid reporting a nested build/build/log when running from the build directory
    try {
        auto cwd = std::filesystem::current_path();
        if (cwd.filename() == "build") {
            defaultLogDir = (cwd.parent_path() / std::filesystem::path("build") / std::filesystem::path("log")).string();
        }
    } catch (...) {
        // fallback to plain relative path
    }
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
