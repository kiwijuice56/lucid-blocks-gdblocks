#include "../include/loot.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void Loot::_bind_methods() {
    BIND_METHOD_NO_PARAMETERS(Loot, realize);

    BIND_REF_ARRAY_PROPERTY(Loot, Item, items);
    BIND_PROPERTY(Loot, PackedFloat32Array, chances);
    BIND_PROPERTY(Loot, PackedInt32Array, counts);
    BIND_PROPERTY(Loot, bool, drop_one_item_at_random);
}

DEFINE_CONSTRUCTORS(Loot);

TypedArray<ItemState> Loot::realize() {
    if (drop_one_item_at_random) {
        float total_weight = 0.0f;
        for (int32_t i = 0; i < items.size(); i++) {
            total_weight += chances[i];
        }
        
        float random_value = UtilityFunctions::randf() * total_weight;
        float cumulative = 0.0f;
        int32_t selected_index = 0;
        
        for (int32_t i = 0; i < items.size(); i++) {
            cumulative += chances[i];
            if (random_value <= cumulative) {
                selected_index = i;
                break;
            }
        }
        
        int32_t random_count = UtilityFunctions::randi_range(1, counts[selected_index]);
        TypedArray<ItemState> drops;
        for (int32_t j = 0; j < random_count; j++) {
            Ref<ItemState> new_drop = memnew(ItemState);
            new_drop->initialize(items[selected_index]);
            drops.append(new_drop);
        }
        
        return drops;
    }

    TypedArray<ItemState> drops;
    for (int32_t i = 0; i < items.size(); i++) {
        Ref<Item> item = items[i];
        for (int32_t j = 0; j < counts[i]; j++) {
            if (UtilityFunctions::randf() < chances[i]) {
                Ref<ItemState> new_drop = memnew(ItemState);
                new_drop->initialize(item);
                drops.append(new_drop);
            }
        }
    }
    return drops;
}

DEFINE_PROPERTY_GETTER_SETTER(Loot, bool, drop_one_item_at_random);
DEFINE_PROPERTY_GETTER_SETTER(Loot, TypedArray<Item>, items);
DEFINE_PROPERTY_GETTER_SETTER(Loot, PackedFloat32Array, chances);
DEFINE_PROPERTY_GETTER_SETTER(Loot, PackedInt32Array, counts);