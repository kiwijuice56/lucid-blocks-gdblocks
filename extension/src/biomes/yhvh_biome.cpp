#include "../../include/biomes/yhvh_biome.h"
#include "../../include/world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

DEFINE_CONSTRUCTORS_AND_BINDINGS(YhvhBiome);

void YhvhBiome::generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate) {
    uint16_t rng_seed = world->generator->seed;

    for (int64_t z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
    for (int64_t x = 0; x < Chunk::CHUNK_SIZE_X; x++) {
        int64_t ground_level = get_ground_level(chunk_position + Vector3i(x, 0, z), biome_coordinate);

        for (int64_t y = 0; y < Chunk::CHUNK_SIZE_Y; y++) {
            Random::scramble_rng_seed(&rng_seed, chunk_position + Vector3i(x, y, z));

            if (block_locked(chunk, Vector3i(x, y, z))) {
                continue;
            }

            Vector3i position = chunk_position + Vector3i(x, y, z);

            int64_t real_y = y + chunk_position.y;
            int32_t block_type = 0;

            if (real_y <= ground_level) {
                block_type = world->block_name_map["yhvh block"];
            }

            chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = block_type;
            chunk->water[Chunk::position_to_index(Vector3i(x, y, z))] = 0;
        }
    }
    }
}

int64_t YhvhBiome::get_ground_level(Vector3i position, Vector3i biome_coordinate) {
    return 0;
}