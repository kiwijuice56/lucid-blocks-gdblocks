
#pragma once

#include "biome.h"
#include "../block.h"
#include "../macros.h"


#include "decoration.h"

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/ref_counted.hpp>

namespace godot {

class WarpedBiome : public Biome {
	DECLARE_CLASS(WarpedBiome, Biome);

	DECLARE_PROPERTY(Vector3i, size, Vector3i(0, 0, 0));
	DECLARE_PROPERTY(double, frequency, 0.01);
	DECLARE_PROPERTY(double, scale, 2.0);
	DECLARE_PROPERTY(double, randomness, 0.1);

	void generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate);
    void generate_decorations(Vector3i chunk_position, Vector3i biome_coordinate);
	void initialize();
};

}