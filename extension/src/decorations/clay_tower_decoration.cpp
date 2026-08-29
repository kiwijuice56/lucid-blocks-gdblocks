#include "../../include/decorations/clay_tower_decoration.h"
#include "../../include/decoration_state.h"
#include "../../include/world.h"
#include "../../include/chunk.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void ClayTowerDecoration::_bind_methods() {
    BIND_REF_PROPERTY(ClayTowerDecoration, Block, default_block);
}

DEFINE_CONSTRUCTORS(ClayTowerDecoration);

int32_t ClayTowerDecoration::replace(Ref<DecorationState> decoration_state, Chunk* chunk, int32_t replace_number, Vector3i position, Vector3i decoration_position) {
    uint16_t rng_seed = world->generator->seed;
    Random::scramble_rng_seed(&rng_seed, decoration_position);

    if (replace_number == 10 && decoration_state->has_cutscene_block) {
        return world->block_name_map["cutscene block"];
    }

    Random::scramble_rng_seed(&rng_seed, position);
    if (replace_number == 1 && Random::randf(rng_seed) < 0.1) {
        return world->block_name_map["clay pot"];
    }

    return 1;
}

DEFINE_PROPERTY_GETTER_SETTER(ClayTowerDecoration, Ref<Block>, default_block);
