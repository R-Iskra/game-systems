#pragma once

#include "event_bus/EventBus.h"

#include <iostream>

struct EnemyDiedEvent {
    int enemy_id;
    int score_value;
};

struct PlayerHurtEvent {
    float damage;
};

class SoundSystem {
    public:
        SoundSystem(game_systems::EventBus& bus) {
            enemy_died_sub = bus.subscribe<EnemyDiedEvent>([this](const EnemyDiedEvent& e) {
                std::cout << "SoundSystem: playing enemy death sound\n";
            });
            player_hurt_sub = bus.subscribe<PlayerHurtEvent>([this](const PlayerHurtEvent& e) {
                std::cout << "SoundSystem: playing player hurt sound\n";
            });
        }
    
    private:
        game_systems::SubscriptionHandle enemy_died_sub;
        game_systems::SubscriptionHandle player_hurt_sub;
};

class UISystem {
    public:
        UISystem(game_systems::EventBus& bus) {
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
        game_systems::SubscriptionHandle enemy_died_sub;
        game_systems::SubscriptionHandle player_hurt_sub;
        int kill_count = 0;
        int player_health = 100;
};

class ScoreSystem {
    public:
        ScoreSystem(game_systems::EventBus& bus) {
            enemy_died_sub = bus.subscribe<EnemyDiedEvent>([this](const EnemyDiedEvent& e) {
                score += e.score_value;
                std::cout << "ScoreSystem: enemy " << e.enemy_id << " died, score now " << score << "\n";
            });
        }

        void unsubscribe() {
            enemy_died_sub = game_systems::SubscriptionHandle();
        }

    private:
        game_systems::SubscriptionHandle enemy_died_sub;
        int score = 0;
};

class CameraSystem {
    public:
        CameraSystem(game_systems::EventBus& bus) {
            player_hurt_sub = bus.subscribe<PlayerHurtEvent>([this](const PlayerHurtEvent& e) {
                std::cout << "CameraSystem: player hurt, *screen shake*\n";
            });
        }

    private:
        game_systems::SubscriptionHandle player_hurt_sub;
};

class AchievementSystem {
    public:
        AchievementSystem(game_systems::EventBus& bus) {
            enemy_died_sub = bus.subscribe<EnemyDiedEvent>([this](const EnemyDiedEvent& e) {
                std::cout << "AchievementSystem: enemy " << e.enemy_id << " died, unlocked 'First Blood'\n";
                enemy_died_sub = game_systems::SubscriptionHandle();
            });
        }

    private:
        game_systems::SubscriptionHandle enemy_died_sub;
};