#pragma once

struct Vector2 {
    float x = 0.f;
    float y = 0.f;
};

struct EntityContext {
    Vector2 position;
    Vector2 velocity;
    bool is_on_floor = false;
    float health = 100.f;

    bool input_move_left = false;
    bool input_move_right = false;
    bool input_jump = false;
};