#include "../../include/structures/sponge.h"
#include "../../include/world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

DEFINE_CONSTRUCTORS_AND_BINDINGS(Sponge);

void Sponge::generate_chunk_data(Chunk* chunk, Vector3i chunk_position) {
    if (invalid) {
        return;
    }

    uint16_t rng_seed = world->generator->seed;

    for (int64_t y = 0; y < Chunk::CHUNK_SIZE_Y; y++) {
    for (int64_t z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
    for (int64_t x = 0; x < Chunk::CHUNK_SIZE_X; x++) {
        Vector3i position = chunk_position + Vector3i(x, y, z);
        Random::scramble_rng_seed(&rng_seed, position);
        double distance = position.distance_to(heart_position) / 86.0;

        if (chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] == 0 && sponge_noise->get_noise_3d(position.x, position.y * 0.5, position.z) - distance > -0.7) {
            chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = world->block_name_map["moss block"];
            chunk->water[Chunk::position_to_index(Vector3i(x, y, z))] = 0;
        } else if (Random::randf(rng_seed) < 0.1 && y > 0 && chunk->blocks[Chunk::position_to_index(Vector3i(x, y - 1, z))] == (int) world->block_name_map["moss block"]) {
            chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = world->block_name_map["moss fuzz foliage"];
        }
    }
    }
    }
}

void Sponge::initialize() {
    sponge_noise = world->generator->noise[15];
}

bool Sponge::is_within_structure(Vector3i position) {
    return position.distance_to(heart_position) <= 80.0;
}
