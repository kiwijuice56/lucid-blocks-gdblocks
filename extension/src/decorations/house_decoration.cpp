#include "../../include/decorations/house_decoration.h"
#include "../../include/decoration_state.h"
#include "../../include/world.h"
#include "../../include/chunk.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void HouseDecoration::_bind_methods() {
    BIND_REF_PROPERTY(HouseDecoration, Block, default_block);
}

DEFINE_CONSTRUCTORS(HouseDecoration);

int32_t HouseDecoration::replace(Ref<DecorationState> decoration_state, Chunk* chunk, int32_t replace_number, Vector3i position, Vector3i decoration_position) {
    uint16_t rng_seed = world->generator->seed;

    Random::scramble_rng_seed(&rng_seed, decoration_position);
    if (replace_number == 1) {
        if (Random::randf(rng_seed) < 0.25) {
            return world->block_name_map["glass"];
        } else {
            return default_block->index;
        }
    }

    Random::scramble_rng_seed(&rng_seed, decoration_position);
    if (replace_number == 2) {
       if (Random::randf(rng_seed) < 0.25) {
            return world->block_name_map["glass"];
        } else {
            return default_block->index;
        }
    }

    Random::scramble_rng_seed(&rng_seed, decoration_position);
    if (replace_number == 3) {
        if (Random::randf(rng_seed) < 0.35) {
            return world->block_name_map["glass"];
        } else {
            return default_block->index;
        }
    }

    Random::scramble_rng_seed(&rng_seed, decoration_position);
    if (replace_number == 4) {
        if (Random::randf(rng_seed) < 0.5) {
            return 0;
        } else {
            return default_block->index;
        }
    }

    Random::scramble_rng_seed(&rng_seed, decoration_position);
    if (replace_number == 5 && Random::randf(rng_seed) < 0.25) {
        return world->block_name_map["bookshelf block"];
    }

    Random::scramble_rng_seed(&rng_seed, decoration_position);
    if (replace_number == 8) {
        if (Random::randf(rng_seed) < 0.35) {
            return world->block_name_map["lamp y-"];
        } else {
            return default_block->index;
        }
    }

    if (replace_number == 10 && decoration_state->has_cutscene_block) {
        return world->block_name_map["cutscene block"];
    }

    Random::scramble_rng_seed(&rng_seed, 13 * position);
    if (replace_number == 7 && Random::randf(rng_seed) < 0.2) {
        Random::scramble_rng_seed(&rng_seed, 17 * position);
        if (Random::randf(rng_seed) < 0.2) {
            return world->block_name_map["hope cube"];
        } else if (Random::randf(rng_seed) < 0.4) {
            return world->block_name_map["cabinet"];
        } else if (Random::randf(rng_seed) < 0.5) {
            return  world->block_name_map["letter cube"];
        } else if (Random::randf(rng_seed) < 0.6) {
            return  world->block_name_map["cache cube"];
        } else if (Random::randf(rng_seed) < 0.8) {
            return world->block_name_map["sponge block"];
        }  else {
            return world->block_name_map["diffuse block"];
        }
    }

    Random::scramble_rng_seed(&rng_seed, 7 * position);
    if (replace_number == 6 && Random::randf(rng_seed) < 0.45) {
        return world->block_name_map["cardboard box"];
    }

    return 1;
}

DEFINE_PROPERTY_GETTER_SETTER(HouseDecoration, Ref<Block>, default_block);
