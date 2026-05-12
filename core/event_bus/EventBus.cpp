#include "EventBus.h"
#include "SubscriptionHandle.h"

#include <algorithm>
#include <cstdio>

void EventBus::unsubscribe(std::type_index type, int id) {
    auto it = subscribers.find(type);
    if (it == subscribers.end()) return;

    auto& vec = it->second;
    vec.erase(
        std::remove_if(vec.begin(), vec.end(), [id](const std::pair<int, Callback>& pair) {
            return pair.first == id;
        }),
        vec.end()
    );
}