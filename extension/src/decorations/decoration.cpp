#include "../../include/decorations/decoration.h"
#include "../../include/decoration_state.h"
#include "../../include/world.h"
#include "../../include/chunk.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void Decoration::_bind_methods() {
    BIND_PROPERTY(Decoration, String, internal_name);
    BIND_PROPERTY(Decoration, Vector3i, size);
    BIND_PROPERTY(Decoration, Vector3i, center_offset);
    BIND_PROPERTY(Decoration, Vector3i, cutscene_block_position);
    BIND_PROPERTY(Decoration, bool, has_cutscene_block);
    BIND_PROPERTY(Decoration, PackedInt32Array, blocks);
}

DEFINE_CONSTRUCTORS(Decoration);

DEFINE_PROPERTY_GETTER_SETTER(Decoration, bool, has_cutscene_block);
DEFINE_PROPERTY_GETTER_SETTER(Decoration, PackedInt32Array, blocks);
DEFINE_PROPERTY_GETTER_SETTER(Decoration, String, internal_name);
DEFINE_PROPERTY_GETTER_SETTER(Decoration, Vector3i, size);
DEFINE_PROPERTY_GETTER_SETTER(Decoration, Vector3i, center_offset);
DEFINE_PROPERTY_GETTER_SETTER(Decoration, Vector3i, cutscene_block_position);

int32_t Decoration::replace(Ref<DecorationState> decoration_state, Chunk* chunk, int32_t replace_number, Vector3i position, Vector3i decoration_position) {
    return 1; // Void
}
