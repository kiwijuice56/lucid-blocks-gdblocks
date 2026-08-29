
#pragma once


#include "macros.h"
#include "biome.h"
#include "../block.h"

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/ref_counted.hpp>

namespace godot {

class DungeonBiome : public Biome {
	DECLARE_CLASS(DungeonBiome, Biome);

	Ref<Noise> presence_noise;

	DECLARE_PROPERTY_NO_DEFAULT(TypedArray<Texture2D>, noise_textures);
	DECLARE_PROPERTY_NO_DEFAULT(PackedFloat64Array, thresholds);
	DECLARE_PROPERTY_NO_DEFAULT(PackedFloat64Array, scales);

	PackedFloat64Array data_0;
	PackedFloat64Array data_1;
	PackedFloat64Array data_2;

	DECLARE_PROPERTY(int, level_height, 7);
	DECLARE_PROPERTY(float, foliage_frequency, 0.06);
	DECLARE_PROPERTY(bool, has_water, true);
	DECLARE_PROPERTY(bool, has_exits, false);

	int width_0 = 0;
	int width_1 = 0;
	int width_2 = 0;

	int height_0 = 0;
	int height_1 = 0;
	int height_2 = 0;
	
	void initialize();
	void generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate);
    void generate_decorations(Vector3i chunk_position, Vector3i biome_coordinate);
	int64_t get_ground_level(Vector3i position, Vector3i biome_coordinate);

	int get_block_at(Vector3i position, int block_index_0);
};

}