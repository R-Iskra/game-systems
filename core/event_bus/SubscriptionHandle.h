#pragma once

#include <typeindex>

class EventBus;

class SubscriptionHandle {
    public:
        SubscriptionHandle() : bus(nullptr), type(typeid(void)), id(-1) {}

        SubscriptionHandle(EventBus* bus, std::type_index type, int id);
        ~SubscriptionHandle();

        SubscriptionHandle(const SubscriptionHandle&) = delete;
        SubscriptionHandle& operator=(const SubscriptionHandle&) = delete;

        SubscriptionHandle(SubscriptionHandle&& other) noexcept;
        SubscriptionHandle& operator=(SubscriptionHandle&& other) noexcept;

    private:
        EventBus* bus;
        std::type_index type;
        int id;
};