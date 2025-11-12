#pragma once
#include "Event.h"
#include <vector>
#include <functional>
#include <map>
#include <unordered_map>

using EventCallback = std::function<void(const Event&)>;

class EventManager {
public:
    static EventManager& GetInstance(); // Good

    // Subscribe to an event type with a callback
    void Subscribe(EventType eventType, EventCallback callback);
    void FireEvent(const Event& event);

private:
    EventManager() {} // Private constructor for singleton
    // Subscribers mapped by event type
    std::unordered_map<EventType, std::vector<EventCallback>> subscribers; 
};