#include "../../include/biomes/window_biome.h"
#include "../../include/world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

DEFINE_CONSTRUCTORS_AND_BINDINGS(WindowBiome);

void WindowBiome::initialize() {
    Biome::initialize();
}

void WindowBiome::generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate) {
    uint16_t rng_seed = world->generator->seed;

    Random::scramble_rng_seed(&rng_seed, Vector3i(chunk_position.x, 0, chunk_position.z));

    for (int64_t z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
    for (int64_t x = 0; x < Chunk::CHUNK_SIZE_X; x++) {
        int64_t ground_level = get_ground_level(chunk_position + Vector3i(x, 0, z), biome_coordinate);
        int64_t water_level = get_water_level(chunk_position + Vector3i(x, 0, z), biome_coordinate);

        for (int64_t y = 0; y < Chunk::CHUNK_SIZE_Y; y++) {
            Vector3i position = chunk_position + Vector3i(x, y, z);
            Vector3i local_position = Vector3i(x, y, z);

            Random::scramble_rng_seed(&rng_seed, position);

            if (block_locked(chunk, Vector3i(x, y, z))) {
                continue;
            }

            int32_t block_type = 0;

            if (UtilityFunctions::posmod(position.y, world->generator->Y_PER_BIOME_Y) == 78) {
                block_type = world->block_name_map["glass"];
            } else if (position.y > ground_level + 1) {
                // Air
            } else if (position.y == ground_level + 1) {
                // Foliage

                block_type = 0;

                // Prevents grass from spawning with no ground under it
                if (is_foliage_safe(chunk, position, local_position)) {
                    // Pick between common/uncommon foliage
                    if (Random::randf(rng_seed) < 0.015) {
                        block_type = world->block_name_map["dandelion foliage"];
                    } else if (Random::randf(rng_seed) < 0.18) {
                        block_type = world->block_name_map["blue grass foliage"];
                    }
                }
            } else if (position.y == ground_level) {
                // Top level of ground
                block_type = world->block_name_map["blue grass block"];
            } else if (position.y < ground_level && position.y > ground_level - dirt_height) {
                // Second layer of ground
                block_type = world->block_name_map["dirt block"];
            } else {
                // Underground
                block_type = world->block_name_map["stone block"];
            }

            chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = block_type;
            chunk->water[Chunk::position_to_index(Vector3i(x, y, z))] = position.y <= water_level ? 255 : 0;
        }
    }
    }
}

void WindowBiome::generate_decorations(Vector3i chunk_position, Vector3i biome_coordinate) { }

int64_t WindowBiome::get_ground_level(Vector3i position, Vector3i biome_coordinate) {
    int biome_height_offset = world->generator->Y_PER_BIOME_Y * (biome_coordinate.y - 6);
    float height = terrain_noise->get_noise_2d(position.x, position.z);
    int64_t boost = (int64_t) (24.0 * (0.35 + height));
    return biome_height_offset + boost + 33;
}