
#pragma once

#include "biome.h"
#include "../block.h"
#include "../macros.h"

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/ref_counted.hpp>

namespace godot {

class HellBiome : public Biome {
	DECLARE_CLASS(HellBiome, Biome);

	Ref<Noise> hell_noise;

	void initialize();
	void generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate);
    void generate_decorations(Vector3i chunk_position, Vector3i biome_coordinate);
};

}