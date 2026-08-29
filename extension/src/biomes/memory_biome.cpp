#include "../../include/biomes/memory_biome.h"
#include "../../include/world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

DEFINE_CONSTRUCTORS_AND_BINDINGS(MemoryBiome);

void MemoryBiome::generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate)  {
    uint16_t rng_seed = world->generator->seed;
    uint16_t base_seed = rng_seed;

    for (int64_t y = 0; y < Chunk::CHUNK_SIZE_Y; y++) {
    for (int64_t z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
    for (int64_t x = 0; x < Chunk::CHUNK_SIZE_X; x++) {
        Vector3i position = chunk_position + Vector3i(x, y, z);
        
        if (block_locked(chunk, Vector3i(x, y, z))) {
            continue;
        }

        int64_t block_type = 0;
        if (position.x % 4 == 0 && position.y % 4 == 0 && position.z % 4 == 0) {
            block_type = world->block_name_map["silver block"];
        }

        chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = block_type;
        chunk->water[Chunk::position_to_index(Vector3i(x, y, z))] = 0;
    }
    }
    }
}
