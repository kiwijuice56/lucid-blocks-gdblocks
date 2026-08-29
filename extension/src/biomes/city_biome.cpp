#include "../../include/biomes/city_biome.h"
#include "../../include/world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

DEFINE_CONSTRUCTORS_AND_BINDINGS(CityBiome);

void CityBiome::generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate) {
    uint16_t rng_seed = world->generator->seed;

    for (int64_t z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
    for (int64_t x = 0; x < Chunk::CHUNK_SIZE_X; x++) {
        int64_t ground_level = get_ground_level(chunk_position + Vector3i(x, 0, z), biome_coordinate);
        int64_t water_level = get_water_level(chunk_position + Vector3i(x, 0, z), biome_coordinate);

        for (int64_t y = 0; y < Chunk::CHUNK_SIZE_Y; y++) {
            Vector3i local_position = Vector3i(x, y, z);
            Vector3i position = chunk_position + local_position;
            Random::scramble_rng_seed(&rng_seed, position);

            if (block_locked(chunk, local_position)) {
                continue;
            }
            
            int32_t block_type = 0;

            if (position.y == ground_level + 1) {
                block_type = 0;

                // Prevents grass from spawning with no ground under it
                if (is_foliage_safe(chunk, position, local_position)) {
                    // Pick between common/uncommon foliage
                    if (Random::randf(rng_seed) < 0.015) {
                       block_type = position.y <= water_level ? world->block_name_map["lively seaweed foliage"] : world->block_name_map["dandelion foliage"];
                    } else if (Random::randf(rng_seed) < 0.18) {
                        block_type = position.y <= water_level ? world->block_name_map["seaweed foliage"] : world->block_name_map["grass foliage"];
                    }
                }
            } else if (position.y > ground_level) {
                block_type = 0;
            } else if (position.y == ground_level) {
                // Top level of ground
                int randomized_beach_height = beach_height + (int) (6 * sand_noise->get_noise_2d(position.x, position.z));
                block_type = position.y <= water_level + randomized_beach_height ? world->block_name_map["sand block"] : world->block_name_map["grass block"];
            } else if (position.y < ground_level && position.y > ground_level - dirt_height) {
                // Second layer of ground
                int randomized_beach_height = beach_height + (int) (6 * sand_noise->get_noise_2d(position.x, position.z));
                block_type = position.y <= water_level + randomized_beach_height ? world->block_name_map["sand block"] : world->block_name_map["dirt block"];
            } else {
                // Underground
                block_type = world->block_name_map["stone block"];
            }

            if (is_cave_void(position, ground_level)) {
                block_type = 0;

                // Build dams around water and chunk borders
                if (is_cave_dam(position, biome_coordinate, ground_level) || position.y <= water_level && is_on_horizontal_border(chunk, position, local_position)) {
                    block_type = world->block_name_map["sandstone block"];
                }
            } else if (is_cave_ore(position, ground_level, rng_seed, 0.03)) {
                block_type = world->block_name_map["plastril ore block"];
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

void CityBiome::generate_decorations(Vector3i chunk_position, Vector3i biome_coordinate) {
    if (random_decorations.size() == 0) {
        return;
    }

    uint16_t rng_seed = world->generator->seed;

    for (int64_t z = 0; z < Chunk::CHUNK_SIZE_Z; z += 2) {
    for (int64_t x = 0; x < Chunk::CHUNK_SIZE_X; x += 2) {
        Vector3i position = Vector3i(x, 0, z) + chunk_position;

        // Check if this spot has no pillar first
        Random::scramble_rng_seed(&rng_seed, position);
        if (Random::randf(rng_seed) > random_decoration_frequency) {
            continue;
        }

        // Check if this vertical column contains the ground-level
        int64_t ground_level = get_ground_level(position, biome_coordinate);
        int64_t y = ground_level - chunk_position.y;
        if (y < 0 || y > Chunk::CHUNK_SIZE_Y || is_cave_void(position + Vector3i(0, y - 1, 0), ground_level)) {
            continue;
        }

        // Place the pillar
        Ref<DecorationState> d = memnew(DecorationState);
        d->decoration = random_decorations[Random::randi(rng_seed) % random_decorations.size()];
        d->position = chunk_position + Vector3i(x, y, z);

        world->place_decoration(d);
    }
    }
}
