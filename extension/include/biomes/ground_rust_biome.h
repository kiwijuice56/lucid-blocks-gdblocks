
#pragma once


#include "macros.h"
#include "biome.h"
#include "../block.h"


#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/ref_counted.hpp>

namespace godot {

class GroundRustBiome : public Biome {
	DECLARE_CLASS(GroundRustBiome, Biome);

	Ref<Noise> rust_noise_1;

	void generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate);
    void generate_decorations(Vector3i chunk_position, Vector3i biome_coordinate);
	void initialize();

	DECLARE_PROPERTY(float, cutoff, 0.0);
	DECLARE_PROPERTY(float, decay, 0.0);
};

}