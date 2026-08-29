#include "../../include/decorations/rock_decoration.h"
#include "../../include/world.h"
#include "../../include/chunk.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void RockDecoration::_bind_methods() {
    BIND_REF_PROPERTY(RockDecoration, Block, main_block_type);
    BIND_REF_PROPERTY(RockDecoration, Block, ore_block_type);
}

DEFINE_CONSTRUCTORS(RockDecoration);

int32_t RockDecoration::replace(Ref<DecorationState> decoration_state, Chunk* chunk, int32_t replace_number, Vector3i position, Vector3i decoration_position) {
    // Remove grass on top of this rock
    if (replace_number == 1) {
        if (chunk->is_block_foliage[chunk->get_block_index_at(position)]) {
            return 1;
        } else {
            return 0;
        }
    }

    return UtilityFunctions::randf() < 0.25 ? ore_block_type->index : main_block_type->index;
}

DEFINE_PROPERTY_GETTER_SETTER(RockDecoration, Ref<Block>, main_block_type);
DEFINE_PROPERTY_GETTER_SETTER(RockDecoration, Ref<Block>, ore_block_type);