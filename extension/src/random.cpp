#include "../random.h"
#include "../decoration_state.h"

using namespace godot;

void Random::scramble_rng_seed(uint16_t* seed, Vector3i position) {
    uint32_t h = 0;

    h ^= (uint32_t)(position.x * 73856093);
    h ^= (uint32_t)(position.y * 19349663) << 5;
    h ^= (uint32_t)(position.z * 83492791) >> 3;

    h ^= h >> 13;
    h *= 0x5bd1e995;
    h ^= h >> 15;

    *seed ^= (uint16_t)(h ^ (h >> 16));
}

uint16_t Random::randi(uint16_t seed) {
    return seed;
}

double Random::randf(uint16_t seed) {
    return seed / 65536.0;
}

DecorationState::Direction Random::randdir(uint16_t seed) {
    int direction = randi(seed) % 4;
    if (direction == 1) {
        return DecorationState::East;
    } else if (direction == 2) {
        return DecorationState::South;
    } else if (direction == 3) {
        return DecorationState::West;
    } else {
        return DecorationState::North;
    }
}