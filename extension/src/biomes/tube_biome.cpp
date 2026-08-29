#include "../../include/biomes/tube_biome.h"
#include "../../include/world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

DEFINE_CONSTRUCTORS_AND_BINDINGS(TubeBiome);

void TubeBiome::initialize() {
    Biome::initialize();
    pipe_noise = world->generator->noise[5];
}

void TubeBiome::generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate)  {
    uint16_t rng_seed = world->generator->seed;

    for (int64_t y = 0; y < Chunk::CHUNK_SIZE_Y; y++) {
    for (int64_t z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
    for (int64_t x = 0; x < Chunk::CHUNK_SIZE_X; x++) {

        Vector3i position = chunk_position + Vector3i(x, y, z);

        Random::scramble_rng_seed(&rng_seed, position);

        if (block_locked(chunk, Vector3i(x, y, z))) {
            continue;
        }

        bool x_aligned = position.x % 19 == 0;
        bool y_aligned = position.y % 19 == 0;
        bool z_aligned = position.z % 19 == 0;

        int32_t aligned = 0;
        if (x_aligned) aligned++;
        if (y_aligned) aligned++;
        if (z_aligned) aligned++;

        int32_t block_type = 0;

        if (aligned < 2) {
            block_type = 0;
        } else {
            float presence_noise = pipe_noise->get_noise_3dv(Vector3(position) *  Vector3(0.7, 0.7, 0.7));

            if (presence_noise < +0.1) {
                chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = 0;
            } else {
                float color_noise = pipe_noise->get_noise_3dv(Vector3(position) * Vector3(0.4, 0.4, 0.4));
                block_type = world->block_name_map["blue plastic block"];
                if (color_noise < +0.5) block_type = world->block_name_map["yellow plastic block"];
                if (color_noise < +0.07) block_type = world->block_name_map["green plastic block"];
                if (color_noise < -0.25) block_type = world->block_name_map["red plastic block"];
            }
        }

        chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = block_type;
        chunk->water[Chunk::position_to_index(Vector3i(x, y, z))] = 0;
    }
    }
    }
}

void TubeBiome::generate_decorations(Vector3i chunk_position, Vector3i biome_coordinate) { }