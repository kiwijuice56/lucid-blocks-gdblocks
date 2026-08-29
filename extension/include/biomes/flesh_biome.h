
#pragma once


#include "macros.h"
#include "biome.h"
#include "../block.h"

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/ref_counted.hpp>

namespace godot {

class FleshBiome : public Biome {
	DECLARE_CLASS(FleshBiome, Biome);

	void initialize();
	void generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate);
    void generate_decorations(Vector3i chunk_position, Vector3i biome_coordinate);
	int64_t get_block_at(Chunk* chunk, Vector3i position, int64_t ground_level, Ref<Noise> terrain_noise, uint16_t rng_seed);
	int64_t get_ground_level(Vector3i position, Vector3i biome_coordinate);
};

}