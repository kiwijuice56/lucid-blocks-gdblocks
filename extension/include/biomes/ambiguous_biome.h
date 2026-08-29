
#pragma once


#include "macros.h"
#include "biome.h"
#include "../block.h"

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/ref_counted.hpp>

namespace godot {

class AmbiguousBiome : public Biome {
	DECLARE_CLASS(AmbiguousBiome, Biome);

	const uint8_t BLOCK_SIZE = 3;
	const double DECORATION_SPAWN_CHANCE = 0.0005;
	const uint8_t NUM_SURFACE_DECORATIONS = 8;
	String surface_decorations[8] = {"cross 1", "cross 2", "cross 3", "pillar 1", "pillar 2", "cube 1", "cube 2", "turtle"};

	void generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate);
    void generate_decorations(Vector3i chunk_position, Vector3i biome_coordinate);
	int64_t get_ground_level(Vector3i position, Vector3i biome_coordinate);
};

}