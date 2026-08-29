
#pragma once


#include "macros.h"
#include "biome.h"
#include "../block.h"

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/ref_counted.hpp>

namespace godot {

class FlowerBiome : public Biome {
	DECLARE_CLASS(FlowerBiome, Biome);

	DECLARE_PROPERTY(int, base_height, 40);
	DECLARE_PROPERTY(int, grid_size, 6);
	DECLARE_PROPERTY(int, scaffold_levels, 3);
	DECLARE_PROPERTY(float, flower_frequency, 0.6);

	void generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate);
};

}