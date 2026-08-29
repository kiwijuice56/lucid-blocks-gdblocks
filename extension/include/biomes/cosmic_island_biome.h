
#pragma once


#include "macros.h"
#include "biome.h"
#include "../block.h"

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/ref_counted.hpp>

namespace godot {

class CosmicIslandBiome : public Biome {
	DECLARE_CLASS(CosmicIslandBiome, Biome);

	DECLARE_PROPERTY(int, grid_size, 24);
	DECLARE_PROPERTY(int, cube_size, 6);

	void generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate);
};

}