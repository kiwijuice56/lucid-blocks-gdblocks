
#pragma once

#include "item.h"
#include "macros.h"

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/ref_counted.hpp>

namespace godot {

class ItemState : public Resource {
	DECLARE_CLASS(ItemState, Resource);

    DECLARE_PROPERTY(uint32_t, id, 0);
    DECLARE_PROPERTY(uint32_t, count, 1);
    DECLARE_PROPERTY(uint32_t, durability, 1);
    DECLARE_PROPERTY(Vector3i, position, Vector3i(0, 0, 0));

    void initialize(Ref<Item> item);
    PackedInt32Array get_save_data();
    void load_from_save_data(PackedInt32Array data);
};

}