#include "../../include/biomes/fudge_biome.h"
#include "../../include/world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void FudgeBiome::_bind_methods() {
    BIND_PROPERTY(FudgeBiome, bool, imprecise);
}

DEFINE_CONSTRUCTORS(FudgeBiome);

DEFINE_PROPERTY_GETTER_SETTER(FudgeBiome, bool, imprecise);

void FudgeBiome::initialize() {
    Biome::initialize();
    terrain_noise = world->generator->noise[0];
}

void FudgeBiome::generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate) {
    uint16_t rng_seed = world->generator->seed;
    Random::scramble_rng_seed(&rng_seed, chunk_position);

    if (imprecise) chunk_position += chunk_position * Vector3i(712, 0, 325);

    const double FOLIAGE_RARE_CHANCE = 0.0015;
    const double FOLIAGE_CHANCE = 0.03;
    const double DOUBLE_FOLIAGE_CHANCE = 0.8;

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

            if (position.y > ground_level + 2) {
                // Air
            } else if (position.y == ground_level + 2) {
                // Double cactus

                block_type = 0;
                if (position.y > water_level + beach_height + 1 && local_position.y >= 1 &&
                    Random::randf(rng_seed) < DOUBLE_FOLIAGE_CHANCE &&
                    is_foliage_safe(chunk, position, local_position) &&
                    chunk->get_block_index_at(local_position - Vector3i(0, 1, 0)) == (int) world->block_name_map["oppleberry block"]) {

                        block_type = world->block_name_map["ambiguous block 6"];
                }
            } else if (position.y == ground_level + 1) {
                // Foliage

                if (water_level < position.y && position.y <= water_level + beach_height + 1) {
                    // No foliage on beaches
                } else {
                    block_type = 0;

                    // Prevents foliage from spawning with no ground under it
                    if (is_foliage_safe(chunk, position, local_position)) {
                        // Pick between common/uncommon foliage
                        if (Random::randf(rng_seed) < FOLIAGE_RARE_CHANCE) {
                            block_type = world->block_name_map["ambiguous block 6"];
                        } else if (Random::randf(rng_seed) < FOLIAGE_CHANCE) {
                            block_type = world->block_name_map["oppleberry block"];
                        }
                    }
                }
            } else if (position.y == ground_level || position.y == ground_level - 1) {
                // Top level of ground
                block_type = world->block_name_map["dirt block"];
            } else if (position.y < ground_level && position.y > ground_level - dirt_height) {
                // Second layer of ground
                block_type = world->block_name_map["pink plastic block"];
            } else {
                // Underground
                block_type = world->block_name_map["white plastic block"];
            }

            if (is_cave_void(position, ground_level)) {
                block_type = 0;

                // Build dams around water and chunk borders
                if (is_cave_dam(position, biome_coordinate, ground_level) || position.y <= water_level && is_on_horizontal_border(chunk, position, local_position)) {
                    block_type = world->block_name_map["pink plastic block"];
                }
            }

            // Biome floor
            if (is_on_vertical_border(chunk, position, local_position)) {
                block_type = world->block_name_map["white plastic block"];
            }

            chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = block_type;
            chunk->water[Chunk::position_to_index(Vector3i(x, y, z))] = (position.y <= water_level && ground_level - water_level < DRY_CAVE_HEIGHT) ? 255 : 0;
        }
    }
    }
}

void FudgeBiome::generate_decorations(Vector3i chunk_position, Vector3i biome_coordinate) {
    if (!imprecise) simple_decoration_shuffle(chunk_position, biome_coordinate, true, false);
}

int64_t FudgeBiome::get_ground_level(Vector3i position, Vector3i biome_coordinate) {
    int river_offset = 0;
    double presence_noise = river_noise_1->get_noise_2d(position.x, position.z);
    int biome_height_offset = world->generator->Y_PER_BIOME_Y * (biome_coordinate.y - 6);
    if (-0.4 < presence_noise && presence_noise < 0.4) {
        double angle = river_noise_2->get_noise_2d(position.x, position.z) * 2 * Math_PI;
        Vector2 warp = 8.0 * Vector2(UtilityFunctions::cos(angle), UtilityFunctions::sin(angle));

        double warped_noise = river_noise_1->get_noise_2d(position.x + warp.x, position.z + warp.y);
        river_offset = (int) (-22 * (1.0 - UtilityFunctions::smoothstep(0, 0.17, UtilityFunctions::absf(warped_noise))));
    }

    if (is_cliff) {
        const float e = 2.71828;
        float height = terrain_noise->get_noise_2d(position.x, position.z);
        float v = (1 + UtilityFunctions::pow(e, -60 * (height - 0.32)));
        if (UtilityFunctions::is_zero_approx(v)) {
            v = 0.01;
        }
        return biome_height_offset + river_offset + (int64_t) (32.0 + 60.0 / v + height * 42.0 + 4 * UtilityFunctions::sin(height * 64.0));
    } else {
        float height = terrain_noise->get_noise_2d(position.x, position.z);
        return biome_height_offset + 8 + river_offset + (int64_t) (48.0 * (0.35 + height / 2 + UtilityFunctions::pow(height, 10) + UtilityFunctions::pow(height + 0.5, 6)));
    }
}