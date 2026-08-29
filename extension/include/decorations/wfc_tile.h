#pragma once
#include "../include/macros.h"
#include "../include/decoration_state.h"

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/classes/ref_counted.hpp>


namespace godot {

class Decoration;

class WfcTile : public Resource {
	DECLARE_CLASS(WfcTile, Resource);

    DECLARE_PROPERTY(String, internal_name, "");
    DECLARE_PROPERTY_NO_DEFAULT(TypedArray<Decoration>, decorations);
    DecorationState::Direction direction = DecorationState::Direction::North;

    DECLARE_PROPERTY(bool, directional_variants, false);
    DECLARE_PROPERTY(uint32_t, t_mask, 0);
    DECLARE_PROPERTY(uint32_t, b_mask, 0);
    DECLARE_PROPERTY(uint32_t, n_mask, 0);
    DECLARE_PROPERTY(uint32_t, e_mask, 0);
    DECLARE_PROPERTY(uint32_t, s_mask, 0);
    DECLARE_PROPERTY(uint32_t, w_mask, 0);

};

}