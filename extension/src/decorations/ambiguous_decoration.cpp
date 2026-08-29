#include "../../include/decorations/ambiguous_decoration.h"
#include "../../include/decoration_state.h"
#include "../../include/world.h"
#include "../../include/chunk.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

DEFINE_CONSTRUCTORS_AND_BINDINGS(AmbiguousDecoration);

int32_t AmbiguousDecoration::replace(Ref<DecorationState> decoration_state, Chunk* chunk, int32_t replace_number, Vector3i position, Vector3i decoration_position) {
    if (replace_number == 1) {
        return world->block_name_map["ambiguous block 1"];
    } else if (replace_number == 2) {
        return world->block_name_map["ambiguous block 2"];
    } else if (replace_number == 3) {
        return world->block_name_map["ambiguous block 3"];
    } else {
        return world->block_name_map["ambiguous block 4"];
    }
}
