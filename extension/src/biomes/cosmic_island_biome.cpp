#include "../../include/biomes/cosmic_island_biome.h"
#include "../../include/world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void CosmicIslandBiome::_bind_methods() {
    BIND_PROPERTY(CosmicIslandBiome, int, grid_size);
    BIND_PROPERTY(CosmicIslandBiome, int, cube_size);
}

DEFINE_CONSTRUCTORS(CosmicIslandBiome);

DEFINE_PROPERTY_GETTER_SETTER(CosmicIslandBiome, int, grid_size);
DEFINE_PROPERTY_GETTER_SETTER(CosmicIslandBiome, int, cube_size);

void CosmicIslandBiome::generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate) {
    Ref<Block> cube_block = building_blocks[0];
    int cube_block_index = cube_block->index;

    Ref<Block> top_block = building_blocks[1];
    int top_block_index = top_block->index;

    Ref<Block> common_foliage = building_blocks[2];
    int common_foliage_index = common_foliage->index;

    Ref<Block> rare_foliage = building_blocks[3];
    int rare_foliage_index = rare_foliage->index;


    uint16_t rng_seed = world->generator->seed;

    for (int z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
    for (int x = 0; x < Chunk::CHUNK_SIZE_X; x++) {
        for (int y = 0; y < Chunk::CHUNK_SIZE_Y; y++) {
            Vector3i local_position = Vector3i(x, y, z);
            Vector3i position = chunk_position + local_position;

            Random::scramble_rng_seed(&rng_seed, local_position);

            if (block_locked(chunk, local_position)) {
                continue;
            }

            int block_type = 0;

            // Get local position within the grid cell
            int x_local = UtilityFunctions::posmod(position.x, grid_size);
            int y_local = UtilityFunctions::posmod(position.y, grid_size);
            int z_local = UtilityFunctions::posmod(position.z, grid_size);

            // Check if we're in the corner 8x8x8 cube (positions 0-7 in each axis)
            if (x_local < cube_size && y_local <= cube_size && z_local < cube_size) {
                if (y_local == cube_size) {
                    if (!is_foliage_safe(chunk, position, local_position) || Random::randf(rng_seed) > 0.3) {
                        block_type = 0;
                    } else {
                        block_type = Random::randf(rng_seed) < 0.04 ? rare_foliage_index : common_foliage_index;
                    }
                } else if (y_local == cube_size - 1) {
                    block_type = top_block_index;
                } else {
                    block_type = cube_block_index;
                }                
            }

            chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = block_type;
            chunk->water[Chunk::position_to_index(Vector3i(x, y, z))] = 0;
        }
    }
    }
}