#include "../../include/biomes/flower_biome.h"
#include "../../include/world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void FlowerBiome::_bind_methods() {
    BIND_PROPERTY(FlowerBiome, int, base_height);
    BIND_PROPERTY(FlowerBiome, int, grid_size);
    BIND_PROPERTY(FlowerBiome, int, scaffold_levels);
    BIND_PROPERTY(FlowerBiome, float, flower_frequency);
}

DEFINE_CONSTRUCTORS(FlowerBiome);

DEFINE_PROPERTY_GETTER_SETTER(FlowerBiome, int, base_height);
DEFINE_PROPERTY_GETTER_SETTER(FlowerBiome, int, grid_size);
DEFINE_PROPERTY_GETTER_SETTER(FlowerBiome, int, scaffold_levels);
DEFINE_PROPERTY_GETTER_SETTER(FlowerBiome, float, flower_frequency);

void FlowerBiome::generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate) {
    Ref<Block> edge_block = building_blocks[0];
    Ref<Block> floor_block = building_blocks[1];
    Ref<Block> soil_block = building_blocks[2];
    
    int edge_block_index = edge_block->index;
    int floor_block_index = floor_block->index;
    int soil_block_index = soil_block->index;

    uint16_t rng_seed = world->generator->seed;

    for (int z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
    for (int x = 0; x < Chunk::CHUNK_SIZE_X; x++) {
        int snapped_x = grid_size * ((x + chunk_position.x) / grid_size);
        int snapped_z = grid_size * ((z + chunk_position.z) / grid_size);
        int ground_level = get_ground_level(Vector3(snapped_x, 0, snapped_z), biome_coordinate);
        int64_t water_level = get_water_level(chunk_position + Vector3i(x, 0, z), biome_coordinate);

        for (int y = 0; y < Chunk::CHUNK_SIZE_Y; y++) {
            Vector3i position = chunk_position + Vector3i(x, y, z);

            Random::scramble_rng_seed(&rng_seed, position);

            if (block_locked(chunk, Vector3i(x, y, z))) {
                continue;
            }

            int ground_level_grid = (position.y - ground_level + base_height) / grid_size;
            int block_type = 0;

            int x_local = UtilityFunctions::posmod(position.x, grid_size);
            int y_local = UtilityFunctions::posmod(position.y, grid_size);
            int z_local = UtilityFunctions::posmod(position.z, grid_size);

            bool x_aligned = x_local == 0;
            bool y_aligned = y_local == 0;
            bool z_aligned = z_local == 0;

            int aligned = 0;
            if (x_aligned) aligned++;
            if (y_aligned) aligned++;
            if (z_aligned) aligned++;

            if (ground_level_grid > 0 && ground_level_grid <= scaffold_levels) {
                if (aligned >= 2) {
                    block_type = edge_block_index;
                }
            } else if (ground_level_grid == 0) {
                if (aligned >= 2) {
                    block_type = edge_block_index;
                } else if (y_aligned) {
                    block_type = floor_block_index;
                } else if (is_foliage_safe(chunk, position, Vector3i(x, y, z)) && y_local == 1 && !(z_aligned || x_aligned)) {
                    if (Random::randf(rng_seed) > flower_frequency) {
                        block_type = 0;
                    } else {
                        Random::scramble_rng_seed(&rng_seed, position * Vector3(7, 13, 17));
                        Ref<Block> flower = building_blocks[3 + Random::randi(rng_seed) % (building_blocks.size() - 3)];
                        int flower_index = flower->index;
                        block_type = flower_index;
                    }
                }
            } else if (ground_level_grid < 0) {
                if (aligned >= 2) {
                    block_type = edge_block_index;
                } else {
                    block_type = soil_block_index;
                } 
            }       
            chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = block_type;
            chunk->water[Chunk::position_to_index(Vector3i(x, y, z))] = position.y <= water_level ? 255 : 0;
        }
    }
    }
}
