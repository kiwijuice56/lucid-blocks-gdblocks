#include "../../include/biomes/ball_biome.h"
#include "../../include/world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

DEFINE_CONSTRUCTORS_AND_BINDINGS(BallBiome);

void BallBiome::generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate)  {
    uint16_t rng_seed = world->generator->seed;
    Ref<Noise> pipe_noise = world->generator->noise[5];

    for (int64_t y = 0; y < Chunk::CHUNK_SIZE_Y; y++) {
    for (int64_t z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
    for (int64_t x = 0; x < Chunk::CHUNK_SIZE_X; x++) {

        Vector3i position = chunk_position + Vector3i(x, y, z);

        Random::scramble_rng_seed(&rng_seed, position);

        if (block_locked(chunk, Vector3i(x, y, z))) {
            continue;
        }

        const int GRID_SIZE = 69;
        const int RADIUS = 26;

        Vector3i center = Vector3i(position.x, position.y, position.z).snappedi(GRID_SIZE);

        int squared_distance = position.distance_squared_to(center);

        int32_t block_type = 0;

        if (squared_distance <= RADIUS * RADIUS) {
            float presence_noise = pipe_noise->get_noise_3dv(Vector3(position) * Vector3(0.45, 0.45, 0.45));

            if (presence_noise < -0.25) {
                block_type = 0;
            } else {
                rng_seed = world->generator->seed;
                Random::scramble_rng_seed(&rng_seed, center);
                block_type = Ref<Block>(building_blocks[Random::randi(rng_seed) % building_blocks.size()])->index;
            }
        }

        chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = block_type;
        chunk->water[Chunk::position_to_index(Vector3i(x, y, z))] = 0;
    }
    }
    }
}