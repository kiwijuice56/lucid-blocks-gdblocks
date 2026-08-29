
#pragma once


#include "macros.h"
#include "biome.h"
#include "../block.h"

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/ref_counted.hpp>

namespace godot {

class TreeMirrorBiome : public Biome {
	DECLARE_CLASS(TreeMirrorBiome, Biome);

	DECLARE_PROPERTY(Vector3i, grid, Vector3i(0, 0, 0));

	void generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate);
    void generate_decorations(Vector3i chunk_position, Vector3i biome_coordinate);
};

}