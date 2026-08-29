#include "../../include/biomes/swan_biome.h"
#include "../../include/world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

DEFINE_CONSTRUCTORS_AND_BINDINGS(SwanBiome);

void SwanBiome::generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate)  {
    uint16_t rng_seed = world->generator->seed;

    Random::scramble_rng_seed(&rng_seed, Vector3i(chunk_position.x, 0, chunk_position.z));
    bool solid_chunk = Random::randf(rng_seed) < 0.02;

    for (int64_t z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
    for (int64_t x = 0; x < Chunk::CHUNK_SIZE_X; x++) {
        Random::scramble_rng_seed(&rng_seed, Vector3i(chunk_position.x, 0, chunk_position.z) + Vector3i(x, 0, z));
       
        Ref<Block> block = building_blocks[Random::randi(rng_seed) % building_blocks.size()];
        int block_index = block->index;

        int biome_height_offset = world->generator->Y_PER_BIOME_Y * (biome_coordinate.y - 6);
        int ground_level = biome_height_offset;
        int offset = Random::randi(rng_seed) % 4 - 8;

        for (int64_t y = 0; y < Chunk::CHUNK_SIZE_X; y++) {
            Vector3i position = chunk_position + Vector3i(x, y, z);
            if (block_locked(chunk, Vector3i(x, y, z))) {
                continue;
            }

            
            int block_type = 0;
            if (solid_chunk || position.y < ground_level + offset) {
                block_type = block_index;
            }

            chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = block_type;
            chunk->water[Chunk::position_to_index(Vector3i(x, y, z))] = 0;
        }
        }
    }
}
