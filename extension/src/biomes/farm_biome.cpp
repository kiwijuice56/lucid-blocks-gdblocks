#include "../../include/biomes/farm_biome.h"
#include "../../include/world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void FarmBiome::_bind_methods() {
    BIND_PROPERTY(FarmBiome, int, base_ground_level);
}

DEFINE_CONSTRUCTORS(FarmBiome);

DEFINE_PROPERTY_GETTER_SETTER(FarmBiome, int, base_ground_level);

void FarmBiome::generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate) {
    uint16_t rng_seed = world->generator->seed;
    Random::scramble_rng_seed(&rng_seed, chunk_position);

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

            if (position.y > ground_level + 1) {
                // Air
            } else if (position.y == ground_level + 1) {
                // Foliage

                block_type = 0;

                // Prevents grass from spawning with no ground under it
                if (is_foliage_safe(chunk, position, local_position)) {
                    block_type = world->block_name_map["wheat foliage"];
                }
            } else if (position.y == ground_level) {
                // Top level of ground
                block_type = world->block_name_map["bright grass block"];
            } else if (position.y < ground_level && position.y > ground_level - dirt_height) {
                // Second layer of ground
                block_type = world->block_name_map["dirt block"];
            } else {
                // Underground
                block_type = world->block_name_map["stone block"];
            }

            // Create caves
            if (is_cave_void(position, ground_level)) {
                block_type = 0;

                // Build dams around water and chunk borders
                if (is_cave_dam(position, biome_coordinate, ground_level) || position.y <= water_level && is_on_horizontal_border(chunk, position, local_position)) {
                    block_type = world->block_name_map["stone block"];
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

void FarmBiome::generate_decorations(Vector3i chunk_position, Vector3i biome_coordinate) { }

int64_t FarmBiome::get_ground_level(Vector3i position, Vector3i biome_coordinate) {
    int biome_height_offset = world->generator->Y_PER_BIOME_Y * (biome_coordinate.y - 6);
    return base_ground_level + biome_height_offset + 43 * terrain_noise->get_noise_2d(position.x, position.z);
}