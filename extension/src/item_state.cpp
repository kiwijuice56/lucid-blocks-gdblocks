#include "../include/item_state.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void ItemState::_bind_methods() {
    BIND_METHOD(ItemState, initialize, "item");
    BIND_METHOD(ItemState, load_from_save_data, "data");
    BIND_METHOD_NO_PARAMETERS(ItemState, get_save_data);

    BIND_PROPERTY(ItemState, uint32_t, durability);
    BIND_PROPERTY(ItemState, uint32_t, id);
    BIND_PROPERTY(ItemState, uint32_t, count);
    BIND_PROPERTY(ItemState, Vector3i, position);
}

DEFINE_CONSTRUCTORS(ItemState);

void ItemState::initialize(Ref<Item> item) {
    id = item->id;
    count = 1;
    durability = item->max_durability;
    position = Vector3i();
}

PackedInt32Array ItemState::get_save_data() {
    PackedInt32Array data;
    data.resize(3);
    data[0] = id;
    data[1] = count;
    data[2] = durability;
    return data;
}

void ItemState::load_from_save_data(PackedInt32Array data) {
    id = data[0];
    count = data[1];
    durability = data[2];
}

DEFINE_PROPERTY_GETTER_SETTER(ItemState, uint32_t, id);
DEFINE_PROPERTY_GETTER_SETTER(ItemState, uint32_t, count);
DEFINE_PROPERTY_GETTER_SETTER(ItemState, uint32_t, durability);
DEFINE_PROPERTY_GETTER_SETTER(ItemState, Vector3i, position);