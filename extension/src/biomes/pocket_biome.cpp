#include "../../include/biomes/pocket_biome.h"
#include "../../include/world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

DEFINE_CONSTRUCTORS_AND_BINDINGS(PocketBiome);

void PocketBiome::generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate) {
    for (int64_t y = 0; y < Chunk::CHUNK_SIZE_Y; y++) {
    for (int64_t z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
    for (int64_t x = 0; x < Chunk::CHUNK_SIZE_X; x++) {
        if (block_locked(chunk, Vector3i(x, y, z))) {
            continue;
        }

        Vector3i position = chunk_position + Vector3i(x, y, z);
        int32_t block_type = 0;

        int32_t local_x = ((position.x % 1000) + 1000) % 1000;
        int32_t local_z = ((position.z % 1000) + 1000) % 1000;

        bool in_structure_x = (local_x >= 992 || local_x < 8);
        bool in_structure_z = (local_z >= 992 || local_z < 8);

        if (in_structure_x && in_structure_z) {
            int32_t struct_x = (local_x >= 992) ? (local_x - 1000) : local_x;
            int32_t struct_z = (local_z >= 992) ? (local_z - 1000) : local_z;

            if (struct_x == 0 && position.y == 16 && struct_z == 0) {
                block_type = world->block_name_map["naraka block"];
            } else if (position.y >= 16) {
                block_type = 0;
            } else {
                block_type = world->block_name_map["osmium block"];
            }
        } else {
            block_type = 0;
        }

        chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = block_type;
        chunk->water[Chunk::position_to_index(Vector3i(x, y, z))] = position.y <= 0 ? 255 : 0;
    }
    }
    }
}

int64_t PocketBiome::get_ground_level(Vector3i position, Vector3i biome_coordinate) {
    return 16;
}