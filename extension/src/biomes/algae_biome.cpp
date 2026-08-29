#include "../../include/biomes/algae_biome.h"
#include "../../include/world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

DEFINE_CONSTRUCTORS_AND_BINDINGS(AlgaeBiome);

void AlgaeBiome::generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate) {
    Ref<Noise> terrain_noise = world->generator->noise[6];
    int64_t moss_number = world->block_name_map["moss block"];
    uint16_t rng_seed = world->generator->seed;

    for (int64_t z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
    for (int64_t x = 0; x < Chunk::CHUNK_SIZE_X; x++) {
        int biome_height_offset = world->generator->Y_PER_BIOME_Y * (biome_coordinate.y - 6);
        float height = terrain_noise->get_noise_2d(x + chunk_position.x, z + chunk_position.z);
        int64_t ground_level = biome_height_offset + (int64_t) (96.0 * (0.25 + height / 2 + UtilityFunctions::pow(height, 8) + UtilityFunctions::pow(height + 0.5, 4)));
        int64_t water_level = get_water_level(chunk_position + Vector3i(x, 0, z), biome_coordinate);

        for (int64_t y = 0; y < Chunk::CHUNK_SIZE_Y; y++) {
            Random::scramble_rng_seed(&rng_seed, chunk_position + Vector3i(x, y, z));

            if (block_locked(chunk, Vector3i(x, y, z))) {
                continue;
            }

            int64_t block_type = get_block_at(chunk, chunk_position + Vector3i(x, y, z), ground_level, terrain_noise);

            // Sometimes spawn grass if block below is moss
            if (block_type == 0 && y > 0 && !block_locked(chunk, Vector3i(x, y - 1, z)) && Random::randf(rng_seed) < 0.17 && chunk->blocks[Chunk::position_to_index(Vector3i(x, y - 1, z))] == moss_number) {
                block_type = world->block_name_map["moss fuzz foliage"];
            }

            if (is_on_vertical_border(chunk, chunk_position + Vector3i(x, y, z), Vector3i(x, y, z))) {
                block_type = world->block_name_map["limestone tile block"];
            }

            chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = block_type;
            chunk->water[Chunk::position_to_index(Vector3i(x, y, z))] = (y + chunk_position.y) <= water_level ? 255 : 0;
        }
    }
    }
}

// Helper method to get individual blocks, since we sometimes have to check twice
int64_t AlgaeBiome::get_block_at(Chunk* chunk, Vector3i position, int64_t ground_level, Ref<Noise> terrain_noise) {
    double terrain = terrain_noise->get_noise_3d(position.x, position.y, position.z);

    if (ground_level <= 0) {
        ground_level = 1;
    }

    terrain += 0.55 * (ground_level - position.y) / (double) ground_level;

    int32_t block_type = 0;

    if (terrain < 0.1) {
        return 0;
    } else  {
        double moss = terrain_noise->get_noise_3d(4.0 * position.x, 6.0 * position.y, 4.0 * position.z);
        if (moss < -0.2) {
            return world->block_name_map["moss block"];
        } else {
            return world->block_name_map["limestone block"];
        }
    }
}

int64_t AlgaeBiome::get_ground_level(Vector3i position, Vector3i biome_coordinate) {
    return GROUND_LEVEL_INVALID;
}
