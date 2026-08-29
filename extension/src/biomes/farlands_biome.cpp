#include "../../include/biomes/farlands_biome.h"
#include "../../include/world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

DEFINE_CONSTRUCTORS_AND_BINDINGS(FarlandsBiome);

void FarlandsBiome::initialize() {
    Biome::initialize();
    terrain_noise = world->generator->noise[0];
}

void FarlandsBiome::generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate) {
    uint16_t rng_seed = world->generator->seed;
    Random::scramble_rng_seed(&rng_seed, chunk_position);

    bool melon_chunk = Random::randf(rng_seed) < rare_chunk_chance;

    for (int64_t z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
    for (int64_t x = 0; x < Chunk::CHUNK_SIZE_X; x++) {
        int64_t ground_level = get_ground_level(Vector3i(1, 2, 1) * chunk_position + Vector3i(x + x, x + z, z), biome_coordinate);
        int64_t water_level = get_water_level(chunk_position + Vector3i(x, 0, z), biome_coordinate);

        for (int64_t y = 0; y < Chunk::CHUNK_SIZE_Y; y++) {
            Random::scramble_rng_seed(&rng_seed, chunk_position + Vector3i(x, y, z));

            if (block_locked(chunk, Vector3i(x, y, z))) {
                continue;
            }

            int64_t real_y = y + chunk_position.y;
            int32_t block_type = 0;

            if (real_y > ground_level + 1) {
                // Air
            } else if (real_y == ground_level + 1) {
                // Foliage

                if (water_level < real_y && real_y <= water_level + beach_height + 1) {
                    // No foliage on beaches
                } else {
                    block_type = 0;

                    // Prevents grass from spawning with no ground under it
                    if (y > 0 && !block_locked(chunk, Vector3i(x, y - 1, z))) {
                        // Pick between common/uncommon foliage
                        if (Random::randf(rng_seed) < 0.015) {
                            if (melon_chunk) {
                                block_type = real_y <= water_level ? 0 : (int) world->block_name_map["melon block"];
                            } else {
                                block_type = real_y <= water_level ? world->block_name_map["lively seaweed foliage"] : world->block_name_map["dandelion foliage"];
                            }
                        } else if (Random::randf(rng_seed) < 0.18) {
                            block_type = real_y <= water_level ? world->block_name_map["seaweed foliage"] : world->block_name_map["grass foliage"];
                        }
                    }
                }
            } else if (real_y == ground_level) {
                // Top level of ground
                block_type = real_y <= water_level + beach_height ? world->block_name_map["sand block"] : world->block_name_map["grass block"];
            } else if (real_y < ground_level && real_y > ground_level - dirt_height) {
                // Second layer of ground
                block_type = world->block_name_map["dirt block"];
            } else {
                // Underground
                block_type = world->block_name_map["stone block"];
            }

            chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = block_type;
            chunk->water[Chunk::position_to_index(Vector3i(x, y, z))] = real_y <= water_level ? 255 : 0;
        }
    }
    }
}

void FarlandsBiome::generate_decorations(Vector3i chunk_position, Vector3i biome_coordinate) {
    simple_decoration_shuffle(chunk_position, biome_coordinate, false, false);
}

int64_t FarlandsBiome::get_ground_level(Vector3i position, Vector3i biome_coordinate) {
    int biome_height_offset = world->generator->Y_PER_BIOME_Y * (biome_coordinate.y - 6);
    if (is_cliff) {
        const float e = 2.71828;
        float height = terrain_noise->get_noise_3d(position.x, position.y, position.z);
        return biome_height_offset + (int64_t) (32.0 + 90.0 / (1 + UtilityFunctions::pow(e, -60 * (height - 0.32))) + height * 42.0 + 4 * UtilityFunctions::sin(height * 64.0));
    } else {
        return biome_height_offset + (int64_t) (96.0 * (0.425 + terrain_noise->get_noise_3d(position.x, position.y, position.z)));
    }
}