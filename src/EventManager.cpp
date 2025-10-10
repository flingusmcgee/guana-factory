#include "EventManager.h"
#include "Event.h"

// Provide a global instance of the event manager
EventManager& EventManager::GetInstance() {
    static EventManager instance;
    return instance;
}

// Register a callback for a specific event type
void EventManager::Subscribe(EventType eventType, EventCallback callback) {
    subscribers[eventType].push_back(callback);
}

// Notify all subscribers aout n event
void EventManager::FireEvent(const Event& event) {
    auto it = subscribers.find(event.GetType());
    if (it != subscribers.end()) {
        for (auto& callback : it->second) {
            callback(event);
        }
    }
}