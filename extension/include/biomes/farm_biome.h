
#pragma once

#include "../macros.h"
#include "biome.h"
#include "../block.h"

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/ref_counted.hpp>

namespace godot {

class FarmBiome : public Biome {
	DECLARE_CLASS(FarmBiome, Biome);

	DECLARE_PROPERTY(int, base_ground_level, 0);

	void generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate);
    void generate_decorations(Vector3i chunk_position, Vector3i biome_coordinate);
	int64_t get_ground_level(Vector3i position, Vector3i biome_coordinate);
};

}