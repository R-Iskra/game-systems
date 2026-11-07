#include "EventBus.h"

#include "SubscriptionHandle.h"

#include <algorithm>

namespace game_systems {

void EventBus::unsubscribe(std::type_index type, int id) {
    for (auto it = pending.begin(); it != pending.end(); ++it) {
        if (it->subscriber.id == id) {
            pending.erase(it);
            return;
        }
    }

    auto it = subscribers.find(type);
    if (it == subscribers.end()) return;

    auto& vec = it->second;

    if (publish_depth > 0) {
        for (auto& sub : vec) {
            if (sub.id == id) {
                sub.active = false;
                has_inactive = true;
                return;
            }
        }

        return;
    }

    vec.erase(
        std::remove_if(
            vec.begin(), vec.end(), [id](const Subscriber& sub) { return sub.id == id; }
        ),
        vec.end()
    );
}

void EventBus::flush_pending() {
    if (has_inactive) {
        for (auto& [type, vec] : subscribers) {
            vec.erase(
                std::remove_if(
                    vec.begin(), vec.end(), [](const Subscriber& sub) { return !sub.active; }
                ),
                vec.end()
            );
        }

        has_inactive = false;
    }

    for (auto& p : pending) {
        subscribers[p.type].push_back(std::move(p.subscriber));
    }

    pending.clear();
}

} // namespace game_systems