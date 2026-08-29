#pragma once

#include <godot_cpp/core/class_db.hpp>
#include "../decoration_state.h"

namespace godot {

class Random {

public:
    static void scramble_rng_seed(uint16_t* seed, Vector3i position);
    static uint16_t randi(uint16_t seed);
    static double randf(uint16_t seed);
    static DecorationState::Direction randdir(uint16_t seed);
};

}