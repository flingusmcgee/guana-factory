#pragma once
#include <string>

namespace DebugHud {
    void Init();
    void Toggle();
    bool Visible();
    // extra is a C-string pointer (may be nullptr or empty)
    void Draw(int entities, const char* extra);
}
