#include "../include/item.h"


#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void Item::_bind_methods() {
    BIND_METHOD(Item, set_stack_size_increase, "new_val");

    ADD_GROUP("Identity", "");
    BIND_PROPERTY(Item, uint32_t, id);
    BIND_PROPERTY(Item, bool, internal);
    BIND_REF_PROPERTY(Item, Essence, essence);

    ADD_SUBGROUP("Name", "");
    BIND_PROPERTY(Item, String, display_name);
    BIND_PROPERTY(Item, String, internal_name);

    ADD_GROUP("Appearance", "");
    BIND_REF_PROPERTY(Item, Texture2D, icon);

    ADD_GROUP("Behavior", "");
    BIND_REF_PROPERTY(Item, PackedScene, held_item_scene);
    BIND_PROPERTY(Item, String, held_item_path);
    BIND_PROPERTY(Item, uint32_t, stack_size);
    BIND_PROPERTY(Item, uint32_t, max_durability);

    ADD_GROUP("Internal (automatically generated)", "");
    BIND_PROPERTY(Item, Color, color);
    BIND_REF_PROPERTY(Item, Mesh, mesh);
}

DEFINE_CONSTRUCTORS(Item);

uint32_t Item::get_stack_size() const {
    return UtilityFunctions::clampi(stack_size != 50 ? stack_size : stack_size_increase + stack_size, 0, 999);
}

void Item::set_stack_size(uint32_t new_stack_size) {
    stack_size = new_stack_size;
}

void Item::set_stack_size_increase(int increase) {
    Item::stack_size_increase = increase;
}

int Item::stack_size_increase = 0;

DEFINE_PROPERTY_GETTER_SETTER(Item, uint32_t, id);
DEFINE_PROPERTY_GETTER_SETTER(Item, uint32_t, max_durability);
DEFINE_PROPERTY_GETTER_SETTER(Item, String, display_name);
DEFINE_PROPERTY_GETTER_SETTER(Item, String, internal_name);
DEFINE_PROPERTY_GETTER_SETTER(Item, Ref<Texture2D>, icon);
DEFINE_PROPERTY_GETTER_SETTER(Item, String, held_item_path);
DEFINE_PROPERTY_GETTER_SETTER(Item, Ref<PackedScene>, held_item_scene);
DEFINE_PROPERTY_GETTER_SETTER(Item, Ref<Mesh>, mesh);
DEFINE_PROPERTY_GETTER_SETTER(Item, Ref<Essence>, essence);
DEFINE_PROPERTY_GETTER_SETTER(Item, Color, color);
DEFINE_PROPERTY_GETTER_SETTER(Item, bool, internal);