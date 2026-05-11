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