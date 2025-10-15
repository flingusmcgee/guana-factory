#include "Game.h"
#include "AssetManager.h"
#include "EntityManager.h"
#include "EventManager.h"
#include "ArchetypeManager.h"
#include "Log.h"
#include "Config.h"
#include <iostream>

// A temporary function to handle collision events for testing
void OnCollision(const Event& event) {
    const CollisionEvent& collision = static_cast<const CollisionEvent&>(event);

    // Placeholder: print entity IDs to console
    std::cout << "WOAH: Collision Detected!" << std::endl;
    std::cout << "EntityA ID: " << collision.entityA->id << std::endl;
    std::cout << "EntityB ID: " << collision.entityB->id << std::endl;
}

// Provide a single, global instance of the game
Game& Game::GetInstance() {
    static Game instance;
    return instance;
}

// Setup the window, camera, and all managers
void Game::Init() {
    const int targetFPS = 60;
    // Load configuration (optional)
    bool cfgLoaded = false;
    if (Config::Load("config.ini")) cfgLoaded = true;
    else if (Config::Load("../config.ini")) cfgLoaded = true;

    // Apply config-driven debug/time settings
    bool debugEnabled = Config::GetBool("debug.enabled", false);
    float cfgTimeScale = 1.0f;
    try { cfgTimeScale = std::stof(Config::GetString("debug.time_scale", "1.0")); } catch(...) {}
    if (debugEnabled) {
        Log::Info("Debug enabled via config");
        SetTimeScale(cfgTimeScale);
    }

    InitWindow(screenWidth, screenHeight, "guana factory");
    SetTargetFPS(targetFPS);

    // Camera setup
    camera.position = { 10.0f, 10.0f, 10.0f };
    camera.target = { 0.0f, 0.0f, 0.0f };
    camera.up = { 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // Load assets and initialize systems
    AssetManager::LoadAssets();
    // Try a couple of likely archetype paths (executable working directory may vary)
    const char* archetypePaths[] = { "res/archetypes", "../res/archetypes" };
    bool loadedAny = false;
    for (const char* ap : archetypePaths) {
        ArchetypeManager::GetInstance().LoadArchetypesFromDirectory(ap);
        if (ArchetypeManager::GetInstance().GetLoadedCount() > 0) {
            Log::Info(std::string("Loaded archetypes from: ") + ap);
            loadedAny = true;
            break;
        }
    }
    if (!loadedAny) {
        Log::Warning("No archetypes loaded; checked common paths");
    }
    EntityManager::GetInstance().Init();

    // Subscribe to events
    EventManager::GetInstance().Subscribe(EventType::Collision, OnCollision);
    
    // Create entities using the factory function
    EntityManager::GetInstance().CreateEntityFromArchetype("cube_red", { -2.0f, 0.5f, 0.0f });
    EntityManager::GetInstance().CreateEntityFromArchetype("cube_blue", { 2.0f, 0.5f, 0.0f });
}

// The main game loop
void Game::Run() {
    while (!WindowShouldClose()) {
        Update();
        Render();
    }
}

// Set the global time scale for the game logic
void Game::SetTimeScale(float scale) {
    timeScale = scale;
}

// Update game logic
void Game::Update() {
    UpdateCamera(&camera, CAMERA_FREE); 
    
    // Apply the time scale to the delta time before passing it to other systems
    float scaledDeltaTime = GetFrameTime() * timeScale;
    EntityManager::GetInstance().UpdateAll(scaledDeltaTime);
}

// Draw everything to the screen
void Game::Render() {
    BeginDrawing();
    ClearBackground(SKYBLUE);

    BeginMode3D(camera);
        DrawGrid(20, 1.0f);
        EntityManager::GetInstance().RenderAll();
    EndMode3D();

    DrawFPS(10, 10);
    EndDrawing();
}

// Unload assets and close the window
void Game::Shutdown() {
    AssetManager::UnloadAssets();
    CloseWindow();
}