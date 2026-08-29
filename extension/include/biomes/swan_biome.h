
#pragma once


#include "macros.h"
#include "biome.h"
#include "../block.h"

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/ref_counted.hpp>

namespace godot {

class SwanBiome : public Biome {
	DECLARE_CLASS(SwanBiome, Biome);

	void generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate);
};

}