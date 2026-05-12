#pragma once

#include <functional>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include "SubscriptionHandle.h"

class EventBus {
    public:
        EventBus() = default;
        ~EventBus() = default;

        template <typename TEvent> 
        SubscriptionHandle subscribe(std::function<void(const TEvent&)> callback);

        template <typename TEvent>
        void publish(const TEvent& event);

    private:
        void unsubscribe(std::type_index type, int id);
        friend class SubscriptionHandle;

        using Callback = std::function<void(const void*)>;
        std::unordered_map<std::type_index, std::vector<std::pair<int, Callback>>> subscribers;
        int next_id = 0;
};

template <typename TEvent>
SubscriptionHandle EventBus::subscribe(std::function<void(const TEvent&)> callback) {
    int id = next_id++;

    Callback erased = [callback](const void* event) {
        callback(*static_cast<const TEvent*>(event));
    };

    subscribers[typeid(TEvent)].push_back({id, erased});

    return SubscriptionHandle(this, typeid(TEvent), id);
}

template <typename TEvent>
void EventBus::publish(const TEvent& event) {
    auto it = subscribers.find(typeid(TEvent));
    if (it == subscribers.end()) return;

    const void* raw = &event;
    for (const auto& pair : it->second) {
        pair.second(raw);
    }
}