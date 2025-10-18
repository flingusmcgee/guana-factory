#include "Game.h"
#include "AssetManager.h"
#include "EntityManager.h"
#include "EventManager.h"
#include "ArchetypeManager.h"
#include "Log.h"
#include "Config.h"
#include <iostream>
#include <cmath>

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
    int targetFPS = 60;
    // Load configuration (optional)
    bool cfgLoaded = false;
    if (Config::Load("config.ini")) cfgLoaded = true;
    else if (Config::Load("../config.ini")) cfgLoaded = true;

    // Apply config-driven debug/time settings
    bool debugEnabled = Config::GetBool("debug.enabled", false);
    float cfgTimeScale = Config::GetFloat("debug.time_scale", 1.0f);
    if (debugEnabled) {
        Log::Info("Debug enabled via config");
        SetTimeScale(cfgTimeScale);
    }

    // Window settings
    std::string winTitle = Config::GetString("window.title", "guana factory");
    screenWidth = Config::GetInt("window.width", screenWidth);
    screenHeight = Config::GetInt("window.height", screenHeight);
    targetFPS = Config::GetInt("window.target_fps", targetFPS);

    InitWindow(screenWidth, screenHeight, winTitle.c_str());
    cursorLocked = true;
    exitRequested = false;
    DisableCursor();
    SetTargetFPS(targetFPS);

    // Camera setup
    camera.position = { 10.0f, 10.0f, 10.0f };
    camera.target = { 0.0f, 0.0f, 0.0f };
    camera.up = { 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // Load assets and initialize systems
    // Allow the icon path to be configurable
    AssetManager::LoadAssets();
    // Try a couple of likely archetype paths (executable working directory may vary)
    // Archetype paths: support a comma-separated list in config
    std::string apaths = Config::GetString("archetypes.paths", "res/archetypes,../res/archetypes");
    bool loadedAny = false;
    size_t start = 0;
    while (start < apaths.size()) {
        size_t comma = apaths.find(',', start);
        std::string token = (comma == std::string::npos) ? apaths.substr(start) : apaths.substr(start, comma - start);
        // trim
        size_t s = 0; while (s < token.size() && std::isspace(static_cast<unsigned char>(token[s]))) ++s;
        size_t e = token.size(); while (e > s && std::isspace(static_cast<unsigned char>(token[e-1]))) --e;
        std::string path = token.substr(s, e - s);
        if (!path.empty()) {
            ArchetypeManager::GetInstance().LoadArchetypesFromDirectory(path.c_str());
            if (ArchetypeManager::GetInstance().GetLoadedCount() > 0) {
                Log::Info(std::string("Loaded archetypes from: ") + path);
                loadedAny = true;
                break;
            }
        }
        if (comma == std::string::npos) break;
        start = comma + 1;
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
    while (!WindowShouldClose() && !exitRequested) {
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
    if (IsKeyPressed(KEY_ESCAPE)) {
        exitRequested = true;
        return;
    }

    if (IsKeyPressed(KEY_TAB)) {
        cursorLocked = !cursorLocked;
        if (cursorLocked) {
            DisableCursor();
            GetMouseDelta(); // flush accumulated movement when relocking
        } else {
            EnableCursor();
            GetMouseDelta(); // discard delta so camera doesn't jump after unlocking
        }
    }

    // Apply the time scale to the delta time before passing it to other systems
    float scaledDeltaTime = GetFrameTime() * timeScale;

    if (cursorLocked) {
        UpdateCameraControls(scaledDeltaTime);
    }

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
    EnableCursor();
    CloseWindow();
}

void Game::UpdateCameraControls(float dt) {
    Vector3 move = { 0.0f, 0.0f, 0.0f };
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) move.z += 1.0f;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) move.z -= 1.0f;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) move.x += 1.0f;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) move.x -= 1.0f;
    if (IsKeyDown(KEY_SPACE)) move.y += 1.0f;
    if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) move.y -= 1.0f;

    float length = std::sqrt(move.x*move.x + move.y*move.y + move.z*move.z);
    if (length > 0.0f) {
        move.x /= length;
        move.y /= length;
        move.z /= length;
    }

    float speed = cameraBaseSpeed;
    if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) speed *= cameraBoostMultiplier;

    Vector3 scaledMove = { move.x * speed * dt, move.y * speed * dt, move.z * speed * dt };

    Vector2 mouseDelta = GetMouseDelta();
    Vector3 rotation = { mouseDelta.y * cameraSensitivity, mouseDelta.x * cameraSensitivity, 0.0f };

    UpdateCameraPro(&camera, scaledMove, rotation, 0.0f);
}