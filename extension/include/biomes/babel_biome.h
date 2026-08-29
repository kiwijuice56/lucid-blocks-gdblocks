
#pragma once


#include "macros.h"
#include "biome.h"
#include "../block.h"

#include "decoration.h"

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/ref_counted.hpp>

namespace godot {

class BabelBiome : public Biome {
	DECLARE_CLASS(BabelBiome, Biome);

	void generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate);
};

}