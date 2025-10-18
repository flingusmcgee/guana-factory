#pragma once
#include "include/raylib.h"

// The main game singleton. Manages the game loop and core systems
class Game {
public:
    static Game& GetInstance();
    void Init();
    void Run();
    void Shutdown();

    // The time scale management function
    void SetTimeScale(float scale);

private:
    Game() {}
    void Update();
    void Render();
    
    int screenWidth = 800;
    int screenHeight = 450;
    Camera3D camera = {};
    void UpdateCameraControls(float dt);
    float cameraBaseSpeed = 6.0f;
    float cameraBoostMultiplier = 3.0f;
    float cameraSensitivity = 0.003f;
    bool cursorLocked = true;
    bool exitRequested = false;

    // The master time scale for all game logic
    float timeScale = 1.0f;
};
