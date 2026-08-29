#include "../../include/biomes/naraka_wasteland_biome.h"
#include "../../include/world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

DEFINE_CONSTRUCTORS_AND_BINDINGS(NarakaWastelandBiome);

void NarakaWastelandBiome::initialize() {
    Biome::initialize();
    cloud_noise = world->generator->noise[30];
}

void NarakaWastelandBiome::generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate) {
    uint16_t rng_seed = world->generator->seed;
    Random::scramble_rng_seed(&rng_seed, chunk_position);

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

                block_type = 0;

                // Prevents grass from spawning with no ground under it
                if (is_foliage_safe(chunk, position, local_position)) {
                    // Pick between common/uncommon foliage
                    if (Random::randf(rng_seed) < 0.015) {
                        if (melon_chunk) {
                            block_type = (int) world->block_name_map["flesh block"];
                        } else {
                            block_type = (int) world->block_name_map["dandelion foliage"];
                        }
                    } else if (Random::randf(rng_seed) < 0.18) {
                        block_type = (int) world->block_name_map["desert grass foliage"];
                    }
                }
            } else if (position.y == ground_level) {
                // Top level of ground
                int randomized_beach_height = beach_height + (int) (6 * sand_noise->get_noise_2d(position.x, position.z));
                block_type = (int) world->block_name_map["sand block"];
            } else if (position.y < ground_level && position.y > ground_level - dirt_height) {
                // Second layer of ground
                // int randomized_beach_height = beach_height + (int) (6 * sand_noise->get_noise_2d(position.x, position.z));
                block_type = (int) world->block_name_map["sand block"];
            } else {
                // Underground
                block_type = (int) world->block_name_map["stone block"];
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

            int biome_height_offset = world->generator->Y_PER_BIOME_Y * (biome_coordinate.y - 6);
            float close_to_border_penalty = UtilityFunctions::clampf((40 + biome_height_offset - position.y) / 32.0f, -10.0f, 1.0f);
            if (position.y <= ground_level - 8 && 0.1 * cloud_noise->get_noise_3d(position.x, position.y, position.z) - close_to_border_penalty < 0) {
                block_type = 0;
            }

            chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = block_type;
            chunk->water[Chunk::position_to_index(Vector3i(x, y, z))] = 0;
        }
    }
    }
}

void NarakaWastelandBiome::generate_decorations(Vector3i chunk_position, Vector3i biome_coordinate) {
    simple_decoration_shuffle(chunk_position, biome_coordinate, true, false);
}

int64_t NarakaWastelandBiome::get_ground_level(Vector3i position, Vector3i biome_coordinate) {
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
        return 50 + biome_height_offset + river_offset + (int64_t) (32.0 + 90.0 / v + height * 42.0 + 4 * UtilityFunctions::sin(height * 64.0));
    } else {
        float height = terrain_noise->get_noise_2d(position.x, position.z);
        return 50 + biome_height_offset + river_offset + (int64_t) (96.0 * (0.15 + height / 2 + UtilityFunctions::pow(height, 8) + UtilityFunctions::pow(height + 0.5, 4)));
    }
}
