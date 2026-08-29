
#pragma once


#include "macros.h"
#include "biome.h"
#include "../block.h"

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/ref_counted.hpp>

namespace godot {

class FlatsBiome : public Biome {
	DECLARE_CLASS(FlatsBiome, Biome);

	const uint8_t NUM_BUILDING_COLORS = 7;
	String building_colors[7] = {"white plastic block", "red plastic block", "yellow plastic block", "blue plastic block", "pink plastic block", "purple plastic block", "orange plastic block"};

	void generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate);
	int64_t get_ground_level(Vector3i position, Vector3i biome_coordinate);
	bool in_ring(Vector3i position, int ring_size, int grid_size);
	bool is_tile_highway(Vector3i position);
};

}