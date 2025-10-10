#include "Game.h"
#include "AssetManager.h"
#include "EntityManager.h"
#include "EventManager.h"
#include "ArchetypeManager.h"
#include "Log.h"
#include <iostream>


// A temporary function to handle collision events for testing
void OnCollision(const Event& event) {
    const CollisionEvent& collision = static_cast<const CollisionEvent&>(event);

    // Placeholder: print entity IDs to console
std::cout << "WOAH: Collision Detected!" << std::endl;
std::cout << "EntityA ID: " << collision.entityA->id << std::endl; // Example: print entityA's ID
std::cout << "EntityB ID: " << collision.entityB->id << std::endl; // Example: print entityB's ID
}

// Provide a single, global instance of the game
Game& Game::GetInstance() {
    static Game instance;
    return instance;
}

// Setup the window, camera, and all managers
void Game::Init() {
    const int screenWidth = 800;
    const int screenHeight = 450;
    const int targetFPS = 60;

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
    ArchetypeManager::GetInstance().LoadArchetypesFromDirectory("../res/archetypes");
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

// Update game logic
void Game::Update() {
    //std::cout << "Game::Update() has been called. DeltaTime is: " << GetFrameTime() << std::endl;
    UpdateCamera(&camera, CAMERA_FREE); 
    EntityManager::GetInstance().UpdateAll(GetFrameTime());
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
