
#pragma once


#include "macros.h"
#include "biome.h"
#include "../block.h"

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/ref_counted.hpp>

namespace godot {

class RandomBiome : public Biome {
	DECLARE_CLASS(RandomBiome, Biome);

	Ref<Noise> random_noise;
	Ref<Noise> cloud_noise;
	uint16_t seed;

	DECLARE_PROPERTY(bool, is_aether, false);

	// forbidden blocks
	int respawn_block_index = 0;
    int heaven_block_index = 0;
	int lavender_block_index = 0;
	int gold_block_index = 0;
	int alma_block_index = 0;

	// default block
	int dirt_block_index = 0;

	struct RandomBiomeIdentity {
		int terrain_type = 0; // terrain types: regular, stepped, flat-like, flat
		int grid_size = 8;
		int offset_size = 4;
		float height_multiplier = 1.0;
		float frequency_multiplier = 1.0;
		int ground_type = 0; // ground types: stacked
		bool corrupt = false;
		bool cliff = false;
		bool checkerboard = false;
		bool grid = false;
		int ground_type_1 = 0;
		int ground_type_2 = 0;
		int ground_type_3 = 0;
		int ground_type_4 = 0;
		int ground_type_5 = 0;
		int ground_type_6 = 0;
		int wacky_block_1 = 0;
		int foliage_1 = 0;
		int foliage_2 = 0;
		int foliage_3 = 0;
		int decoration_index_1 = 0;
		int decoration_index_2 = 0;
		int decoration_index_3 = 0;
		int water_decoration_index_1 = 0;
		int water_decoration_index_2 = 0;
		float decoration_frequency_multiplier = 1.0;
	};

	void initialize();
	
	uint16_t get_biome_seed(Vector3i position, Vector3i biome_coordinate);
	RandomBiomeIdentity randomize(Chunk* chunk, Vector3i position, Vector3i biome_coordinate);
	int get_random_block(Chunk* chunk, uint16_t seed, bool allow_air, bool allow_living, bool allow_transparent, bool allow_foliage);
	int get_specialized_ground_level(RandomBiomeIdentity r, Vector3i column_position, Vector3i biome_coordinate);

	void generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate);
    void generate_decorations(Vector3i chunk_position, Vector3i biome_coordinate);
};

}