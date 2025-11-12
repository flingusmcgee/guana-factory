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
    void UpdateCameraControls(float dt, Vector2 mouseDelta);
    float cameraBaseSpeed = 6.0f;
    float cameraBoostMultiplier = 3.0f;
    float cameraSensitivity = 0.003f;
    bool cursorLocked = false;
    bool exitRequested = false;
    // Camera orientation state for mouse-look
    float cameraYaw = 0.0f;   // radians
    float cameraPitch = 0.0f; // radians
    bool debugHudVisible = false;
    bool previousWindowFocused = false;
    bool cameraInvertX = false;
    bool cameraInvertY = false;
    bool cursorLockedBeforeFocusLoss = false;

    // The master time scale for all game logic
    float timeScale = 1.0f;
    // Debug HUD throttling (cached string updated at debugHudInterval)
    float debugHudTimer = 0.0f;
    float debugHudInterval = 0.1f; // seconds (10 Hz)
    char debugHudBuf[256] = {0};
    int debugHudBufLen = 0;
    // Lightweight profiler (ms)
    bool profilerEnabled = true;
    double lastFrameMs = 0.0;
    double lastUpdateMs = 0.0;
    double lastRenderMs = 0.0;
    double lastCameraMs = 0.0;
    double lastEntityMs = 0.0;
};
