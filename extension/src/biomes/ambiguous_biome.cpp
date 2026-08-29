#include "../../include/biomes/ambiguous_biome.h"
#include "../../include/world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

DEFINE_CONSTRUCTORS_AND_BINDINGS(AmbiguousBiome);

void AmbiguousBiome::generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate) {
    uint16_t rng_seed = world->generator->seed;

    for (int64_t z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
    for (int64_t x = 0; x < Chunk::CHUNK_SIZE_X; x++) {
        int64_t snapped_x = BLOCK_SIZE * ((x + chunk_position.x) / BLOCK_SIZE);
        int64_t snapped_z = BLOCK_SIZE * ((z + chunk_position.z) / BLOCK_SIZE);
        int64_t ground_level = get_ground_level(chunk_position + Vector3i(x, 0, z), biome_coordinate);
        int64_t water_level = get_water_level(chunk_position + Vector3i(x, 0, z), biome_coordinate);

        for (int64_t y = 0; y < Chunk::CHUNK_SIZE_Y; y++) {
            Random::scramble_rng_seed(&rng_seed, chunk_position + Vector3i(x, y, z));

            if (block_locked(chunk, Vector3i(x, y, z))) {
                continue;
            }

            Vector3i position = chunk_position + Vector3i(x, y, z);

            int64_t real_y = y + chunk_position.y;
            int64_t snapped_y = BLOCK_SIZE * (real_y / BLOCK_SIZE);
            int32_t block_type = 0;

            if (real_y == ground_level + 1 && Random::randf(rng_seed) < 0.005 && y > 0 && !block_locked(chunk, Vector3i(x, y - 1, z))) {
                block_type = world->block_name_map["grave foliage"];
            } else if (real_y <= ground_level) {
                int64_t checker = ((snapped_x + snapped_z) / BLOCK_SIZE) % 5;
                if (checker < 0) {
                    checker = -checker;
                }
                block_type = world->block_name_map[checker == 0 ? "ambiguous block 3" : (checker == 1 ? "ambiguous block 2" : (checker == 2 ? "ambiguous block 1" : (checker == 3 ? "ambiguous block 4" : "ambiguous block 5")))];
            }

            chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = block_type;
            chunk->water[Chunk::position_to_index(Vector3i(x, y, z))] = real_y <= water_level ? 255 : 0;
        }
    }
    }
}

void AmbiguousBiome::generate_decorations(Vector3i chunk_position, Vector3i biome_coordinate) {
    uint16_t rng_seed = world->generator->seed;

    for (int64_t z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
    for (int64_t x = 0; x < Chunk::CHUNK_SIZE_X; x++) {
        Vector3i position = Vector3i(x, 0, z) + chunk_position;

        // Check if decoration should exist here
        Random::scramble_rng_seed(&rng_seed, position);
        if (Random::randf(rng_seed) > DECORATION_SPAWN_CHANCE) {
            continue;
        }

        // Find decoration type
        Random::scramble_rng_seed(&rng_seed, position);
        uint8_t decoration_index = Random::randi(rng_seed) % NUM_SURFACE_DECORATIONS;

        // Check if this vertical column contains the block above ground-level
        int64_t ground_level = get_ground_level(position, biome_coordinate);
        int64_t y = ground_level - chunk_position.y;

        // Make cubes fly
        if (decoration_index > 4) {
            y += 12 + (Random::randi(rng_seed) % 12);
        }

        if (y < 0 || y > Chunk::CHUNK_SIZE_Y) {
            continue;
        }

        position = chunk_position + Vector3i(x, y, z);

        // Place the decoration
        Ref<DecorationState> d = memnew(DecorationState);
        String decoration_name = surface_decorations[decoration_index];
        d->decoration = world->decoration_name_map[decoration_name];
        d->position = position;
        d->direction = Random::randdir(rng_seed);

        world->place_decoration(d);
    }
    }
}

int64_t AmbiguousBiome::get_ground_level(Vector3i position, Vector3i biome_coordinate) {
    int biome_height_offset = world->generator->Y_PER_BIOME_Y * (biome_coordinate.y - 6);
    int64_t snapped_x = BLOCK_SIZE * (position.x / BLOCK_SIZE);
    int64_t snapped_z = BLOCK_SIZE * (position.z / BLOCK_SIZE);
    if (!is_cliff) {
        int64_t y = (int64_t) (96.0 * (0.425 + terrain_noise->get_noise_2d(snapped_x, snapped_z)));
        int64_t snapped_y = BLOCK_SIZE * (y / BLOCK_SIZE);
        return biome_height_offset + snapped_y;
    } else {
        float height = terrain_noise->get_noise_2d(snapped_x, snapped_z);
        int64_t y = (int64_t) (96.0 * (0.15 + height / 2 + UtilityFunctions::pow(height, 8) + UtilityFunctions::pow(height + 0.5, 4)));
        int64_t snapped_y = BLOCK_SIZE * (y / BLOCK_SIZE);
        return biome_height_offset + snapped_y;
    }
}
