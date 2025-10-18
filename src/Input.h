#pragma once
#include <string>
#include <vector>
#include <map>

// Simple input mapping layer. Actions are strings mapped to one or more keys.
namespace Input {
    void Init();
    void LoadFromConfig();
    void BindDefault();
    void Update();

    bool IsDown(const std::string& action);
    bool WasPressed(const std::string& action);
    bool WasReleased(const std::string& action);

    // low-level utility
    int StringToKey(const std::string& name);
}
