#include "../../include/structures/wood_temple.h"
#include "../../include/world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

DEFINE_CONSTRUCTORS_AND_BINDINGS(WoodTemple);

int WoodTemple::update_rule(int x, int y, int z, int type) {
    const int WOOD = 0;
    const int LEAVES = 1;
    const int VEIN = 2; 
    const int BUD = 3; 

    uint16_t rng_seed = world->generator->seed;
    Random::scramble_rng_seed(&rng_seed, heart_position + Vector3i(x * 7, y * 13, z * 21));

    int wood_touch = touch_count(x, y, z, WOOD);
    int leaf_touch = touch_count(x, y, z, LEAVES);
    int vein_touch = touch_count(x, y, z, VEIN);
    
    if (type == AIR) {
        if (wood_touch >= 1 && Random::randf(rng_seed) < (0.02 + vein_touch * 0.03)) {
            return BUD;
        } else if (leaf_touch >= 2 && Random::randf(rng_seed) < 0.04) {
            return LEAVES;
        }
    }

    int bud_touch  = touch_count(x, y, z, BUD);
    if (type == WOOD) {
        if (Random::randf(rng_seed) < 0.03 && wood_touch >= 3 && wood_touch <= 6) {
            return VEIN; 
        } else if (wood_touch <= 2 && Random::randf(rng_seed) < 0.2 + z / 512.0) {
            return AIR;
        }
        return WOOD;
    }

    if (type == VEIN) {
        if (wood_touch < 2) {
            return AIR;
        } else if (Random::randf(rng_seed) < 0.03 + z / 800.0) {
            return AIR;
        }
        return VEIN;
    }

    if (type == BUD) {
        double up_bias = (z > BOUND_SIZE / 2) ? 0.15 : -0.05;
        if (Random::randf(rng_seed) < (0.4 + vein_touch * 0.3 + up_bias)) {
            return WOOD;
        } else if (leaf_touch >= 1 || Random::randf(rng_seed) < 0.15) {
            return LEAVES;
        }   
        return BUD;
    }

    if (type == LEAVES) {
        if (leaf_touch >= 5) {
            return AIR;
        } else if (wood_touch == 0 && Random::randf(rng_seed) < 0.25) {
            return AIR;
        }   
        return LEAVES;
    }

    return type;
}

