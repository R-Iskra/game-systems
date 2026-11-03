#pragma once

#include "SubscriptionHandle.h"

#include <functional>
#include <typeindex>
#include <unordered_map>
#include <vector>
#include <utility>

class EventBus {
    public:
        EventBus() = default;
        ~EventBus() = default;

        EventBus(const EventBus&) = delete;
        EventBus& operator=(const EventBus&) = delete;
        EventBus(EventBus&&) = delete;
        EventBus& operator=(EventBus&&) = delete;

        template <typename TEvent> 
        SubscriptionHandle subscribe(std::function<void(const TEvent&)> callback);

        template <typename TEvent>
        void publish(const TEvent& event);

    private:
        void unsubscribe(std::type_index type, int id);
        void flush_pending();
        friend class SubscriptionHandle;

        using Callback = std::function<void(const void*)>;

        struct Subscriber {
            int id;
            Callback callback;
            bool active = true;
        };

        struct PendingSubscriber {
            std::type_index type;
            Subscriber subscriber;
        };

        struct PublishGuard {
            EventBus& bus;
            explicit PublishGuard(EventBus& bus) : bus(bus) { ++bus.publish_depth; }
            ~PublishGuard() {
                if (--bus.publish_depth == 0) bus.flush_pending();
            }
        };

        std::unordered_map<std::type_index, std::vector<Subscriber>> subscribers;

        std::vector<PendingSubscriber> pending;
        int publish_depth = 0;
        bool has_inactive = false;

        int next_id = 0;
};

template <typename TEvent>
SubscriptionHandle EventBus::subscribe(std::function<void(const TEvent&)> callback) {
    int id = next_id++;

    Callback erased = [callback = std::move(callback)](const void* event) {
        callback(*static_cast<const TEvent*>(event));
    };

    Subscriber sub{ id, std::move(erased) };

    if (publish_depth > 0) {
        pending.push_back({ typeid(TEvent), std::move(sub) });
    } else {
        subscribers[typeid(TEvent)].push_back(std::move(sub));
    }

    return SubscriptionHandle(this, typeid(TEvent), id);
}

template <typename TEvent>
void EventBus::publish(const TEvent& event) {
    auto it = subscribers.find(typeid(TEvent));
    if (it == subscribers.end()) return;

    PublishGuard guard(*this);

    const void* raw = &event;
    for (const auto& sub : it->second) {
        if (sub.active) {
            sub.callback(raw);
        }
    }
}