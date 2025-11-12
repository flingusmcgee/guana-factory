#include "Game.h"
#include "AssetManager.h"
#include "EntityManager.h"
#include "EventManager.h"
#include "ArchetypeManager.h"
#include "Log.h"
#include "Config.h"
#include "SystemManager.h"
#include "Input.h"
#include "DebugHud.h"
#include <cmath>
#include <cstdio>
#include <chrono>

// Collision event handler — use Log so level can be controlled via config
void OnCollision(const Event& event) {
    const CollisionEvent& collision = static_cast<const CollisionEvent&>(event);
    bool debugCollisions = Config::GetBool("debug.collision_logs", false);
    // The collision system already de-duplicates pairs; log at Info if the
    // user explicitly enabled collision debug, otherwise keep it at Debug.
    std::string msg = "Collision: A=" + std::to_string(collision.entityA->id)
        + " B=" + std::to_string(collision.entityB->id);
    if (debugCollisions) Log::Info(msg);
    else Log::Warning(msg); // keep visible but less spammy than repeated cout
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
    if (!Config::Load("config.ini")) Config::Load("../config.ini");

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
    exitRequested = false;

    // Input and debug HUD
    Input::Init();
    DebugHud::Init();

    // Track window focus so Alt+Tab releases the cursor
    previousWindowFocused = IsWindowFocused();
    // Only lock/disable the cursor at startup if the window is actually focused.
    // This prevents the invisible game window from confining mouse movement
    // when the user launches the game on another tab.
    cursorLocked = previousWindowFocused;
    cursorLockedBeforeFocusLoss = false;
    if (cursorLocked) {
        DisableCursor();
    } else {
        EnableCursor();
    }

    // Camera setup
    camera.position = { 10.0f, 10.0f, 10.0f };
    camera.target = { 0.0f, 0.0f, 0.0f };
    camera.up = { 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // Initialize camera yaw/pitch from initial position->target so we look at the scene
    {
        Vector3 dir;
        dir.x = camera.target.x - camera.position.x;
        dir.y = camera.target.y - camera.position.y;
        dir.z = camera.target.z - camera.position.z;
        float len = std::sqrt(dir.x*dir.x + dir.y*dir.y + dir.z*dir.z);
        if (len > 1e-6f) {
            dir.x /= len; dir.y /= len; dir.z /= len;
            cameraYaw = std::atan2(dir.x, dir.z);
            cameraPitch = std::asin(dir.y);
        } else {
            cameraYaw = 0.0f; cameraPitch = 0.0f;
        }
    }

    // Read camera settings from config (sensitivity and inversion)
    cameraSensitivity = Config::GetFloat("camera.sensitivity", cameraSensitivity);
    cameraInvertX = Config::GetBool("camera.invert_x", cameraInvertX);
    cameraInvertY = Config::GetBool("camera.invert_y", cameraInvertY);

    // Load assets and initialize systems
    // Allow the icon path to be configurable
    AssetManager::LoadAssets();
    // Initialize registered systems
    SystemManager::GetInstance().InitAll();
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
    // Profiling: frame start
    auto frameStart = std::chrono::high_resolution_clock::now();

    // Update mapped input
    Input::Update();

    // If the window lost/gained focus (Alt+Tab), adjust cursor state
    bool focused = IsWindowFocused();
    if (!focused && previousWindowFocused) {
        // lost focus
        cursorLockedBeforeFocusLoss = cursorLocked;
        if (cursorLocked) {
            cursorLocked = false;
            EnableCursor();
            GetMouseDelta(); // flush so we don't get a big jump when returning
        } else {
            EnableCursor();
        }
    } else if (focused && !previousWindowFocused) {
        // regained focus
        if (cursorLockedBeforeFocusLoss) {
            cursorLocked = true;
            DisableCursor();
            GetMouseDelta(); // flush
        }
    }
    previousWindowFocused = focused;

    if (Input::WasPressed("quit")) {
        exitRequested = true;
        return;
    }

    if (Input::WasPressed("toggle_cursor")) {
        cursorLocked = !cursorLocked;
        if (cursorLocked) {
            DisableCursor();
            GetMouseDelta(); // flush accumulated movement when relocking
        } else {
            EnableCursor();
            GetMouseDelta(); // discard delta so camera doesn't jump after unlocking
        }
    }

    if (Input::WasPressed("debug_toggle")) {
        DebugHud::Toggle();
    }

    // Apply the time scale to the delta time before passing it to other systems
    float scaledDeltaTime = GetFrameTime() * timeScale;

    // Update debug HUD buffer at a throttled interval
    debugHudTimer -= GetFrameTime();
    if (debugHudTimer <= 0.0f) {
        debugHudTimer = debugHudInterval;
        // Rebuild the HUD buffer (include profiler timings when enabled)
        if (profilerEnabled) {
            debugHudBufLen = std::snprintf(debugHudBuf, sizeof(debugHudBuf),
                "pos=%.2f,%.2f,%.2f tgt=%.2f,%.2f,%.2f yaw=%.2f pitch=%.2f sens=%.2f | U=%.2f C=%.2f E=%.2f R=%.2f F=%.2fms",
                camera.position.x, camera.position.y, camera.position.z,
                camera.target.x, camera.target.y, camera.target.z,
                cameraYaw, cameraPitch, cameraSensitivity,
                lastUpdateMs, lastCameraMs, lastEntityMs, lastRenderMs, lastFrameMs);
        } else {
            debugHudBufLen = std::snprintf(debugHudBuf, sizeof(debugHudBuf),
                "pos=%.2f,%.2f,%.2f tgt=%.2f,%.2f,%.2f yaw=%.2f pitch=%.2f sens=%.2f",
                camera.position.x, camera.position.y, camera.position.z,
                camera.target.x, camera.target.y, camera.target.z,
                cameraYaw, cameraPitch, cameraSensitivity);
        }
        if (debugHudBufLen < 0) debugHudBufLen = 0;
        if (debugHudBufLen >= static_cast<int>(sizeof(debugHudBuf))) debugHudBufLen = static_cast<int>(sizeof(debugHudBuf)) - 1;
        debugHudBuf[debugHudBufLen] = '\0';
    }

    // Read mouse delta once per frame to avoid jitter
    Vector2 mouseDelta = { 0.0f, 0.0f };
    if (cursorLocked && focused) {
        mouseDelta = GetMouseDelta();
    } else {
        // Flush any accumulated movement when unlocked/unfocused
        GetMouseDelta();
    }

    // Profile camera update
    auto camStart = std::chrono::high_resolution_clock::now();
    UpdateCameraControls(scaledDeltaTime, mouseDelta);
    auto camEnd = std::chrono::high_resolution_clock::now();

    // Profile entity updates
    auto entStart = std::chrono::high_resolution_clock::now();
    EntityManager::GetInstance().UpdateAll(scaledDeltaTime);
    auto entEnd = std::chrono::high_resolution_clock::now();

    // Update systems
    SystemManager::GetInstance().UpdateAll(scaledDeltaTime);

    // Record timings (milliseconds)
    if (profilerEnabled) {
        lastCameraMs = std::chrono::duration<double, std::milli>(camEnd - camStart).count();
        lastEntityMs = std::chrono::duration<double, std::milli>(entEnd - entStart).count();
    }

    auto frameEnd = std::chrono::high_resolution_clock::now();
    if (profilerEnabled) {
        lastUpdateMs = std::chrono::duration<double, std::milli>(frameEnd - frameStart).count();
    }
}

// Draw everything to the screen
void Game::Render() {
    auto renderStart = std::chrono::high_resolution_clock::now();
    BeginDrawing();
    ClearBackground(SKYBLUE);

    BeginMode3D(camera);
        DrawGrid(20, 1.0f);
        EntityManager::GetInstance().RenderAll();
    EndMode3D();
    DrawFPS(10, 10);
    // Debug hud (throttled) - use cached buffer updated in Update()
    DebugHud::Draw(static_cast<int>(EntityManager::GetInstance().GetActiveCount()), debugHudBuf);
    // Small origin marker so you can see where (0,0,0) is
    Vector3 origin = { 0.0f, 0.0f, 0.0f };
    DrawSphere(origin, 0.1f, RED);
    // Crosshair in the center of the screen
    {
        int cx = screenWidth / 2;
        int cy = screenHeight / 2;
        DrawLine(cx - 8, cy, cx + 8, cy, BLACK);
        DrawLine(cx, cy - 8, cx, cy + 8, BLACK);
        DrawCircle(cx, cy, 2, WHITE);
    }
    EndDrawing();
    auto renderEnd = std::chrono::high_resolution_clock::now();
    if (profilerEnabled) {
        lastRenderMs = std::chrono::duration<double, std::milli>(renderEnd - renderStart).count();
        lastFrameMs = lastUpdateMs + lastRenderMs;
    }
}

// Unload assets and close the window
void Game::Shutdown() {
    AssetManager::UnloadAssets();
    SystemManager::GetInstance().ShutdownAll();
    EnableCursor();
    CloseWindow();
}

void Game::UpdateCameraControls(float dt, Vector2 mouseDelta) {
    // Read movement from mapped input (camera-relative)
    float mx = 0.0f; // right
    float my = 0.0f; // up
    float mz = 0.0f; // forward

    if (Input::IsDown("move_forward")) mz += 1.0f;
    if (Input::IsDown("move_back")) mz -= 1.0f;
    if (Input::IsDown("move_right")) mx += 1.0f;
    if (Input::IsDown("move_left")) mx -= 1.0f;
    if (Input::IsDown("move_up")) my += 1.0f;
    if (Input::IsDown("move_down")) my -= 1.0f;

    // Normalize horizontal/forward plane to avoid faster diagonal movement
    float planarLen = std::sqrt(mx*mx + mz*mz);
    if (planarLen > 1e-6f) {
        mx /= planarLen;
        mz /= planarLen;
    }

    float speed = cameraBaseSpeed;
    if (Input::IsDown("sprint")) speed *= cameraBoostMultiplier;

        // Mouse look: accumulate yaw/pitch
        // mouseDelta passed in from Update() to avoid multiple reads per frame
        if (cursorLocked) {
            Vector2 md = mouseDelta;
            // horizontal
            float hx = md.x * cameraSensitivity;
            if (!cameraInvertX) hx = -hx;
            cameraYaw += hx;
            // vertical
            float vy = -md.y * cameraSensitivity;
            if (cameraInvertY) vy = -vy;
            cameraPitch += vy;
            const float pitchLimit = 1.55f;
            if (cameraPitch > pitchLimit) cameraPitch = pitchLimit;
            if (cameraPitch < -pitchLimit) cameraPitch = -pitchLimit;
        }

        // Build forward vector from yaw/pitch
        Vector3 forward;
        forward.x = std::cos(cameraPitch) * std::sin(cameraYaw);
        forward.y = std::sin(cameraPitch);
        forward.z = std::cos(cameraPitch) * std::cos(cameraYaw);
        // normalize forward
        float flen = std::sqrt(forward.x*forward.x + forward.y*forward.y + forward.z*forward.z);
        if (flen > 1e-6f) { forward.x /= flen; forward.y /= flen; forward.z /= flen; }

        // right = cross(forward, up) so positive X movement strafes right
        Vector3 up = { 0.0f, 1.0f, 0.0f };
        Vector3 right;
        right.x = forward.y * up.z - forward.z * up.y;
        right.y = forward.z * up.x - forward.x * up.z;
        right.z = forward.x * up.y - forward.y * up.x;
        float rlen = std::sqrt(right.x*right.x + right.y*right.y + right.z*right.z);
        if (rlen > 1e-6f) { right.x /= rlen; right.y /= rlen; right.z /= rlen; }

        // Movement in world space
        Vector3 movement = { 0.0f, 0.0f, 0.0f };
        movement.x = (right.x * mx + up.x * my + forward.x * mz) * speed * dt;
        movement.y = (right.y * mx + up.y * my + forward.y * mz) * speed * dt;
        movement.z = (right.z * mx + up.z * my + forward.z * mz) * speed * dt;

        camera.position.x += movement.x;
        camera.position.y += movement.y;
        camera.position.z += movement.z;

        // Update target to look along forward
        camera.target.x = camera.position.x + forward.x;
        camera.target.y = camera.position.y + forward.y;
        camera.target.z = camera.position.z + forward.z;
}