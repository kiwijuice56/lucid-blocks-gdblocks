
#pragma once

#include "item.h"
#include "loot.h"
#include "macros.h"

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/audio_stream.hpp>
#include <godot_cpp/classes/resource_loader.hpp>

namespace godot {

class Block : public Item {
	DECLARE_CLASS(Block, Item);

	enum SoundType {
        SOUND_NONE,
		SOUND_DIRT,
        SOUND_STONE,
        SOUND_WOOD,
        SOUND_GLASS,
        SOUND_METAL,
		SOUND_MEAT,
		SOUND_CARDBOARD,
		SOUND_PLASTIC,
		SOUND_LEAVES,
		SOUND_SNOW,
		SOUND_GRASS,
		SOUND_CLOTH,
    };

	DECLARE_PROPERTY(SoundType, break_sound_type, SOUND_NONE);
	DECLARE_PROPERTY(SoundType, step_sound_type, SOUND_NONE);

    DECLARE_PROPERTY_NO_DEFAULT(Ref<Texture2D>, texture);
	DECLARE_PROPERTY_NO_DEFAULT(Ref<Texture2D>, metallic_texture);
	DECLARE_PROPERTY_NO_DEFAULT(Ref<Texture2D>, roughness_texture);
	DECLARE_PROPERTY_NO_DEFAULT(Ref<AudioStream>, break_sound);
	DECLARE_PROPERTY_NO_DEFAULT(Ref<AudioStream>, step_sound);
	DECLARE_PROPERTY_NO_DEFAULT(Ref<Item>, drop_item);
	DECLARE_PROPERTY_NO_DEFAULT(Ref<Loot>, drop_loot);
	DECLARE_PROPERTY_NO_DEFAULT(Ref<PackedScene>, living_block_scene);

	DECLARE_PROPERTY(String, living_block_path, "");
	DECLARE_PROPERTY(bool, unbreakable, false);
	DECLARE_PROPERTY(bool, textureless, false);
	DECLARE_PROPERTY(bool, override_icon, false);
	DECLARE_PROPERTY(bool, foliage, false);
	DECLARE_PROPERTY(bool, foliage_can_float, false);
	DECLARE_PROPERTY(bool, directional, false);
	DECLARE_PROPERTY(bool, top_face_random, false);
	DECLARE_PROPERTY(Vector3i, direction, Vector3i(0, 1, 0));
	DECLARE_PROPERTY(bool, transparent, false);
	DECLARE_PROPERTY(bool, can_drop, true);
	DECLARE_PROPERTY(float, flammability, 0.5);
	DECLARE_PROPERTY(float, break_time, 1.0);
	DECLARE_PROPERTY(bool, pickaxe_affinity, false);
	DECLARE_PROPERTY(bool, pickaxe_required, false);
	DECLARE_PROPERTY(bool, meat_affinity, false);
	DECLARE_PROPERTY(bool, plant_affinity, false);
	DECLARE_PROPERTY(bool, shovel_affinity, false);
	DECLARE_PROPERTY(bool, griefable, true);
	DECLARE_PROPERTY(bool, no_collision_block, false);
	DECLARE_PROPERTY(bool, axe_affinity, false);
	DECLARE_PROPERTY(bool, axe_required, false);
	DECLARE_PROPERTY(bool, sustain_fire, false);
	
	uint32_t index = 0;

	uint32_t get_index() const;	
};

}

VARIANT_ENUM_CAST(Block::SoundType);
