
#pragma once


#include "macros.h"
#include "biome.h"
#include "../block.h"

#include "decoration.h"

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/ref_counted.hpp>

namespace godot {

class PoolBiome : public Biome {
	DECLARE_CLASS(PoolBiome, Biome);

	DECLARE_PROPERTY_NO_DEFAULT(TypedArray<Decoration>, decorations);

	Ref<Noise> extra_void_noise;

	void generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate);
    void generate_decorations(Vector3i chunk_position, Vector3i biome_coordinate);
	void initialize();
};

}