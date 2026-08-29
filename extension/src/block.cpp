#include "../include/block.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void Block::_bind_methods() {
    BIND_METHOD_NO_PARAMETERS(Block, get_index);

    ADD_GROUP("Appearance", "");
    BIND_REF_PROPERTY(Block, Texture2D, texture);
    BIND_REF_PROPERTY(Block, Texture2D, metallic_texture);
    BIND_REF_PROPERTY(Block, Texture2D, roughness_texture);
    BIND_PROPERTY(Block, bool, textureless);
    BIND_PROPERTY(Block, bool, override_icon);
    BIND_PROPERTY(Block, bool, transparent);
    BIND_PROPERTY(Block, bool, top_face_random);

    ADD_SUBGROUP("Foliage", "");
    BIND_PROPERTY(Block, bool, foliage);
    BIND_PROPERTY(Block, bool, foliage_can_float);

    ADD_GROUP("Sound", "");
    BIND_ENUM_PROPERTY(Block, break_sound_type, "None,Dirt,Stone,Wood,Glass,Metal,Meat,Cardboard,Plastic,Leaves,Snow,Grass,Cloth");
    BIND_ENUM_PROPERTY(Block, step_sound_type, "None,Dirt,Stone,Wood,Glass,Metal,Meat,Cardboard,Plastic,Leaves,Snow,Grass,Cloth");

    BIND_ENUM_CONSTANT(SOUND_NONE);
    BIND_ENUM_CONSTANT(SOUND_DIRT);
    BIND_ENUM_CONSTANT(SOUND_STONE);
    BIND_ENUM_CONSTANT(SOUND_WOOD);
    BIND_ENUM_CONSTANT(SOUND_GLASS);
    BIND_ENUM_CONSTANT(SOUND_METAL);
    BIND_ENUM_CONSTANT(SOUND_MEAT);
    BIND_ENUM_CONSTANT(SOUND_CARDBOARD);
    BIND_ENUM_CONSTANT(SOUND_PLASTIC);
    BIND_ENUM_CONSTANT(SOUND_LEAVES);
    BIND_ENUM_CONSTANT(SOUND_SNOW);
    BIND_ENUM_CONSTANT(SOUND_GRASS);
    BIND_ENUM_CONSTANT(SOUND_CLOTH);

    BIND_REF_PROPERTY(Block, AudioStream, break_sound);
    BIND_REF_PROPERTY(Block, AudioStream, step_sound);

    ADD_GROUP("Behavior", "");
    BIND_PROPERTY(Block, bool, no_collision_block);
    BIND_PROPERTY(Block, bool, unbreakable);
    BIND_PROPERTY(Block, bool, griefable);
    BIND_REF_PROPERTY(Block, PackedScene, living_block_scene);
    BIND_PROPERTY(Block, String, living_block_path);
    BIND_PROPERTY(Block, float, flammability);
    BIND_PROPERTY(Block, bool, sustain_fire);

    ADD_SUBGROUP("Drops", "");
    BIND_PROPERTY(Block, bool, can_drop);
    BIND_REF_PROPERTY(Block, Item, drop_item);
    BIND_REF_PROPERTY(Block, Loot, drop_loot);

    ADD_SUBGROUP("Direction", "");
    BIND_PROPERTY(Block, bool, directional);
    BIND_PROPERTY(Block, Vector3i, direction);

    ADD_GROUP("Mining", "");
    BIND_PROPERTY(Block, bool, pickaxe_affinity);
    BIND_PROPERTY(Block, bool, pickaxe_required);
    BIND_PROPERTY(Block, bool, meat_affinity);
    BIND_PROPERTY(Block, bool, plant_affinity);
    BIND_PROPERTY(Block, bool, shovel_affinity);
    BIND_PROPERTY(Block, bool, axe_affinity);
    BIND_PROPERTY(Block, bool, axe_required);
    BIND_PROPERTY(Block, float, break_time);
}

DEFINE_CONSTRUCTORS(Block);

bool Block::get_foliage() const {
    return foliage;
}

void Block::set_foliage(bool new_val) {
    if (new_val) {
        set_transparent(true);
    } else {
        set_foliage_can_float(false);
    }
    foliage = new_val;
}

uint32_t Block::get_index() const {
    return index;
}

DEFINE_PROPERTY_GETTER_SETTER(Block, Block::SoundType, break_sound_type);
DEFINE_PROPERTY_GETTER_SETTER(Block, Block::SoundType, step_sound_type);
DEFINE_PROPERTY_GETTER_SETTER(Block, String, living_block_path);
DEFINE_PROPERTY_GETTER_SETTER(Block, Ref<Texture2D>, texture);
DEFINE_PROPERTY_GETTER_SETTER(Block, Ref<Texture2D>, metallic_texture);
DEFINE_PROPERTY_GETTER_SETTER(Block, Ref<Texture2D>, roughness_texture);
DEFINE_PROPERTY_GETTER_SETTER(Block, Ref<AudioStream>, break_sound);
DEFINE_PROPERTY_GETTER_SETTER(Block, Ref<AudioStream>, step_sound);
DEFINE_PROPERTY_GETTER_SETTER(Block, Ref<Item>, drop_item);
DEFINE_PROPERTY_GETTER_SETTER(Block, Ref<Loot>, drop_loot);
DEFINE_PROPERTY_GETTER_SETTER(Block, Ref<PackedScene>, living_block_scene);
DEFINE_PROPERTY_GETTER_SETTER(Block, bool, foliage_can_float);
DEFINE_PROPERTY_GETTER_SETTER(Block, bool, transparent);
DEFINE_PROPERTY_GETTER_SETTER(Block, bool, directional);
DEFINE_PROPERTY_GETTER_SETTER(Block, Vector3i, direction);
DEFINE_PROPERTY_GETTER_SETTER(Block, bool, can_drop);
DEFINE_PROPERTY_GETTER_SETTER(Block, bool, axe_affinity);
DEFINE_PROPERTY_GETTER_SETTER(Block, bool, pickaxe_affinity);
DEFINE_PROPERTY_GETTER_SETTER(Block, bool, axe_required);
DEFINE_PROPERTY_GETTER_SETTER(Block, bool, pickaxe_required);
DEFINE_PROPERTY_GETTER_SETTER(Block, bool, sustain_fire);
DEFINE_PROPERTY_GETTER_SETTER(Block, float, break_time);
DEFINE_PROPERTY_GETTER_SETTER(Block, float, flammability);
DEFINE_PROPERTY_GETTER_SETTER(Block, bool, override_icon);
DEFINE_PROPERTY_GETTER_SETTER(Block, bool, textureless);
DEFINE_PROPERTY_GETTER_SETTER(Block, bool, unbreakable);
DEFINE_PROPERTY_GETTER_SETTER(Block, bool, meat_affinity);
DEFINE_PROPERTY_GETTER_SETTER(Block, bool, plant_affinity);
DEFINE_PROPERTY_GETTER_SETTER(Block, bool, shovel_affinity);
DEFINE_PROPERTY_GETTER_SETTER(Block, bool, top_face_random);
DEFINE_PROPERTY_GETTER_SETTER(Block, bool, griefable);
DEFINE_PROPERTY_GETTER_SETTER(Block, bool, no_collision_block);
