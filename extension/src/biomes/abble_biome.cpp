#include "../../include/biomes/abble_biome.h"
#include "../../include/world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

DEFINE_CONSTRUCTORS_AND_BINDINGS(AbbleBiome);

void AbbleBiome::initialize() {
    Biome::initialize();
    mountain_noise = world->generator->noise[0];
}

void AbbleBiome::generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate) {
    uint16_t rng_seed = world->generator->seed;
    Random::scramble_rng_seed(&rng_seed, chunk_position);

    const double FOLIAGE_RARE_CHANCE = 0.0025;
    const double FOLIAGE_CHANCE = 0.05;

    bool melon_chunk = Random::randf(rng_seed) < rare_chunk_chance;

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
                if (water_level < position.y && position.y <= water_level + beach_height + 1) {
                    // No foliage on beaches
                } else {
                    block_type = 0;

                    // Prevents grass from spawning with no ground under it
                    if (is_foliage_safe(chunk, position, local_position)) {
                        // Pick between common/uncommon foliage
                        if (Random::randf(rng_seed) < FOLIAGE_CHANCE) {
                            block_type = melon_chunk ? world->block_name_map["pulp block"] : world->block_name_map["abble flower foliage"];
                        } else if (Random::randf(rng_seed) < FOLIAGE_CHANCE) {
                            block_type = world->block_name_map["abble foliage"];
                        }
                    }
                }
            } else if (position.y == ground_level || position.y == ground_level - 1) {
                // Top level of ground
                block_type = world->block_name_map["abble block"];
            } else if (position.y < ground_level && position.y > ground_level - dirt_height) {
                // Second layer of ground
                block_type = world->block_name_map["abble block"];
            } else {
                // Underground
                block_type = world->block_name_map["dried abble block"];
            }

            if (is_cave_void(position, ground_level)) {
                block_type = 0;

                // Build dams around water and chunk borders
                if (is_cave_dam(position, biome_coordinate, ground_level) || position.y <= water_level && is_on_horizontal_border(chunk, position, local_position)) {
                    block_type = world->block_name_map["dried abble block"];
                }
            }

            // Biome floor
            if (is_on_vertical_border(chunk, position, local_position)) {
                block_type = world->block_name_map["dried abble block"];
            }

            // Honeycombs
            const double MAIN_THRESHOLD = 0.19;
            const double SECONDARY_THRESHOLD = 0.015;
            const double WARP = 12.0;
            const double Y_DECAY = 0.008;
            const double Y_POWER = 3.5;

            double presence_noise = river_noise_1->get_noise_3d(position.x, position.y, position.z);
            if (UtilityFunctions::absf(presence_noise) < MAIN_THRESHOLD - UtilityFunctions::pow(Y_DECAY * UtilityFunctions::maxi(0, position.y), Y_POWER)) {
                double angle = river_noise_2->get_noise_3d(position.x, position.y, position.z) * 2 * Math_PI;
                Vector3 warp = WARP * Vector3(UtilityFunctions::cos(angle), UtilityFunctions::cos(angle), UtilityFunctions::sin(angle));

                double warped_noise = river_noise_1->get_noise_3d(position.x + warp.x, position.y + warp.y, position.z + warp.z);
                if (UtilityFunctions::absf(warped_noise) < SECONDARY_THRESHOLD) {
                    block_type = world->block_name_map["comb block"];
                }
            }

            chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = block_type;
            chunk->water[Chunk::position_to_index(Vector3i(x, y, z))] = (position.y <= water_level && ground_level - water_level < DRY_CAVE_HEIGHT) ? 255 : 0;
        }
    }
    }
}

void AbbleBiome::generate_decorations(Vector3i chunk_position, Vector3i biome_coordinate) {
    simple_decoration_shuffle(chunk_position, biome_coordinate, true, true);
}

int64_t AbbleBiome::get_ground_level(Vector3i position, Vector3i biome_coordinate) {
    int base = Biome::get_ground_level(position, biome_coordinate);
    double noise = mountain_noise->get_noise_2d(1.6 * position.x, 1.6 * position.z);
    double noise_sq = noise * noise;
    noise += noise_sq;
    return base + (int) (64.0 * (0.3 + noise));
}