#include "../../include/biomes/biome.h"
#include "../../include/world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void Biome::_bind_methods() {
    BIND_METHOD_NO_PARAMETERS(Biome, sync_and_initialize);

    ADD_GROUP("Identity", "");
    BIND_PROPERTY(Biome, String, internal_name);
    BIND_PROPERTY(Biome, int32_t, height);
    BIND_PROPERTY(Biome, double, proportion);
    BIND_PROPERTY(Biome, double, gravity_scale);

    ADD_GROUP("Basic Generation", "");
    BIND_PROPERTY(Biome, bool, is_cliff);
    BIND_PROPERTY(Biome, int, beach_height);
    BIND_PROPERTY(Biome, int, dirt_height);
    BIND_PROPERTY(Biome, double, rare_chunk_chance);
    BIND_PROPERTY(Biome, double, random_decoration_frequency);
    BIND_PROPERTY(Biome, double, decoration_noise_threshold);
    BIND_REF_ARRAY_PROPERTY(Biome, Decoration, random_decorations);
    BIND_REF_ARRAY_PROPERTY(Biome, Decoration, water_random_decorations);
    BIND_REF_ARRAY_PROPERTY(Biome, Block, building_blocks);

    ADD_GROUP("Audio", "");
    BIND_REF_PROPERTY(Biome, AudioStream, ambience);
    BIND_REF_PROPERTY(Biome, AudioStream, music_override);

    ADD_GROUP("Appearance", "");
    BIND_REF_PROPERTY(Biome, Essence, essence);
    BIND_PROPERTY(Biome, Color, sky_tint);
    BIND_PROPERTY(Biome, String, weather);
    BIND_PROPERTY(Biome, double, fog_scale);
    BIND_PROPERTY(Biome, bool, pretty_sky);
    BIND_PROPERTY(Biome, bool, cosmic_sky);
    BIND_PROPERTY(Biome, bool, torus_sky);

    ADD_GROUP("Structures", "st_");
    BIND_REF_ARRAY_PROPERTY_PREFIXED(Biome, Structure, st_, structures);
    BIND_PROPERTY_PREFIXED(Biome, PackedFloat32Array, st_, structure_proportions);

    ADD_GROUP("Entity Spawning", "sp_");
    BIND_PROPERTY_PREFIXED(Biome, float, sp_, day_spawn_rate);
    BIND_PROPERTY_PREFIXED(Biome, PackedFloat32Array, sp_, day_spawn_proportions);
    BIND_REF_ARRAY_PROPERTY_PREFIXED(Biome, PackedScene, sp_, day_spawns);
    BIND_PROPERTY_PREFIXED(Biome, float, sp_, night_spawn_rate);
    BIND_PROPERTY_PREFIXED(Biome, PackedFloat32Array, sp_, night_spawn_proportions);
    BIND_REF_ARRAY_PROPERTY_PREFIXED(Biome, PackedScene, sp_, night_spawns);
}

DEFINE_CONSTRUCTORS(Biome);

void Biome::sync_and_initialize(World* new_world, int new_index) {
    index = new_index;
    world = new_world;
    initialize();
}

void Biome::initialize() {
    terrain_noise = world->generator->noise[0];
    cave_noise = world->generator->noise[10];
    river_noise_1 = world->generator->noise[19];
    river_noise_2 = world->generator->noise[20];
    sand_noise = world->generator->noise[21];
    decoration_noise = world->generator->noise[29];
}

void Biome::generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate) { }

void Biome::generate_decorations(Vector3i chunk_position, Vector3i biome_coordinate) { }

void Biome::simple_decoration_shuffle(Vector3i chunk_position, Vector3i biome_coordinate, bool consider_cave_voids, bool use_decoration_noise) {
    if (random_decorations.size() == 0 && water_random_decorations.size() == 0) {
        return;
    }

    uint16_t rng_seed = world->generator->seed;

    for (int64_t z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
    for (int64_t x = 0; x < Chunk::CHUNK_SIZE_X; x++) {
        Vector3i position = Vector3i(x, 0, z) + chunk_position;

        // Check if decoration should exist here
        Random::scramble_rng_seed(&rng_seed, position);
        if (Random::randf(rng_seed) > random_decoration_frequency) {
            continue;
        }

        if (use_decoration_noise && decoration_noise->get_noise_3d(position.x, position.y, position.z) <= decoration_noise_threshold) {
            continue;
        }

        // Check if this vertical column contains the block above ground-level
        int64_t ground_level = get_ground_level(position, biome_coordinate);
        int64_t y = 1 + ground_level - chunk_position.y;
        if (y < 0 || y > Chunk::CHUNK_SIZE_Y || consider_cave_voids && is_cave_void(position + Vector3i(0, y - 1, 0), ground_level)) {
            continue;
        }

        // Find decoration type
        Random::scramble_rng_seed(&rng_seed, position);
        Ref<DecorationState> d = memnew(DecorationState);

        int64_t water_level = get_water_level(chunk_position + Vector3i(x, 0, z), biome_coordinate);
        if (ground_level <= water_level) {
            if (water_random_decorations.size() == 0) {
                continue;
            }
            d->decoration = water_random_decorations[Random::randi(rng_seed) % water_random_decorations.size()];
        } else {
            if (random_decorations.size() == 0) {
                continue;
            }
            d->decoration = random_decorations[Random::randi(rng_seed) % random_decorations.size()];
        }

        position = chunk_position + Vector3i(x, y, z);

        // Place the decoration
        d->position = position;
        d->direction = Random::randdir(rng_seed);

        world->place_decoration(d);
    }
    }
}

int64_t Biome::get_ground_level(Vector3i position, Vector3i biome_coordinate) {
    int biome_height_offset = world->generator->Y_PER_BIOME_Y * (biome_coordinate.y - 6);

    int river_offset = 0;
    double presence_noise = river_noise_1->get_noise_2d(position.x, position.z);
    if (-0.4 < presence_noise && presence_noise < 0.4) {
        double angle = river_noise_2->get_noise_2d(position.x, position.z) * 2 * Math_PI;
        Vector2 warp = 6.0 * Vector2(UtilityFunctions::cos(angle), UtilityFunctions::sin(angle));

        double warped_noise = river_noise_1->get_noise_2d(position.x + warp.x, position.z + warp.y);
        river_offset = (int) (-22 * (1.0 - UtilityFunctions::smoothstep(0, 0.14, UtilityFunctions::absf(warped_noise))));
    }    

    if (is_cliff) {
        const float e = 2.71828;
        float height = terrain_noise->get_noise_2d(position.x, position.z);
        float v = (1 + UtilityFunctions::pow(e, -60 * (height - 0.32)));
        if (UtilityFunctions::is_zero_approx(v)) {
            v = 0.01;
        }
        return biome_height_offset + river_offset + (int64_t) (32.0 + 90.0 / v + height * 42.0 + 4 * UtilityFunctions::sin(height * 64.0));
    } else {
        float height = terrain_noise->get_noise_2d(position.x, position.z);
        return biome_height_offset + river_offset + 6 + (int64_t) (96.0 * (0.15 + height / 2 + UtilityFunctions::pow(height, 8) + UtilityFunctions::pow(height + 0.5, 4)));
    }
}

int64_t Biome::get_water_level(Vector3i position, Vector3i biome_coordinate) {
    int biome_height_offset = world->generator->Y_PER_BIOME_Y * (biome_coordinate.y - 6);
    int water_offset = WATER_LEVEL;

    if (UtilityFunctions::posmod(biome_coordinate.y, world->generator->BIOME_MAP_SIZE_Y) == 5) {
        water_offset = WATER_LEVEL_2;
    }
    
    return water_offset + biome_height_offset;
}

bool Biome::block_locked(Chunk* chunk, Vector3i local_position) {
    return !chunk->consistent_biome && chunk->get_biome_at(local_position) != index;
}

bool Biome::is_cave_void(Vector3i position, int ground_level) {
    if (position.y > ground_level) {
        return false;
    }

    if (is_cliff) {
        // Apply a slight penalty to dirt/grass level blocks so that cave scarring is less jarring
        bool dirt_penalty = position.y <= ground_level && position.y > ground_level - dirt_height;
        double penalty = dirt_penalty ? 0.2 - 0.2 * (32 + ground_level / 8.0 - position.y) / (double) dirt_height: 0;

        // Boost cave generation closer to the middle of the biome, vertically
        // Edited here: caves up high cause lag :(
        const double spread = 300;
        const double lower_limit = -30;
        double upper_limit = 32 + ground_level / 8.0;
        double center = (upper_limit + lower_limit) / 2;
        double y_boost = UtilityFunctions::exp(-0.5/spread * (position.y - center) * (position.y - center));

        return cave_noise->get_noise_3dv(position + Vector3i(0, position.y, 0)) < -0.8 + 0.30 * y_boost - penalty;
    } else {
        // Apply a slight penalty to dirt/grass level blocks so that cave scarring is less jarring
        bool dirt_penalty = position.y <= ground_level && position.y > ground_level - dirt_height;
        double penalty = dirt_penalty ? 0.2 - 0.2 * (ground_level - position.y) / (double) dirt_height: 0;

        // Boost cave generation closer to the middle of the biome, vertically
        const double spread = 300;
        const double lower_limit = -30;
        double upper_limit = ground_level + 12;
        double center = (upper_limit + lower_limit) / 2;
        double y_boost = UtilityFunctions::exp(-0.5/spread * (position.y - center) * (position.y - center));

        return cave_noise->get_noise_3dv(position + Vector3i(0, position.y, 0)) < -0.67 + 0.30 * y_boost - penalty;
    }
}

bool Biome::is_cave_ore(Vector3i position, int ground_level, uint16_t rng_seed, double ore_chance) {
    int i = Random::randi(rng_seed) % 6;
    Vector3i random_direction;
    if (i == 0) random_direction = Vector3i(+1, 0, 0);
    if (i == 1) random_direction = Vector3i(-1, 0, 0);
    if (i == 2) random_direction = Vector3i(0, +1, 0);
    if (i == 3) random_direction = Vector3i(0, -1, 0);
    if (i == 4) random_direction = Vector3i(0, 0, +1);
    if (i == 5) random_direction = Vector3i(0, 0, -1);
    return Random::randf(rng_seed) < ore_chance && is_cave_void(position + random_direction, ground_level) && is_cave_void(position + Vector3i(0, 1, 0), ground_level);
}

bool Biome::is_cave_dam(Vector3i position, Vector3i biome_coordinate, int ground_level) {
    int water_level = get_water_level(position, biome_coordinate);
    return position.y <= water_level && ground_level - water_level >= DRY_CAVE_HEIGHT && ground_level - water_level <= DRY_CAVE_HEIGHT + DRY_CAVE_OFFSET;
}

bool Biome::is_on_vertical_border(Chunk* chunk, Vector3i position, Vector3i local_position) {
    return biome_border_test(chunk, position - Vector3i(0, 1, 0), local_position - Vector3i(0, 1, 0), true);
}

bool Biome::is_on_horizontal_border(Chunk* chunk, Vector3i position, Vector3i local_position) {
    return biome_border_test(chunk, position + Vector3i(1, 0, 0), local_position + Vector3i(1, 0, 0), false) ||
           biome_border_test(chunk, position - Vector3i(1, 0, 0), local_position - Vector3i(1, 0, 0), false) ||
           biome_border_test(chunk, position + Vector3i(0, 0, 1), local_position + Vector3i(0, 0, 1), false) ||
           biome_border_test(chunk, position - Vector3i(0, 0, 1), local_position - Vector3i(0, 0, 1), false);
}

bool Biome::biome_border_test(Chunk* chunk, Vector3i target_position, Vector3i target_local_position, bool unequal_height) {
    uint8_t biome_index = index;

    if (!chunk->in_bounds(target_local_position)) {
        biome_index = chunk->world->generator->get_biome_index_at(target_position, Generator::LAYER_BASE);
    } else if (block_locked(chunk, target_local_position)) {
        biome_index = chunk->get_biome_at(target_local_position);
    }

    return biome_index != index && (!unequal_height || (int) chunk->biome_height_map[biome_index] != height);
}

bool Biome::even_biome_border_test(Chunk* chunk, Vector3i target_position, Vector3i target_local_position) {
    uint8_t biome_index = index;

    if (!chunk->in_bounds(target_local_position)) {
        biome_index = chunk->world->generator->get_biome_index_at(target_position, Generator::LAYER_BASE);
    } else if (block_locked(chunk, target_local_position)) {
        biome_index = chunk->get_biome_at(target_local_position);
    }

    return biome_index != index && (int) chunk->biome_height_map[biome_index] == height;
}

bool Biome::is_foliage_safe(Chunk* chunk, Vector3i position, Vector3i local_position) {
    Vector3i under_local_position = Vector3i(local_position.x, local_position.y - 1, local_position.z);
    return local_position.y > 0 && !block_locked(chunk, under_local_position) && chunk->get_block_index_at(under_local_position) != 0;
}

bool Biome::is_solid_at(Vector3i position) {
    return false;
}

PackedFloat32Array Biome::get_structure_proportions() const {
    return day_spawn_proportions;
}

void Biome::set_structure_proportions(PackedFloat32Array new_structure_proportions) {
    structure_proportions = new_structure_proportions;
}

DEFINE_PROPERTY_GETTER_SETTER(Biome, String, internal_name);
DEFINE_PROPERTY_GETTER_SETTER(Biome, double, proportion);
DEFINE_PROPERTY_GETTER_SETTER(Biome, int32_t, height);
DEFINE_PROPERTY_GETTER_SETTER(Biome, bool, is_cliff);
DEFINE_PROPERTY_GETTER_SETTER(Biome, Ref<Essence>, essence);
DEFINE_PROPERTY_GETTER_SETTER(Biome, Color, sky_tint);
DEFINE_PROPERTY_GETTER_SETTER(Biome, String, weather);
DEFINE_PROPERTY_GETTER_SETTER(Biome, TypedArray<Block>, building_blocks);
DEFINE_PROPERTY_GETTER_SETTER(Biome, TypedArray<Decoration>, random_decorations);
DEFINE_PROPERTY_GETTER_SETTER(Biome, TypedArray<Decoration>, water_random_decorations);
DEFINE_PROPERTY_GETTER_SETTER(Biome, int, beach_height);
DEFINE_PROPERTY_GETTER_SETTER(Biome, int, dirt_height);
DEFINE_PROPERTY_GETTER_SETTER(Biome, double, rare_chunk_chance);
DEFINE_PROPERTY_GETTER_SETTER(Biome, double, random_decoration_frequency);
DEFINE_PROPERTY_GETTER_SETTER(Biome, Ref<AudioStream>, ambience);
DEFINE_PROPERTY_GETTER_SETTER(Biome, Ref<AudioStream>, music_override);
DEFINE_PROPERTY_GETTER_SETTER(Biome, double, gravity_scale);
DEFINE_PROPERTY_GETTER_SETTER(Biome, double, fog_scale);
DEFINE_PROPERTY_GETTER_SETTER(Biome, bool, pretty_sky);
DEFINE_PROPERTY_GETTER_SETTER(Biome, bool, cosmic_sky);
DEFINE_PROPERTY_GETTER_SETTER(Biome, bool, torus_sky);
DEFINE_PROPERTY_GETTER_SETTER(Biome, double, decoration_noise_threshold);
DEFINE_PROPERTY_GETTER_SETTER(Biome, float, day_spawn_rate);
DEFINE_PROPERTY_GETTER_SETTER(Biome, float, night_spawn_rate);
DEFINE_PROPERTY_GETTER_SETTER(Biome, TypedArray<Structure>, structures);
DEFINE_PROPERTY_GETTER_SETTER(Biome, TypedArray<PackedScene>, day_spawns);
DEFINE_PROPERTY_GETTER_SETTER(Biome, PackedFloat32Array, day_spawn_proportions);
DEFINE_PROPERTY_GETTER_SETTER(Biome, TypedArray<PackedScene>, night_spawns);
DEFINE_PROPERTY_GETTER_SETTER(Biome, PackedFloat32Array, night_spawn_proportions);
