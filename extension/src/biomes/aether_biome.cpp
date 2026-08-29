#include "../../include/biomes/aether_biome.h"
#include "../../include/world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void AetherBiome::_bind_methods() {
    BIND_PROPERTY(AetherBiome, bool, imprecise);
}

DEFINE_CONSTRUCTORS(AetherBiome);

DEFINE_PROPERTY_GETTER_SETTER(AetherBiome, bool, imprecise);

void AetherBiome::initialize() {
    Biome::initialize();
    cloud_noise = world->generator->noise[30];
}

void AetherBiome::generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate) {
    if (imprecise) chunk_position += Vector3i(167772120000, 0, 167772120000);

    uint16_t rng_seed = world->generator->seed;
    Random::scramble_rng_seed(&rng_seed, chunk_position);

    bool melon_chunk = Random::randf(rng_seed) < rare_chunk_chance;

    for (int64_t z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
    for (int64_t x = 0; x < Chunk::CHUNK_SIZE_X; x++) {
        // offset to cover more ground, since the bottom is chopped off a little
        int64_t ground_level = 64 + get_ground_level(chunk_position + Vector3i(x, 0, z), biome_coordinate);
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
                        float noise_below = cloud_noise->get_noise_3d(position.x, position.y - 1, position.z);
                        if (noise_below < 0) {
                            block_type = 0;
                        } else if (Random::randf(rng_seed) < 0.015) {
                            if (melon_chunk) {
                                block_type = position.y <= water_level ? 0 : (int) world->block_name_map["melon block"];
                            } else {
                                Random::scramble_rng_seed(&rng_seed, position);
                                block_type = position.y <= water_level ? world->block_name_map["lively seaweed foliage"] : 
                                    (Random::randf(rng_seed) < 0.8 ? (Random::randf(rng_seed) < 0.6 ? world->block_name_map["dandelion foliage"] : world->block_name_map["buzz flower foliage"]) : world->block_name_map["rose foliage"]);
                            }
                        } else if (Random::randf(rng_seed) < 0.18) {
                            block_type = position.y <= water_level ? world->block_name_map["seaweed foliage"] : world->block_name_map["grass foliage"];
                        }
                    }
                }
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

            // Create caves
            if (is_cave_void(position, ground_level)) {
                block_type = 0;

                // Build dams around water and chunk borders
                if (is_cave_dam(position, biome_coordinate, ground_level) || position.y <= water_level && is_on_horizontal_border(chunk, position, local_position)) {
                    block_type = world->block_name_map["sandstone block"];
                }
            } else if (is_cave_ore(position, ground_level, rng_seed, 0.03)) {
                block_type = world->block_name_map["plastril ore block"];
            }


            // Aether eating it up!!
            int biome_height_offset = world->generator->Y_PER_BIOME_Y * (biome_coordinate.y - 6);
            float close_to_border_penalty = UtilityFunctions::clampf((40 + biome_height_offset - position.y) / 32.0f, 0.0f, 1.0f);
            if (position.y <= ground_level + 1 && cloud_noise->get_noise_3d(position.x, position.y, position.z) - close_to_border_penalty < 0) {
                block_type = 0;
            }

            chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = block_type;
            chunk->water[Chunk::position_to_index(Vector3i(x, y, z))] = 0;
        }
    }
    }
}

void AetherBiome::generate_decorations(Vector3i chunk_position, Vector3i biome_coordinate) { }
