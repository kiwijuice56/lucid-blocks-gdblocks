#include "../../include/biomes/debug_biome.h"
#include "../../include/world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

DEFINE_CONSTRUCTORS_AND_BINDINGS(DebugBiome);

void DebugBiome::generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate) {
    for (int64_t y = 0; y < Chunk::CHUNK_SIZE_Y; y++) {
    for (int64_t z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
    for (int64_t x = 0; x < Chunk::CHUNK_SIZE_X; x++) {
        if (block_locked(chunk, Vector3i(x, y, z))) {
            continue;
        }

        Vector3i position = chunk_position + Vector3i(x, y, z);

        int32_t block_type = 0;
        if (position.x < 0 || position.x >= 32 || position.y < 0 || position.y >= 16 || position.z < 0 || position.z >= 32) {
            block_type = position.y < 16 ? (position.x < 0 || position.z < 0 ? (int) world->block_name_map["yellow plastic block"] : (int) world->block_name_map["blue plastic block"]) : 0;
        } else {
            block_type = 2;
        }

        chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = block_type;
        chunk->water[Chunk::position_to_index(Vector3i(x, y, z))] = 0;
    }
    }
    }
}

int64_t DebugBiome::get_ground_level(Vector3i position, Vector3i biome_coordinate) {
    return 16;
}