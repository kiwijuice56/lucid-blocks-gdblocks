#include "../../include/structures/decoration_structure.h"
#include "../../include/world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void DecorationStructure::_bind_methods() {
    BIND_REF_PROPERTY(DecorationStructure, Decoration, decoration);
    BIND_PROPERTY(DecorationStructure, bool, perfectly_centered);
    BIND_PROPERTY(DecorationStructure, bool, ignore_ground_level);
}

DEFINE_CONSTRUCTORS(DecorationStructure);

void DecorationStructure::generate_chunk_data(Chunk* chunk, Vector3i chunk_position) {
    Generator::fill_large_decoration(world, chunk, chunk_position, decoration, placed_decoration, false);
}

void DecorationStructure::initialize() {
    uint16_t rng_seed = world->generator->seed;
    Random::scramble_rng_seed(&rng_seed, heart_position);

    placed_decoration.instantiate();
    placed_decoration->decoration = decoration;
    placed_decoration->has_cutscene_block = true;

    uint16_t i = Random::randi(rng_seed) % 4;
    if (i == 0) {
        placed_decoration->direction = DecorationState::North;
    } else if (i == 1) {
        placed_decoration->direction = DecorationState::South;
    } else if (i == 2) {
        placed_decoration->direction = DecorationState::East;
    } else {
        placed_decoration->direction = DecorationState::West;
    }

    placed_decoration->position = perfectly_centered ? grid_center_grounded_position : heart_position;

    if (ignore_ground_level) {
        placed_decoration->position.y = center_position.y;
    }
}

Vector3i DecorationStructure::get_cutscene_block_position() {
    UtilityFunctions::print(internal_name, " position: ", placed_decoration->position);
    return Generator::find_actual_block_position(
        placed_decoration->decoration->cutscene_block_position,
        placed_decoration->position, placed_decoration->decoration->size, placed_decoration->decoration->center_offset,
        placed_decoration->direction
    );
}

bool DecorationStructure::has_cutscene_block() {
    return decoration->has_cutscene_block;
}

bool DecorationStructure::is_within_structure(Vector3i position) {
    return false;
}

DEFINE_PROPERTY_GETTER_SETTER(DecorationStructure, bool, perfectly_centered);
DEFINE_PROPERTY_GETTER_SETTER(DecorationStructure, bool, ignore_ground_level);
DEFINE_PROPERTY_GETTER_SETTER(DecorationStructure, Ref<Decoration>, decoration);
