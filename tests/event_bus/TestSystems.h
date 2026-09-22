#pragma once

#include "../../core/event_bus/EventBus.h"

struct EnemyDiedEvent {
    int enemy_id;
    int score_value;
};

struct PlayerHurtEvent {
    float damage;
};

class SoundSystem {
    public:
        SoundSystem(EventBus& bus) {
            enemy_died_sub = bus.subscribe<EnemyDiedEvent>([this](const EnemyDiedEvent& e) {
                std::cout << "SoundSystem: playing enemy death sound\n";
            });
            player_hurt_sub = bus.subscribe<PlayerHurtEvent>([this](const PlayerHurtEvent& e) {
                std::cout << "SoundSystem: playing player hurt sound\n";
            });
        }
    
    private:
        SubscriptionHandle enemy_died_sub;
        SubscriptionHandle player_hurt_sub;
};

class UISystem {
    public:
        UISystem(EventBus& bus) {
            enemy_died_sub = bus.subscribe<EnemyDiedEvent>([this](const EnemyDiedEvent& e) {
                kill_count++;
                std::cout << "UISystem: enemy " << e.enemy_id << " died, updating kill counter to " << kill_count << "\n";
            });
            player_hurt_sub = bus.subscribe<PlayerHurtEvent>([this](const PlayerHurtEvent& e) {
                player_health -= e.damage;
                std::cout << "UISystem: player hurt for " << e.damage <<  " health now " << player_health << "\n";
            });
        }

    private:
        SubscriptionHandle enemy_died_sub;
        SubscriptionHandle player_hurt_sub;
        int kill_count = 0;
        int player_health = 100;
};

class ScoreSystem {
    public:
        ScoreSystem(EventBus& bus) {
            enemy_died_sub = bus.subscribe<EnemyDiedEvent>([this](const EnemyDiedEvent& e) {
                score += e.score_value;
                std::cout << "ScoreSystem: enemy " << e.enemy_id << " died, score now " << score << "\n";
            });
        }

        void unsubscribe() {
            enemy_died_sub = SubscriptionHandle();
        }

    private:
        SubscriptionHandle enemy_died_sub;
        int score = 0;
};

class CameraSystem {
    public:
        CameraSystem(EventBus& bus) {
            player_hurt_sub = bus.subscribe<PlayerHurtEvent>([this](const PlayerHurtEvent& e) {
                std::cout << "CameraSystem: player hurt, *screen shake*\n";
            });
        }

    private:
        SubscriptionHandle player_hurt_sub;
};