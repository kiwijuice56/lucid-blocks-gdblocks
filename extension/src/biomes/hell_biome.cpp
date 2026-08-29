#include "../../include/biomes/hell_biome.h"
#include "../../include/world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

DEFINE_CONSTRUCTORS_AND_BINDINGS(HellBiome);

void HellBiome::initialize() {
    Biome::initialize();
    hell_noise = world->generator->noise[27];
}

void HellBiome::generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate) {
    uint16_t rng_seed = world->generator->seed;
    Random::scramble_rng_seed(&rng_seed, chunk_position);

    const double FOLIAGE_RARE_CHANCE = 0.03;
    const double FOLIAGE_CHANCE = 0.12;

    bool melon_chunk = Random::randf(rng_seed) < rare_chunk_chance;

    for (int64_t z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
    for (int64_t x = 0; x < Chunk::CHUNK_SIZE_X; x++) {
    for (int64_t y = 0; y < Chunk::CHUNK_SIZE_Y; y++) {
        Vector3i local_position = Vector3i(x, y, z);
        Vector3i position = chunk_position + local_position;
        Random::scramble_rng_seed(&rng_seed, position);

        if (block_locked(chunk, local_position)) {
            continue;
        }

        int block_type = 0;

        double noise = hell_noise->get_noise_3dv(position);
        
        if (noise < -0.1) {
            block_type = world->block_name_map["dirt block"];
        } else if (is_foliage_safe(chunk, position, local_position)) {
            int below_index = chunk->blocks[Chunk::position_to_index(local_position - Vector3i(0, 1, 0))];

            if (!chunk->is_block_foliage[below_index]) {
                // Pick between common/uncommon foliage
                if (Random::randf(rng_seed) < FOLIAGE_RARE_CHANCE) {
                    if (melon_chunk) {
                        block_type = (int) world->block_name_map["eye cyst block"];
                    } else {
                        block_type = (int) world->block_name_map["dandelion foliage"];
                    }
                } else if (Random::randf(rng_seed) < FOLIAGE_CHANCE) {
                    block_type = (int) world->block_name_map["grass foliage"];
                }
            }
        }

        chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = block_type;
        chunk->water[Chunk::position_to_index(Vector3i(x, y, z))] = 0;
    }
    }
    }
}

void HellBiome::generate_decorations(Vector3i chunk_position, Vector3i biome_coordinate) { }