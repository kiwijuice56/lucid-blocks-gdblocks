
#pragma once


#include "macros.h"
#include "biome.h"
#include "../block.h"

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/ref_counted.hpp>

namespace godot {

class FractalBiome : public Biome {
	DECLARE_CLASS(FractalBiome, Biome);

	Ref<Noise> mud_noise;
	Ref<Noise> tendril_noise;

	void initialize();
	void generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate);
    void generate_decorations(Vector3i chunk_position, Vector3i biome_coordinate);
	int32_t get_block_at(Vector3i position);
	int64_t get_ground_level(Vector3i position, Vector3i biome_coordinate);
	bool is_solid_at(Vector3i position);
};

}