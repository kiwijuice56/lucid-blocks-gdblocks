#include "../../include/biomes/moon_biome.h"
#include "../../include/world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

DEFINE_CONSTRUCTORS_AND_BINDINGS(MoonBiome);

void MoonBiome::initialize() {
    Biome::initialize();
    terrain_noise = world->generator->noise[6];
}

void MoonBiome::generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate) {
    uint16_t rng_seed = world->generator->seed;

    for (int64_t z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
    for (int64_t x = 0; x < Chunk::CHUNK_SIZE_X; x++) {
        float height = terrain_noise->get_noise_2d(x + chunk_position.x, z + chunk_position.z);
        int64_t water_level = get_water_level(chunk_position + Vector3i(x, 0, z), biome_coordinate);

        for (int64_t y = 0; y < Chunk::CHUNK_SIZE_Y; y++) {
            Random::scramble_rng_seed(&rng_seed, chunk_position + Vector3i(x, y, z));

            if (block_locked(chunk, Vector3i(x, y, z))) {
                continue;
            }

            int64_t block_type = get_block_at(chunk, chunk_position + Vector3i(x, y, z), biome_coordinate, 0);

            chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = block_type;
            chunk->water[Chunk::position_to_index(Vector3i(x, y, z))] = 0;
        }
    }
    }
}

// Helper method to get individual blocks, since we sometimes have to check twice
int64_t MoonBiome::get_block_at(Chunk* chunk, Vector3i position, Vector3i biome_coordinate, int64_t ground_level) {
    double terrain = terrain_noise->get_noise_3d(position.x, position.y, position.z);

    if (terrain < 0.0) {
        int biome_height_offset = world->generator->Y_PER_BIOME_Y * (biome_coordinate.y - 6);
        float close_to_border_penalty_1 = UtilityFunctions::clampf((40 + biome_height_offset - position.y) / 32.0f, 0.0f, 1.0f);
        float close_to_border_penalty_2 = UtilityFunctions::clampf((position.y - (biome_height_offset + world->generator->Y_PER_BIOME_Y - 76) ) / 48.0f, 0.0f, 1.0f);
        if (terrain + close_to_border_penalty_1 + close_to_border_penalty_2 > 0.0) {
            return 0;
        }

        return world->block_name_map["sky stone block"];
    } else  {
        return 0;
    }
}

int64_t MoonBiome::get_ground_level(Vector3i position, Vector3i biome_coordinate) {
    return GROUND_LEVEL_INVALID;
}
