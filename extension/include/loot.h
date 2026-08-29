
#pragma once

#include "item.h"
#include "item_state.h"
#include "macros.h"

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/ref_counted.hpp>

namespace godot {

class Loot : public Resource {
	DECLARE_CLASS(Loot, Resource);

    DECLARE_PROPERTY_NO_DEFAULT(TypedArray<Item>, items);
    DECLARE_PROPERTY_NO_DEFAULT(PackedFloat32Array, chances);
    DECLARE_PROPERTY_NO_DEFAULT(PackedInt32Array, counts);
    DECLARE_PROPERTY(bool, drop_one_item_at_random, false);

    TypedArray<ItemState> realize();
};
}