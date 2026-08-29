
#pragma once


#include "macros.h"
#include "biome.h"
#include "../block.h"

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/ref_counted.hpp>

namespace godot {

class CheckerBiome : public Biome {
	DECLARE_CLASS(CheckerBiome, Biome);

	const uint8_t BLOCK_SIZE = 3;

	void generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate);
    void generate_decorations(Vector3i chunk_position, Vector3i biome_coordinate);
	int64_t get_ground_level(Vector3i position, Vector3i biome_coordinate);
};

}