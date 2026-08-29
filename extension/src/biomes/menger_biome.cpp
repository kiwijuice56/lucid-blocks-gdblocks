#include "../../include/biomes/menger_biome.h"
#include "../../include/world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

DEFINE_CONSTRUCTORS_AND_BINDINGS(MengerBiome);

void MengerBiome::initialize() {
    Biome::initialize();
    menger_noise = world->generator->noise[5];
}

void MengerBiome::generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate)  {
    uint16_t rng_seed = world->generator->seed;

    Ref<Block> main_block = building_blocks[0];
    int block_index = main_block->index;

    for (int64_t y = 0; y < Chunk::CHUNK_SIZE_Y; y++) {
    for (int64_t z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
    for (int64_t x = 0; x < Chunk::CHUNK_SIZE_X; x++) {
        int64_t water_level = get_water_level(chunk_position + Vector3i(x, 0, z), biome_coordinate);
        Vector3i position = chunk_position + Vector3i(x, y, z);
        Vector3i old_position = position;

        Random::scramble_rng_seed(&rng_seed, position);

        if (block_locked(chunk, Vector3i(x, y, z))) {
            continue;
        }

        int block_type = block_index;

        if (menger_noise->get_noise_3d(position.x, position.y, position.z) < 0.62) {
            block_type = 0;
        } else {
            while (UtilityFunctions::absi(position.x) > 0 || UtilityFunctions::absi(position.y) > 0 || UtilityFunctions::absi(position.z) > 0) {
                int cx = UtilityFunctions::posmod(position.x, 3);
                int cy = UtilityFunctions::posmod(position.y, 3);
                int cz = UtilityFunctions::posmod(position.z, 3);

                int hole_axes = (cx == 1) + (cy == 1) + (cz == 1);
                if (hole_axes >= 2) {
                    block_type = 0;
                    break;
                }

                position.x /= 3;
                position.y /= 3;
                position.z /= 3;
            }
        }

        if (is_on_vertical_border(chunk, old_position, Vector3i(x, y, z))) {
            block_type = block_index;
        }

        chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = block_type;
        chunk->water[Chunk::position_to_index(Vector3i(x, y, z))] = old_position.y <= water_level ? 255 : 0;
    }
    }
    }
}

void MengerBiome::generate_decorations(Vector3i chunk_position, Vector3i biome_coordinate) { }