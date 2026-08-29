
#pragma once


#include "macros.h"
#include "biome.h"
#include "../block.h"

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/ref_counted.hpp>

namespace godot {

class PrisonBiome : public Biome {
	DECLARE_CLASS(PrisonBiome, Biome);

	const int64_t CELL_SIZE = 9;
	const int64_t CELL_HEIGHT = 6;

	void generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate);
    void generate_decorations(Vector3i chunk_position, Vector3i biome_coordinate);
};

}