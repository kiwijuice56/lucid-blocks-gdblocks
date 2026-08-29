
#pragma once

#include "../macros.h"
#include "../decoration_state.h"
#include "../chunk.h"
#include "../structures/structure.h"
#include "../random.h"

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/noise.hpp>
#include <godot_cpp/classes/fast_noise_lite.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/audio_stream.hpp>

namespace godot {

class World;

class Biome : public Resource {
	DECLARE_CLASS(Biome, Resource);

    static const int64_t WATER_LEVEL = 25; // water level at the overworld
    static const int64_t WATER_LEVEL_2 = 32; // water level at the underworld
    static const int64_t GROUND_LEVEL_INVALID = -1000000;

	const int64_t DRY_CAVE_HEIGHT = 10; // Height above water-level at which caves can spawn with no water
	const int64_t DRY_CAVE_OFFSET = 2;

    // (both set by generator)
    World* world;
    uint8_t index = 0; // Essentially an ID

    DECLARE_PROPERTY(String, internal_name, "");
    DECLARE_PROPERTY(double, proportion, 0);
    DECLARE_PROPERTY(int32_t, height, 0);
    DECLARE_PROPERTY(bool, is_cliff, false);
    DECLARE_PROPERTY_NO_DEFAULT(Ref<Essence>, essence);
    DECLARE_PROPERTY(Color, sky_tint, Color(0, 0, 0, 1));
    DECLARE_PROPERTY(String, weather, "");
    DECLARE_PROPERTY_NO_DEFAULT(TypedArray<Block>, building_blocks);
    DECLARE_PROPERTY_NO_DEFAULT(TypedArray<Decoration>, random_decorations);
    DECLARE_PROPERTY_NO_DEFAULT(TypedArray<Decoration>, water_random_decorations);
    DECLARE_PROPERTY(double, random_decoration_frequency, 0.005);
    DECLARE_PROPERTY(int, beach_height, 4);
    DECLARE_PROPERTY(int, dirt_height, 3);
    DECLARE_PROPERTY(double, decoration_noise_threshold, -0.1);
    DECLARE_PROPERTY(double, rare_chunk_chance, 0.085);
    DECLARE_PROPERTY(double, gravity_scale, 1.0);
    DECLARE_PROPERTY(double, fog_scale, 1.0);
    DECLARE_PROPERTY(bool, pretty_sky, false);
    DECLARE_PROPERTY(bool, cosmic_sky, false);
    DECLARE_PROPERTY(bool, torus_sky, false);

    DECLARE_PROPERTY_NO_DEFAULT(Ref<AudioStream>, ambience);
    DECLARE_PROPERTY_NO_DEFAULT(Ref<AudioStream>, music_override);

    DECLARE_PROPERTY(float, day_spawn_rate, 1.);
    DECLARE_PROPERTY(float, night_spawn_rate, 1.);
    DECLARE_PROPERTY_NO_DEFAULT(TypedArray<Structure>, structures);
    DECLARE_PROPERTY_NO_DEFAULT(PackedFloat32Array, structure_proportions);
    DECLARE_PROPERTY_NO_DEFAULT(TypedArray<PackedScene>, day_spawns);
    DECLARE_PROPERTY_NO_DEFAULT(PackedFloat32Array, day_spawn_proportions);
    DECLARE_PROPERTY_NO_DEFAULT(TypedArray<PackedScene>, night_spawns);
    DECLARE_PROPERTY_NO_DEFAULT(PackedFloat32Array, night_spawn_proportions);

    Ref<Noise> terrain_noise;
    Ref<Noise> cave_noise;

    Ref<Noise> river_noise_1;
    Ref<Noise> river_noise_2;

    Ref<Noise> sand_noise;

    Ref<Noise> decoration_noise;

    void sync_and_initialize(World* world, int index);

    virtual void initialize();
    virtual void generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate);
    virtual void generate_decorations(Vector3i chunk_position, Vector3i biome_coordinate);

    // Helper function to generate decorations by simply randomly selecting and placing them from an array of names
    void simple_decoration_shuffle(Vector3i chunk_position, Vector3i biome_coordinate, bool consider_cave_voids, bool use_decoration_noise);

    // These 3 functions are used for biome + structure generation
    virtual int64_t get_ground_level(Vector3i position, Vector3i biome_coordinate);
    virtual int64_t get_water_level(Vector3i position, Vector3i biome_coordinate);
    virtual bool is_solid_at(Vector3i position);

    // Returns whether this position is air within a cave
    virtual bool is_cave_void(Vector3i position, int ground_level);

    virtual bool is_cave_ore(Vector3i position, int ground_level, uint16_t rng_seed, double ore_chance);

    // Returns whether this position is within a cave, but solid to block off water
    virtual bool is_cave_dam(Vector3i position, Vector3i biome_coordinate, int ground_level);

    // Returns whether this position is above a biome border
    bool is_on_vertical_border(Chunk* chunk, Vector3i position, Vector3i local_position);

    // Returns whether this position is next to a biome border
    bool is_on_horizontal_border(Chunk* chunk, Vector3i position, Vector3i local_position);

    // Helper function
    bool biome_border_test(Chunk* chunk, Vector3i target_position, Vector3i target_local_position, bool unequal_height);

    // Same as regular test, but requires heights to be equal
    bool even_biome_border_test(Chunk* chunk, Vector3i target_position, Vector3i target_local_position);

    // Returns whether this position is OK to place foliage on
    bool is_foliage_safe(Chunk* chunk, Vector3i position, Vector3i local_position);

    // Returns whether this position is being filled by another biome
    bool block_locked(Chunk* chunk, Vector3i local_position);
};

}