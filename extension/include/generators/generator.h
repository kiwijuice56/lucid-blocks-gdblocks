
#pragma once


#include "macros.h"
#include "../biomes/biome.h"
#include "../structures/structure.h"
#include "../random.h"

#include <godot_cpp/classes/noise.hpp>
#include <godot_cpp/classes/fast_noise_lite.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/random_number_generator.hpp>

namespace godot {

class Chunk;
class World;

class Generator : public Resource {
	DECLARE_CLASS(Generator, Resource);

	World* world;

	const int32_t BIOME_MAP_SIZE_X = 32;
	const int32_t BIOME_MAP_SIZE_Z = 32;
	const int32_t BIOME_MAP_SIZE_Y = 30;
	const int32_t Y_PER_BIOME_Y = 256;
	const int32_t Y_BIOME_NOISE = 32;
	const int64_t STRUCTURE_SPAWN_RADIUS_CHUNKS = 3;

	static const int LAYER_BASE = 0;
	static const int LAYER_FUSION = 1;
	static const int LAYER_FUSION_WEIRD = 2;

	int64_t seed;
	// 0th index should be the void biome
	DECLARE_PROPERTY_NO_DEFAULT(TypedArray<Biome>, biomes); // All possible biomes
	DECLARE_PROPERTY_NO_DEFAULT(TypedArray<Noise>, noise); // Noise layers

	TypedArray<Biome> biome_map; // 3D map of possible biomes, indexed using noise
	PackedByteArray biome_index; // Same as above, but index only
	TypedDictionary<int, int> biome_height_map; // Maps biome ID to height

	Ref<Noise> void_noise;
	Ref<Noise> fusion_noise;
	Ref<Noise> noise_x;
    Ref<Noise> noise_z;

	void initialize();

	// Populates water + block arrays of a given chunk
	void generate(World* world, Chunk* chunk, Vector3i chunk_position, int layer);

	// Places decorations (for entire uniform-biome chunks)
	void generate_decorations(World* world, Vector3i chunk_position);

	// Places structures
	void generate_structure(World* world, Vector3i structure_position);

	// Place down blocks of decorations that are in range of this chunk
	void place_decoration_blocks(World* world, Chunk* chunk, Array decorations, Vector3i chunk_position);

	// Replace blocks that require custom logic, like randomly removing leaves on trees
	static int replace_decoration_block(World* world, Chunk* chunk, Ref<Decoration> d, Ref<DecorationState> ds, Vector3i local_position, int block_id);

	// Helper method for place_decoration_blocks
	static void fill_decoration(World* world, Chunk* chunk, Vector3i chunk_position, Ref<Decoration> d, Ref<DecorationState> ds, bool override_air);

	// Transforms a local block position in a decoration to a fully global position, rotated as well
	static Vector3i find_actual_block_position(Vector3i local_position, Vector3i decoration_position, Vector3i decoration_size, Vector3i decoration_center_offset, DecorationState::Direction decoration_direction);

	// Rather than looping through the entire decoration, we loop through the entire chunk -- only faster if the decoration is huge
	static void fill_large_decoration(World* world, Chunk* chunk, Vector3i chunk_position, Ref<Decoration> d, Ref<DecorationState> ds, bool override_air);

	bool is_fusion_block(Vector3i position, int base_block);

	// Places down blocks of closest structure
	void place_structure_blocks(World* world, Chunk* chunk, Vector3i chunk_position);

	Ref<Biome> get_biome_at_real(Vector3i position);
	Ref<Biome> get_biome_at(Vector3i position, int layer);
	uint8_t get_biome_index_at(Vector3i position, int layer);
	uint8_t get_biome_index_at_biome_coordinate(Vector3i biome_coordinate);
	Vector3i get_true_biome_coordinate(Vector3i position);
	Ref<Biome> get_consistent_biome(Vector3i chunk_position, int layer);

	int64_t get_seed() const;
	void set_seed(int64_t new_seed);
};
}