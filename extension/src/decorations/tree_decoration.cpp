#include "../../include/decorations/tree_decoration.h"
#include "../../include/decoration_state.h"
#include "../../include/world.h"
#include "../../include/chunk.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void TreeDecoration::_bind_methods() {
    BIND_REF_PROPERTY(TreeDecoration, Block, leaf_block);
}

DEFINE_CONSTRUCTORS(TreeDecoration);

int32_t TreeDecoration::replace(Ref<DecorationState> decoration_state, Chunk* chunk, int32_t replace_number, Vector3i position, Vector3i decoration_position) {
    uint16_t rng_seed = world->generator->seed;
    if (replace_number == 1) {
        Random::scramble_rng_seed(&rng_seed, position);
        return Random::randf(rng_seed) < 0.5 ? leaf_block->index : 0;
    } else if (replace_number == 2) {
        Random::scramble_rng_seed(&rng_seed, position);
        return Random::randf(rng_seed) < 0.5 ? (int) world->block_name_map["wood block"] : leaf_block->index;
    } else {
        return 0;
    }
}

DEFINE_PROPERTY_GETTER_SETTER(TreeDecoration, Ref<Block>, leaf_block);
