#include "../../include/biomes/void_biome.h"
#include "../../include/world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

DEFINE_CONSTRUCTORS_AND_BINDINGS(VoidBiome);

void VoidBiome::generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate) {
    for (int64_t z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
    for (int64_t x = 0; x < Chunk::CHUNK_SIZE_X; x++) {
        int64_t ground_level = get_ground_level(chunk_position + Vector3i(x, 0, z), biome_coordinate);
        int64_t water_level = get_water_level(chunk_position + Vector3i(x, 0, z), biome_coordinate);

        for (int64_t y = 0; y < Chunk::CHUNK_SIZE_Y; y++) {
            Vector3i local_position = Vector3i(x, y, z);
            Vector3i position = chunk_position + local_position;

            if (block_locked(chunk, local_position)) {
                continue;
            }

            int32_t block_type = 0;

            if (position.y == -49 || position.y <= water_level && is_on_horizontal_border(chunk, position, local_position)) {
                block_type = world->block_name_map["simblock"];
            }

            chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = block_type;
            chunk->water[Chunk::position_to_index(Vector3i(x, y, z))] = 0;
        }
    }
    }
}

void VoidBiome::generate_decorations(Vector3i chunk_position, Vector3i biome_coordinate) { }

int64_t VoidBiome::get_ground_level(Vector3i position, Vector3i biome_coordinate) {
    return GROUND_LEVEL_INVALID;
}