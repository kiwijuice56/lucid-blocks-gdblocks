#include "../../include/biomes/clown_biome.h"
#include "../../include/world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

DEFINE_CONSTRUCTORS_AND_BINDINGS(ClownBiome);

void ClownBiome::initialize() {
    Biome::initialize();
}

void ClownBiome::generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate)  {
    const int ROOM_SIZE = 13;

    uint16_t rng_seed = world->generator->seed;

    for (int64_t y = 0; y < Chunk::CHUNK_SIZE_Y; y++) {
    for (int64_t z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
    for (int64_t x = 0; x < Chunk::CHUNK_SIZE_X; x++) {
        int64_t water_level = get_water_level(chunk_position + Vector3i(x, 0, z), biome_coordinate);
        Vector3i position = chunk_position + Vector3i(x, y, z);

        Random::scramble_rng_seed(&rng_seed, position);

        if (block_locked(chunk, Vector3i(x, y, z))) {
            continue;
        }

        bool x_aligned = position.x % ROOM_SIZE == 0;
        bool y_aligned = position.y % ROOM_SIZE == 0;
        bool z_aligned = position.z % ROOM_SIZE == 0;

        int x_grid = position.x % ROOM_SIZE;
        int y_grid = position.y % ROOM_SIZE;
        int z_grid = position.z % ROOM_SIZE;

        Vector3i snapped_position = position / ROOM_SIZE;

        rng_seed = world->generator->seed;
        Random::scramble_rng_seed(&rng_seed, snapped_position);

        bool white_room = false; // Removed because it looked ugly

        if (x_grid < 0) x_grid += ROOM_SIZE;
        if (y_grid < 0) y_grid += ROOM_SIZE;
        if (z_grid < 0) z_grid += ROOM_SIZE;

        int actual_color = (y_grid - 1) / 3;
        if (actual_color < 0) {
            actual_color = 0;
        }

        int32_t block_type = 0;
        if (white_room && (x_aligned || y_aligned || z_aligned)) {
            block_type = world->block_name_map["black plastic block"];
        } else if (y_grid >= 1 && y_grid <= 4 && (z_grid >= 5 && z_grid <= 8 || x_grid >= 5 && x_grid <= 8) ) {
            block_type = 0;
        } else if (!x_aligned && !y_aligned && !z_aligned) {
            block_type = 0;
        } else {
            if (actual_color == 0) {
                block_type = world->block_name_map["red plastic block"];
            } else if (actual_color == 1) {
                block_type = world->block_name_map["pink plastic block"];
            } else if (actual_color == 2) {
                block_type = world->block_name_map["yellow plastic block"];
            } else if (actual_color == 3) {
                block_type = world->block_name_map["green plastic block"];
            }
        }

        if (is_on_vertical_border(chunk, chunk_position + Vector3i(x, y, z), Vector3i(x, y, z))) {
            block_type = world->block_name_map["white plastic block"];
        }

        chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = block_type;
        chunk->water[Chunk::position_to_index(Vector3i(x, y, z))] = position.y <= water_level ? 255 : 0;
    }
    }
    }
}

void ClownBiome::generate_decorations(Vector3i chunk_position, Vector3i biome_coordinate) { }