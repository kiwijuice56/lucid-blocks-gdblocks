#include "../../include/biomes/checker_biome.h"
#include "../../include/world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

DEFINE_CONSTRUCTORS_AND_BINDINGS(CheckerBiome);

void CheckerBiome::generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate) {
    uint16_t rng_seed = world->generator->seed;

    int floor_type_1 =  Ref<Block>(building_blocks[0])->index;
    int floor_type_2 =  Ref<Block>(building_blocks[1])->index;
    for (int64_t z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
    for (int64_t x = 0; x < Chunk::CHUNK_SIZE_X; x++) {
        int64_t snapped_x = BLOCK_SIZE * ((x + chunk_position.x) / BLOCK_SIZE);
        int64_t snapped_z = BLOCK_SIZE * ((z + chunk_position.z) / BLOCK_SIZE);
        int64_t ground_level = get_ground_level(chunk_position + Vector3i(x, 0, z), biome_coordinate);
        int64_t water_level = get_water_level(chunk_position + Vector3i(x, 0, z), biome_coordinate);

        for (int64_t y = 0; y < Chunk::CHUNK_SIZE_Y; y++) {
            Random::scramble_rng_seed(&rng_seed, chunk_position + Vector3i(x, y, z));

            if (block_locked(chunk, Vector3i(x, y, z))) {
                continue;
            }

            Vector3i position = chunk_position + Vector3i(x, y, z);

            int64_t real_y = y + chunk_position.y;
            int64_t snapped_y = BLOCK_SIZE * (real_y / BLOCK_SIZE);
            int32_t block_type = 0;

            if (real_y <= ground_level) {
                int64_t checker = ((snapped_x + snapped_z) / BLOCK_SIZE) % 2;
                if (checker < 0) {
                    checker = -checker;
                }
                block_type = checker == 0 ? floor_type_1 : floor_type_2;
            }

            chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = block_type;
            chunk->water[Chunk::position_to_index(Vector3i(x, y, z))] = real_y <= water_level ? 255 : 0;
        }
    }
    }
}

void CheckerBiome::generate_decorations(Vector3i chunk_position, Vector3i biome_coordinate) {
    simple_decoration_shuffle(chunk_position, biome_coordinate, false, false);
}

int64_t CheckerBiome::get_ground_level(Vector3i position, Vector3i biome_coordinate) {
    int64_t water_level = get_water_level(position, biome_coordinate);
    return water_level + 1;
}