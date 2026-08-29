#include "../../include/structures/pyramid.h"
#include "../../include/world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

DEFINE_CONSTRUCTORS_AND_BINDINGS(Pyramid);

void Pyramid::generate_chunk_data(Chunk* chunk, Vector3i chunk_position) {
    uint16_t rng_seed = world->generator->seed;

    for (int64_t y = 0; y < Chunk::CHUNK_SIZE_Y; y++) {
    for (int64_t z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
    for (int64_t x = 0; x < Chunk::CHUNK_SIZE_X; x++) {
        Vector3i position = chunk_position + Vector3i(x, y, z);
        Random::scramble_rng_seed(&rng_seed, position);

        // Offset slightly higher
        int64_t height = position.y - (heart_position.y + 32);

        // Base of the pyramid
        if (height < 0) {
            if (position.x > heart_position.x - size / 2 && position.z > heart_position.z - size / 2 &&
                position.x < heart_position.x + size / 2 && position.z < heart_position.z + size / 2) {

                chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = world->block_name_map["sandstone block"];
            }
        } else if (height < size / 2) {

            if (position.x > heart_position.x - size / 2 + height && position.z > heart_position.z - size / 2 + height &&
                position.x < heart_position.x + size / 2 - height && position.z < heart_position.z + size / 2 - height) {

                if (position.x == heart_position.x - size / 2 + height + 1 || position.z == heart_position.z - size / 2 + height + 1||
                    position.x == heart_position.x + size / 2 - height - 1 || position.z == heart_position.z + size / 2 - height - 1) {
                    chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = world->block_name_map["sandstone block"];
                } else {
                    chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = 0;
                }

               chunk->water[Chunk::position_to_index(Vector3i(x, y, z))] = 0;
            }
        }
    }
    }
    }
}

void Pyramid::initialize() {
    uint16_t rng_seed = world->generator->seed;

    Random::scramble_rng_seed(&rng_seed, heart_position);

    size = MIN_SIZE + Random::randi(rng_seed) % (MAX_SIZE - MIN_SIZE);
}

bool Pyramid::is_within_structure(Vector3i position) {
    return false;
}
