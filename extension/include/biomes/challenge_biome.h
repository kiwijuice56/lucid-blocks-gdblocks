
#pragma once


#include "macros.h"
#include "biome.h"
#include "../block.h"

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/ref_counted.hpp>

namespace godot {

class ChallengeBiome : public Biome {
	DECLARE_CLASS(ChallengeBiome, Biome);

	Ref<DecorationState> placed_room;
	DECLARE_PROPERTY_NO_DEFAULT(Ref<Decoration>, room);

	void initialize();
	void generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate);
};

}