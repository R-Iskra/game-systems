#include <iostream>

#include "TestSystems.h"

int main() {
    EventBus bus;

    SoundSystem sound(bus);
    UISystem ui(bus);
    ScoreSystem score(bus);
    CameraSystem camera(bus);

    std::cout << "--- Enemy dies ---\n";
    bus.publish(EnemyDiedEvent{ 1, 100 });

    std::cout << "--- Player hurt ---\n";
    bus.publish(PlayerHurtEvent{ 25.f });

    std::cout << "--- ScoreSystem unsubscribes from EnemyDiedEvent ---\n";
    score.unsubscribe();

    std::cout << "--- Enemy dies again ---\n";
    bus.publish(EnemyDiedEvent{ 2, 200 });

    return 0;
}