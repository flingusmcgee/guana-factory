#pragma once
#include <string>

namespace DebugHud {
    void Init();
    void Toggle();
    bool Visible();
    void Draw(int fps, int entities, const std::string& extra);
}
