
#pragma once


#include "macros.h"
#include "biome.h"
#include "../block.h"

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/ref_counted.hpp>

namespace godot {

class WigglerBiome : public Biome {
	DECLARE_CLASS(WigglerBiome, Biome);

	Ref<Noise> mountain_noise;
	DECLARE_PROPERTY(float, mountain_frequency_multiplier, 0.8);
	DECLARE_PROPERTY(float, mountain_height_multiplier, 1.0);

	void initialize();

	void generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate);
    void generate_decorations(Vector3i chunk_position, Vector3i biome_coordinate);
};

}