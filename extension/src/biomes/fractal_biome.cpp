#include "../../include/biomes/fractal_biome.h"
#include "../../include/world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

DEFINE_CONSTRUCTORS_AND_BINDINGS(FractalBiome);

void FractalBiome::initialize() {
    terrain_noise = world->generator->noise[12];
    mud_noise = world->generator->noise[6];
    tendril_noise = world->generator->noise[13];
}

void FractalBiome::generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate) {
    uint16_t rng_seed = world->generator->seed;
    Random::scramble_rng_seed(&rng_seed, chunk_position);

    for (int64_t z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
    for (int64_t x = 0; x < Chunk::CHUNK_SIZE_X; x++) {
        int64_t water_level = get_water_level(chunk_position + Vector3i(x, 0, z), biome_coordinate);
        
        for (int64_t y = 0; y < Chunk::CHUNK_SIZE_Y; y++) {
            Vector3i local_position = Vector3i(x, y, z);
            Vector3i position = chunk_position + local_position;
            Random::scramble_rng_seed(&rng_seed, position);

            if (block_locked(chunk, local_position)) {
                continue;
            }

            int32_t block_type = get_block_at(position);

            // Biome floor
            if (is_on_vertical_border(chunk, position, local_position)) {
                block_type = world->block_name_map["mud block"];
            }

            chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = block_type;
            chunk->water[Chunk::position_to_index(Vector3i(x, y, z))] = position.y <= water_level ? 255 : 0;
        }
    }
    }
}

int32_t FractalBiome::get_block_at(Vector3i position) {
    position.y = UtilityFunctions::posmod(position.y, world->generator->Y_PER_BIOME_Y);
    double terrain = 0.39 * (60 - position.y) / (double) 60;
    if (terrain_noise->get_noise_3dv(position) < terrain + 0.1) {
        return mud_noise->get_noise_3dv(7 * position) < -0.2 ? world->block_name_map["mud block"] : world->block_name_map["black sand block"];
    } else if (tendril_noise->get_noise_3dv(position * Vector3i(4, 1, 4)) < -0.2 + UtilityFunctions::minf(0, terrain)) {
        return world->block_name_map["clay block"];
    }
    return 0;
}

void FractalBiome::generate_decorations(Vector3i chunk_position, Vector3i biome_coordinate) { }

int64_t FractalBiome::get_ground_level(Vector3i position, Vector3i biome_coordinate) {
    return GROUND_LEVEL_INVALID;
}

bool FractalBiome::is_solid_at(Vector3i position) {
    return get_block_at(position) != 0;
}
