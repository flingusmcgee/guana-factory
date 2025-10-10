#include "src/Game.h"
#include "src/Log.h"

// Entry point
int main() {
    Log::Init(); // Initialize the log
    Log::Info("Engine starting up");
    
    Game::GetInstance().Init();
    Game::GetInstance().Run();
    Game::GetInstance().Shutdown();

    Log::Info("Engine shutting down");
    Log::Shutdown(); // Close the log file
    return 0;
}
