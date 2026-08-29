#include "../../include/biomes/flood_biome.h"
#include "../../include/world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

DEFINE_CONSTRUCTORS_AND_BINDINGS(FloodBiome);

void FloodBiome::initialize() {
    Biome::initialize();
    terrain_noise = world->generator->noise[8];
}

void FloodBiome::generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate) {
    uint16_t rng_seed = world->generator->seed;

    Random::scramble_rng_seed(&rng_seed, Vector3i(chunk_position.x, 0, chunk_position.z));

    bool is_staircase = Random::randf(rng_seed) < 0.01;

    for (int64_t z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
    for (int64_t x = 0; x < Chunk::CHUNK_SIZE_X; x++) {
        int64_t ground_level = get_ground_level(chunk_position + Vector3i(x, 0, z), biome_coordinate);
        int64_t water_level = get_water_level(chunk_position + Vector3i(x, 0, z), biome_coordinate);

        for (int64_t y = 0; y < Chunk::CHUNK_SIZE_Y; y++) {
            Vector3i position = chunk_position + Vector3i(x, y, z);

            if (block_locked(chunk, Vector3i(x, y, z))) {
                continue;
            }

            int32_t block_type = 0;

            if (is_staircase && position.y > water_level) {
                bool on_edge = false;
                int64_t step = 0;
                if (x == 0) {
                    step = z;
                    on_edge = true;
                }
                if (z == Chunk::CHUNK_SIZE_Z - 1) {
                    step = 15 + x;
                    on_edge = true;
                }
                if (x == Chunk::CHUNK_SIZE_X - 1) {
                    step = 45 - z;
                    on_edge = true;
                }
                if (z == 0) {
                    step = 60 - x;
                    on_edge = true;
                }
                block_type = !on_edge || position.y % 60 == step % 60 ? (int) world->block_name_map["simblock"] : 0;
            } else if (position.y <= ground_level) {
                block_type = world->block_name_map["simblock"];
            } else if (position.y < water_level - 16) {
                block_type = world->block_name_map["simblock"];
            }

            chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = block_type;
            chunk->water[Chunk::position_to_index(Vector3i(x, y, z))] = (y + chunk_position.y) <= water_level ? 255 : 0;
        }
    }
    }
}

void FloodBiome::generate_decorations(Vector3i chunk_position, Vector3i biome_coordinate) { }

int64_t FloodBiome::get_ground_level(Vector3i position, Vector3i biome_coordinate) {
    int64_t water_level = get_water_level(position, biome_coordinate);
    bool on_ground = terrain_noise->get_noise_2d(position.x, position.z) > -0.038;
    return on_ground ? water_level : water_level - 16;
}