
#pragma once

#include "biome.h"
#include "../block.h"
#include "../../include/macros.h"

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/ref_counted.hpp>

namespace godot {

class WaterCubeBiome : public Biome {
	DECLARE_CLASS(WaterCubeBiome, Biome);

	DECLARE_PROPERTY(int, grid_size, 0);
	DECLARE_PROPERTY(int, cube_size, 0);

	void generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate);
};

}