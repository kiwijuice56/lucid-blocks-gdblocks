#include "../../include/biomes/night_biome.h"
#include "../../include/world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void NightBiome::_bind_methods() {
    BIND_PROPERTY(NightBiome, bool, grid);
}

DEFINE_CONSTRUCTORS(NightBiome);

void NightBiome::initialize() {
    Biome::initialize();
    ceiling_noise = world->generator->noise[17];
}

void NightBiome::generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate) {
    uint16_t rng_seed = world->generator->seed;
    Random::scramble_rng_seed(&rng_seed, chunk_position);

    for (int64_t z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
    for (int64_t x = 0; x < Chunk::CHUNK_SIZE_X; x++) {
        int64_t ground_level = get_ground_level(chunk_position + Vector3i(x, 0, z), biome_coordinate);
        int64_t water_level = get_water_level(chunk_position + Vector3i(x, 0, z), biome_coordinate);
        int64_t ceiling_level = ground_level + 18 + 16 * ceiling_noise->get_noise_2d(chunk_position.x + x, chunk_position.z + z);

        for (int64_t y = 0; y < Chunk::CHUNK_SIZE_Y; y++) {
            Vector3i local_position = Vector3i(x, y, z);
            Vector3i position = chunk_position + local_position;
            Random::scramble_rng_seed(&rng_seed, position);

            if (block_locked(chunk, local_position)) {
                continue;
            }

            int32_t block_type = 0;

            int mx = position.x % 7;
            int my = position.y % 7;
            int mz = position.z % 7;

            if (mx < 0) mx += 7;
            if (my < 0) my += 7;
            if (mz < 0) mz += 7;


            if (position.y > ceiling_level) {
                // Air
            } else if (position.y == ceiling_level) {
                block_type = world->block_name_map["simblock 2"];
            } else if (position.y == ceiling_level - 1) {
                if (Random::randf(rng_seed) < 0.0025) {
                    block_type = world->block_name_map["sodium lamp y-"];
                }
            } else if (position.y > ground_level + 1) {
                // Air
            } else if (position.y == ground_level + 1) {
                // Foliage
                block_type = 0;

                // Prevents grass from spawning with no ground under it
                if (is_foliage_safe(chunk, position, local_position)) {
                    // Pick between common/uncommon foliage
                    if (Random::randf(rng_seed) < 0.0025) {
                        block_type = world->block_name_map["aster foliage"];
                    } else if (Random::randf(rng_seed) < 0.05) {
                        block_type = world->block_name_map["grass foliage"];
                    }
                }
            } else if (position.y == ground_level) {
                // Top level of ground
                if (!grid || mx == 0 || mz == 0) {
                    block_type = world->block_name_map["grass block"];
                }
            } else if (position.y < ground_level && position.y > ground_level - dirt_height) {
                // Second layer of ground
                if (!grid || mx == 0 || mz == 0) {
                    block_type = world->block_name_map["dirt block"];
                }
            } else {
                // Underground
                block_type = world->block_name_map["stone block"];
            }

            // Biome floor
            if (is_on_vertical_border(chunk, position, local_position)) {
                block_type = world->block_name_map["stone block"];
            }

            chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = block_type;
            chunk->water[Chunk::position_to_index(Vector3i(x, y, z))] = (position.y <= water_level && ground_level - water_level < DRY_CAVE_HEIGHT) ? 255 : 0;
        }
    }
    }
}

void NightBiome::generate_decorations(Vector3i chunk_position, Vector3i biome_coordinate) {
    uint16_t rng_seed = world->generator->seed;

    for (int64_t z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
    for (int64_t x = 0; x < Chunk::CHUNK_SIZE_X; x++) {
        int64_t water_level = get_water_level(chunk_position + Vector3i(x, 0, z), biome_coordinate);
        Vector3i position = Vector3i(x, 0, z) + chunk_position;

        int mx = position.x % 7;
        int my = position.y % 7;
        int mz = position.z % 7;

        if (mx < 0) mx += 7;
        if (my < 0) my += 7;
        if (mz < 0) mz += 7;

        if (grid && !(mx == 0 || mz == 0)) {
            continue;
        }

        // Check if decoration should exist here
        Random::scramble_rng_seed(&rng_seed, position);
        if (Random::randf(rng_seed) > random_decoration_frequency) {
            continue;
        }

        // Find decoration type
        Random::scramble_rng_seed(&rng_seed, position);
        uint8_t decoration_index = Random::randi(rng_seed) % random_decorations.size();

        // Check if this vertical column contains the block above ground-level
        int64_t ground_level =get_ground_level(position, biome_coordinate);
        int64_t y = 1 + ground_level - chunk_position.y;
        if (y < 0 || y > Chunk::CHUNK_SIZE_Y) {
            continue;
        }

        // No decorations under water
        if (ground_level <= water_level) {
            continue;
        }

        position = chunk_position + Vector3i(x, y, z);

        // Place the decoration
        Ref<DecorationState> d = memnew(DecorationState);
        d->decoration = random_decorations[decoration_index];
        d->position = position;

        Random::scramble_rng_seed(&rng_seed, position);
        int64_t direction = Random::randi(rng_seed) % 4;

        if (direction == 0) {
            d->direction = DecorationState::North;
        } else if (direction == 1) {
            d->direction = DecorationState::East;
        } else if (direction == 2) {
            d->direction = DecorationState::South;
        } else if (direction == 3) {
            d->direction = DecorationState::West;
        }

        world->place_decoration(d);
    }
    }
}

int64_t NightBiome::get_ground_level(Vector3i position, Vector3i biome_coordinate) {
    int64_t water_level = get_water_level(position, biome_coordinate);
    return water_level;
}

DEFINE_PROPERTY_GETTER_SETTER(NightBiome, bool, grid);