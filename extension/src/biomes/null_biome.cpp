#include "../../include/biomes/null_biome.h"
#include "../../include/world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

DEFINE_CONSTRUCTORS_AND_BINDINGS(NullBiome);

void NullBiome::generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate) {
    for (int64_t z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
    for (int64_t x = 0; x < Chunk::CHUNK_SIZE_X; x++) {
    for (int64_t y = 0; y < Chunk::CHUNK_SIZE_Y; y++) {
        if (block_locked(chunk, Vector3i(x, y, z))) {
            continue;
        }

        chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = 0;
        chunk->water[Chunk::position_to_index(Vector3i(x, y, z))] = 0;
    }
    }
    }
}
