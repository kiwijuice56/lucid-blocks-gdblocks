
#pragma once


#include "macros.h"
#include "biome.h"
#include "../block.h"

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/ref_counted.hpp>

namespace godot {

class JuliaBiome : public Biome {
	DECLARE_CLASS(JuliaBiome, Biome);

	Ref<Noise> julia_noise;

	void generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate);
    void generate_decorations(Vector3i chunk_position, Vector3i biome_coordinate);
	void initialize();

	double noise3D(const Vector3 &p);
	double gyroid(const Vector3 &p);
	double gyroid_density(const Vector3i &block_pos);
};

}