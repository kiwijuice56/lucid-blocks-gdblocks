#include "../../include/biomes/flesh_biome.h"
#include "../../include/world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

DEFINE_CONSTRUCTORS_AND_BINDINGS(FleshBiome);

void FleshBiome::initialize() {
    river_noise_1 = world->generator->noise[23];
    river_noise_2 = world->generator->noise[24];
}

void FleshBiome::generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate) {
    Ref<Noise> terrain_noise = world->generator->noise[7];
    int64_t flesh_number = world->block_name_map["flesh block"];
    uint16_t rng_seed = world->generator->seed;

    for (int64_t z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
    for (int64_t x = 0; x < Chunk::CHUNK_SIZE_X; x++) {
        int biome_height_offset = world->generator->Y_PER_BIOME_Y * (biome_coordinate.y - 6);
        float height = terrain_noise->get_noise_2d(x + chunk_position.x, z + chunk_position.z);
        int64_t ground_level = biome_height_offset + (int64_t) (12.0 * (0.1 + height / 2 + UtilityFunctions::pow(height, 3) + UtilityFunctions::pow(height + 0.5, 2)));
        int64_t water_level = get_water_level(chunk_position + Vector3i(x, 0, z), biome_coordinate);

        for (int64_t y = 0; y < Chunk::CHUNK_SIZE_Y; y++) {
            Vector3i position = chunk_position + Vector3i(x, y, z);

            Random::scramble_rng_seed(&rng_seed, position);

            if (block_locked(chunk, Vector3i(x, y, z))) {
                continue;
            }

            int64_t block_type = get_block_at(chunk, position, ground_level, terrain_noise, rng_seed);

            // Sometimes spawn grass if block below is moss
            if (block_type == 0 && y > 0 && !block_locked(chunk, Vector3i(x, y - 1, z)) && Random::randf(rng_seed) < 0.01 && chunk->blocks[Chunk::position_to_index(Vector3i(x, y - 1, z))] == flesh_number) {
                block_type = Random::randf(rng_seed) < 0.005 ? world->block_name_map["flesh foliage"] : world->block_name_map["cyst foliage"];
            }

            if (is_on_vertical_border(chunk, position, Vector3i(x, y, z))) {
                block_type = world->block_name_map["flesh block"];
            }

            // Bones
            const double MAIN_THRESHOLD = 0.21;
            const double SECONDARY_THRESHOLD = 0.035;
            const double WARP = 12.0;
            const double Y_DECAY = 0.004;
            const double Y_POWER = 3.5;

            double presence_noise = river_noise_1->get_noise_3d(position.x, position.y, position.z);
            if (UtilityFunctions::absf(presence_noise) < MAIN_THRESHOLD - UtilityFunctions::pow(Y_DECAY * UtilityFunctions::maxi(0, position.y), Y_POWER)) {
                double angle = river_noise_2->get_noise_3d(position.x, position.y, position.z) * 2 * Math_PI;
                Vector3 warp = WARP * Vector3(UtilityFunctions::cos(angle), UtilityFunctions::cos(angle), UtilityFunctions::sin(angle));

                double warped_noise = river_noise_1->get_noise_3d(position.x + warp.x, position.y + warp.y, position.z + warp.z);
                if (UtilityFunctions::absf(warped_noise) < SECONDARY_THRESHOLD) {
                    block_type = world->block_name_map["skin block"];
                }
            }

            chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = block_type;
            chunk->water[Chunk::position_to_index(Vector3i(x, y, z))] = (y + chunk_position.y) <= water_level ? 255 : 0;
        }
    }
    }
}

// Helper method to get individual blocks, since we sometimes have to check twice
int64_t FleshBiome::get_block_at(Chunk* chunk, Vector3i position, int64_t ground_level, Ref<Noise> terrain_noise, uint16_t rng_seed) {
    double terrain = terrain_noise->get_noise_3dv(position);

    if (ground_level <= 0) {
        ground_level = 1;
    }
    terrain += 0.3 * (ground_level - position.y) / (double) ground_level;

    int32_t block_type = 0;

    if (terrain < 0.1) {
        return 0;
    } else  {
        if (Random::randf(rng_seed) < 0.001) {
            return world->block_name_map["eye cyst block"];
        } else {
            return world->block_name_map["flesh block"];
        }
    }
}

void FleshBiome::generate_decorations(Vector3i chunk_position, Vector3i biome_coordinate) { }

int64_t FleshBiome::get_ground_level(Vector3i position, Vector3i biome_coordinate) {
    return GROUND_LEVEL_INVALID;
}
