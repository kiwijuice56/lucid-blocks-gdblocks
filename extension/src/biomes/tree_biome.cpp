#include "../../include/biomes/tree_mirror_biome.h"
#include "../../include/world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void TreeMirrorBiome::_bind_methods() {
    BIND_PROPERTY(TreeMirrorBiome, Vector3i, grid);
}

DEFINE_CONSTRUCTORS(TreeMirrorBiome);

void TreeMirrorBiome::generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate) {
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
                
            } else if (position.y == ground_level + 1) {
                if (water_level < position.y && position.y <= water_level + beach_height + 1) {

                } else {
                    block_type = 0;

                    if (is_foliage_safe(chunk, position, local_position)) {
                        if (Random::randf(rng_seed) < 0.015) {
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
                int randomized_beach_height = beach_height + (int) (6 * sand_noise->get_noise_2d(position.x, position.z));
                block_type = position.y <= water_level + randomized_beach_height ? world->block_name_map["sand block"] : world->block_name_map["grass block"];
            } else if (position.y < ground_level && position.y > ground_level - dirt_height) {
                int randomized_beach_height = beach_height + (int) (6 * sand_noise->get_noise_2d(position.x, position.z));
                block_type = position.y <= water_level + randomized_beach_height ? world->block_name_map["sand block"] : world->block_name_map["dirt block"];
            } else {
                block_type = world->block_name_map["stone block"];
            }

            if (is_cave_void(position, ground_level)) {
                block_type = 0;

                if (is_cave_dam(position, biome_coordinate, ground_level) || position.y <= water_level && is_on_horizontal_border(chunk, position, local_position)) {
                    block_type = world->block_name_map["sandstone block"];
                }
            } else if (is_cave_ore(position, ground_level, rng_seed, 0.03)) {
                block_type = world->block_name_map["plastril ore block"];
            }

            if (is_on_vertical_border(chunk, position, local_position)) {
                block_type = world->block_name_map["stone block"];
            }

            chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = block_type;
            chunk->water[Chunk::position_to_index(Vector3i(x, y, z))] = (position.y <= water_level && ground_level - water_level < DRY_CAVE_HEIGHT) ? 255 : 0;
        }
    }
    }
}

void TreeMirrorBiome::generate_decorations(Vector3i chunk_position, Vector3i biome_coordinate) {
    if (random_decorations.size() == 0 || grid.x == 0 || grid.y == 0 || grid.z == 0) return;

    uint16_t rng_seed = world->generator->seed;

    for (int64_t y = 0; y < Chunk::CHUNK_SIZE_Y; y++) {
    for (int64_t z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
    for (int64_t x = 0; x < Chunk::CHUNK_SIZE_X; x++) {
        Vector3i position = chunk_position + Vector3i(x, y, z);
        if (position.x % grid.x == 0 && position.y % grid.y == 0 && position.z % grid.z == 0) {
            Random::scramble_rng_seed(&rng_seed, chunk_position);
            Ref<Decoration> random_decoration = random_decorations[Random::randi(rng_seed) % random_decorations.size()];
            Ref<DecorationState> d = memnew(DecorationState);
            d->decoration = random_decoration;
            d->position = position;
            d->direction = Random::randdir(rng_seed);
            world->place_decoration(d);
        }
    }
    }
    }

}

DEFINE_PROPERTY_GETTER_SETTER(TreeMirrorBiome, Vector3i, grid);