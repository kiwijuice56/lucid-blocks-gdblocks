#include "../../include/biomes/lotus_biome.h"
#include "../../include/world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

DEFINE_CONSTRUCTORS_AND_BINDINGS(LotusBiome);

void LotusBiome::initialize() {
    Biome::initialize();
    terrain_noise = world->generator->noise[18];
    primary_block = Ref<Block>(building_blocks[0])->index;
}

void LotusBiome::generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate)  {
    uint16_t rng_seed = world->generator->seed;


    for (int64_t z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
    for (int64_t x = 0; x < Chunk::CHUNK_SIZE_X; x++) {
        int ground_level = get_ground_level(chunk_position + Vector3i(x, 0, z), biome_coordinate);
        int water_level = get_water_level(chunk_position + Vector3i(x, 0, z), biome_coordinate);

        for (int64_t y = 0; y < Chunk::CHUNK_SIZE_Y; y++) {
            Vector3i local_position = Vector3i(x, y, z);
            Vector3i position = chunk_position + local_position;
            Random::scramble_rng_seed(&rng_seed, position);
            if (block_locked(chunk, local_position)) {
                continue;
            }

            int block_type = 1;
            if (position.y <= water_level &&
                (
                    even_biome_border_test(chunk, position + Vector3i(1, 0, 0), local_position + Vector3i(1, 0, 0)) ||
                    even_biome_border_test(chunk, position - Vector3i(1, 0, 0), local_position - Vector3i(1, 0, 0)) ||
                    even_biome_border_test(chunk, position + Vector3i(0, 0, 1), local_position + Vector3i(0, 0, 1)) ||
                    even_biome_border_test(chunk, position - Vector3i(0, 0, 1), local_position - Vector3i(0, 0, 1))
                ) || position.y <= ground_level) {
                block_type = primary_block;
            }

            chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = block_type;
            chunk->water[Chunk::position_to_index(Vector3i(x, y, z))] = 0;
        }
    }
    }
}

void LotusBiome::generate_decorations(Vector3i chunk_position, Vector3i biome_coordinate) { }

int64_t LotusBiome::get_ground_level(Vector3i position, Vector3i biome_coordinate) {
    int64_t water_level = get_water_level(position, biome_coordinate);
    return terrain_noise->get_noise_2d(position.x, position.z) > 0.38 ? water_level + 2 : GROUND_LEVEL_INVALID;
}