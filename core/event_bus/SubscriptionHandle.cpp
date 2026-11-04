#include "SubscriptionHandle.h"

#include "EventBus.h"

#include <cstdio>

namespace game_systems {

SubscriptionHandle::SubscriptionHandle(EventBus* bus, std::type_index type, int id)
    : bus(bus), type(type), id(id)
{}

SubscriptionHandle::~SubscriptionHandle() {
    if (bus != nullptr) {
        bus->unsubscribe(type, id);
    }
}

SubscriptionHandle::SubscriptionHandle(SubscriptionHandle&& other) noexcept 
    : bus(other.bus), type(other.type), id(other.id)
{
    other.bus = nullptr;
}

SubscriptionHandle& SubscriptionHandle::operator=(SubscriptionHandle&& other) noexcept {
    if (this != &other) {
        if (bus != nullptr) {
            bus->unsubscribe(type, id);
        }
        bus = other.bus;
        type = other.type;
        id = other.id;
        other.bus = nullptr;
    }
    return *this;
}

} // namespace game_systems