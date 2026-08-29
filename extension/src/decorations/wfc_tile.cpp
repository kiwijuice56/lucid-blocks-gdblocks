#include "../../include/decorations/decoration.h"
#include "../../include/decorations/wfc_tile.h"
#include "../../include/world.h"
#include "../../include/chunk.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void WfcTile::_bind_methods() {
    ADD_GROUP("Identity", "");
    BIND_PROPERTY(WfcTile, String, internal_name);

    ADD_GROUP("Content", "");
    BIND_BITMASK_PROPERTY(WfcTile, Variant::BOOL, directional_variants);
    BIND_REF_ARRAY_PROPERTY(WfcTile, Decoration, decorations);

    ADD_GROUP("Masks", "");
    BIND_BITMASK_PROPERTY(WfcTile, Variant::INT, t_mask);
    BIND_BITMASK_PROPERTY(WfcTile, Variant::INT, b_mask);
    BIND_BITMASK_PROPERTY(WfcTile, Variant::INT, n_mask);
    BIND_BITMASK_PROPERTY(WfcTile, Variant::INT, e_mask);
    BIND_BITMASK_PROPERTY(WfcTile, Variant::INT, s_mask);
    BIND_BITMASK_PROPERTY(WfcTile, Variant::INT, w_mask);
}

DEFINE_CONSTRUCTORS(WfcTile);

DEFINE_PROPERTY_GETTER_SETTER(WfcTile, bool, directional_variants);
DEFINE_PROPERTY_GETTER_SETTER(WfcTile, uint32_t, t_mask);
DEFINE_PROPERTY_GETTER_SETTER(WfcTile, uint32_t, b_mask);
DEFINE_PROPERTY_GETTER_SETTER(WfcTile, uint32_t, n_mask);
DEFINE_PROPERTY_GETTER_SETTER(WfcTile, uint32_t, e_mask);
DEFINE_PROPERTY_GETTER_SETTER(WfcTile, uint32_t, s_mask);
DEFINE_PROPERTY_GETTER_SETTER(WfcTile, uint32_t, w_mask);
DEFINE_PROPERTY_GETTER_SETTER(WfcTile, String, internal_name);
DEFINE_PROPERTY_GETTER_SETTER(WfcTile, TypedArray<Decoration>, decorations);
