#include "../../include/biomes/anger_biome.h"
#include "../../include/world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

DEFINE_CONSTRUCTORS_AND_BINDINGS(AngerBiome);

void AngerBiome::initialize() {
    Biome::initialize();
    terrain_noise = world->generator->noise[8];
}

void AngerBiome::generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate) {
    uint16_t rng_seed = world->generator->seed;

    Random::scramble_rng_seed(&rng_seed, Vector3i(chunk_position.x, 0, chunk_position.z));

    for (int64_t z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
    for (int64_t x = 0; x < Chunk::CHUNK_SIZE_X; x++) {
        int64_t ground_level = get_ground_level(chunk_position + Vector3i(x, 0, z), biome_coordinate);
        int64_t water_level = get_water_level(chunk_position + Vector3i(x, 0, z), biome_coordinate);

        for (int64_t y = 0; y < Chunk::CHUNK_SIZE_Y; y++) {
            Vector3i position = chunk_position + Vector3i(x, y, z);

            if (block_locked(chunk, Vector3i(x, y, z))) {
                continue;
            }

            int32_t block_type = 0;

            if (position.y == ground_level) {
                block_type = world->block_name_map["anger block"];
            }

            if (is_on_vertical_border(chunk, position, Vector3i(x, y, z))) {
                block_type = world->block_name_map["simblock"];
            }

            chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = block_type;
            chunk->water[Chunk::position_to_index(Vector3i(x, y, z))] = (y + chunk_position.y) <= water_level ? 255 : 0;
        }
    }
    }
}

int64_t AngerBiome::get_ground_level(Vector3i position, Vector3i biome_coordinate) {
    int64_t water_level = get_water_level(position, biome_coordinate);
    bool on_ground = terrain_noise->get_noise_2d(position.x, position.z) > -0.038;
    return on_ground ? water_level + 16 : GROUND_LEVEL_INVALID;
}
