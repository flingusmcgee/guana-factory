#pragma once
#include "Entity.h"

enum class EventType {
    Collision,
    EntityDestroyed,
    // Add future event types here
};

// The blueprint for all events
class Event {
public:
    virtual ~Event() = default;
    virtual EventType GetType() const = 0;
};

// Event for when two entities collide.
struct CollisionEvent : public Event {
    Entity* entityA;
    Entity* entityB;

    EventType GetType() const override {
        return EventType::Collision;
    }
};