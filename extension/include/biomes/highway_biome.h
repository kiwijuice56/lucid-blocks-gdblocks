
#pragma once

#include "../macros.h"
#include "biome.h"
#include "../block.h"

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/ref_counted.hpp>

namespace godot {

class HighwayBiome : public Biome {
	DECLARE_CLASS(HighwayBiome, Biome);

	Ref<Noise> road_noise;

	void initialize();
	void generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate);
    void generate_decorations(Vector3i chunk_position, Vector3i biome_coordinate);
	int64_t get_ground_level(Vector3i position, Vector3i biome_coordinate);

	Vector2 road_direction(double x, double z);
	bool is_road(Vector3i position, int ground_level);
};

}