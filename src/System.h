#pragma once

#include <string>

class System {
public:
    virtual ~System() {}
    // Called once at startup. Return true on success.
    virtual bool Init() { return true; }
    // Called each frame with scaled delta time
    virtual void Update(float dt) { (void)dt; }
    // Called once at shutdown
    virtual void Shutdown() {}

    // Optional: textual name for debugging
    virtual std::string Name() const { return "System"; }
};
