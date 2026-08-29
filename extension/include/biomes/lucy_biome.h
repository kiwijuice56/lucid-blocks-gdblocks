
#pragma once


#include "macros.h"
#include "biome.h"
#include "../block.h"

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/ref_counted.hpp>

namespace godot {

class LucyBiome : public Biome {
	DECLARE_CLASS(LucyBiome, Biome);

	Ref<Noise> pipe_noise;

	void generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate);
    void generate_decorations(Vector3i chunk_position, Vector3i biome_coordinate);
	void initialize();
};

}