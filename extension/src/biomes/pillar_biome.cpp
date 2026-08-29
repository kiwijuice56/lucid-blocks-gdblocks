#include "../../include/biomes/pillar_biome.h"
#include "../../include/world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

DEFINE_CONSTRUCTORS_AND_BINDINGS(PillarBiome);

void PillarBiome::initialize() {
    Biome::initialize();
}

void PillarBiome::generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate)  {
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

        bool x_aligned = position.x % 2 == 0;
        bool z_aligned = position.z % 2 == 0;

        int32_t aligned = 0;
        if (x_aligned) aligned++;
        if (z_aligned) aligned++;

        int32_t block_type = 0;

        if (position.y <= water_level) {
            block_type = world->block_name_map["sand block"];
        } else if (aligned < 2) {
            block_type = 0;
        } else {
            block_type = world->block_name_map["cactus block"];
        }

        chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = block_type;
        chunk->water[Chunk::position_to_index(Vector3i(x, y, z))] = 0;
    }
    }
    }
}

void PillarBiome::generate_decorations(Vector3i chunk_position, Vector3i biome_coordinate) { }