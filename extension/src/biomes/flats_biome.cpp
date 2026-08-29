#include "../../include/biomes/flats_biome.h"
#include "../../include/world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

DEFINE_CONSTRUCTORS_AND_BINDINGS(FlatsBiome);

bool FlatsBiome::is_tile_highway(Vector3i position) {
    return true;
}

void FlatsBiome::generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate) {
    const int GRID_SIZE = 36;
    const int MAX_BUILDING_HEIGHT = 6;
    const int FLOOR_HEIGHT = 6;

    const int ROAD_SIZE = 3;
    const int SIDEWALK_SIZE = 2;
    const int GRASS_SIZE = 3;

    for (int64_t z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
    for (int64_t x = 0; x < Chunk::CHUNK_SIZE_X; x++) {
        // Snap the column to the plot
        Vector3i corner = chunk_position + Vector3i(x, 0, z);
        int ground_level = get_ground_level(corner, biome_coordinate);
        int water_level = get_water_level(chunk_position + Vector3i(x, 0, z), biome_coordinate);
       
        Vector3i origin_column = Vector3i(
            UtilityFunctions::floori((float) (chunk_position.x + x) / (float)GRID_SIZE) * GRID_SIZE, 0,
            UtilityFunctions::floori((float) (chunk_position.z + z) / (float)GRID_SIZE) * GRID_SIZE
        );

        uint16_t origin_seed = world->generator->seed;

        // Find the properties of this plot
        Random::scramble_rng_seed(&origin_seed, 3 * origin_column);
        bool is_highway = Random::randf(origin_seed) < 0.4;

        Random::scramble_rng_seed(&origin_seed, 7 * origin_column);
        bool is_building = Random::randf(origin_seed) < 0.65;

        Random::scramble_rng_seed(&origin_seed, 13 * origin_column);
        bool has_windows = Random::randf(origin_seed) < 0.55;

        Random::scramble_rng_seed(&origin_seed, 19 * origin_column);
        int building_height = Random::randi(origin_seed) % MAX_BUILDING_HEIGHT;

        Random::scramble_rng_seed(&origin_seed, 37 * origin_column);
        int32_t building_block = world->block_name_map[building_colors[(int) (Random::randf(origin_seed) * NUM_BUILDING_COLORS)]];

        for (int64_t y = 0; y < Chunk::CHUNK_SIZE_Y; y++) {
            if (block_locked(chunk, Vector3i(x, y, z))) {
                continue;
            }

            Vector3i position = chunk_position + Vector3i(x, y, z);
            Vector3i origin = Vector3i(
                UtilityFunctions::floori((float) position.x / (float) GRID_SIZE) * GRID_SIZE,
                UtilityFunctions::floori((float) position.y / (float) GRID_SIZE) * GRID_SIZE,
                UtilityFunctions::floori((float) position.z / (float) GRID_SIZE) * GRID_SIZE
            );

            Vector3i relative = position - origin;

            int32_t block_type = 0;

            const int DIRT_SIZE = 4;
            if (position.y < ground_level - DIRT_SIZE) {
                block_type = world->block_name_map["black plastic block"];
            } else if (position.y < ground_level) {
                block_type = world->block_name_map["brown plastic block"];
            } else if (position.y == ground_level) {
                if (in_ring(relative, ROAD_SIZE, GRID_SIZE)) {
                    block_type = world->block_name_map["black plastic block"];
                } else if (in_ring(relative, ROAD_SIZE + SIDEWALK_SIZE, GRID_SIZE)) {
                    block_type = world->block_name_map["white plastic block"];
                } else if (is_building && in_ring(relative, ROAD_SIZE + SIDEWALK_SIZE + GRASS_SIZE, GRID_SIZE)) {
                    block_type = world->block_name_map["green plastic block"];
                } else {
                    block_type = is_building ? world->block_name_map["white plastic block"] : world->block_name_map["green plastic block"];
                }
            } else if (is_building) {
                int level_blocks = (position.y - ground_level) - 1;
                int level_story = (int) (level_blocks / (double) FLOOR_HEIGHT);
                if (in_ring(relative, ROAD_SIZE + SIDEWALK_SIZE + GRASS_SIZE - 1, GRID_SIZE)) {
                    // Outside of building
                } else if (in_ring(relative, ROAD_SIZE + SIDEWALK_SIZE + GRASS_SIZE, GRID_SIZE)) {
                    // Wall of building
                    if (level_story <= building_height) {
                        int corner_x = UtilityFunctions::min(relative.x, GRID_SIZE - relative.x - 1);
                        int corner_z = UtilityFunctions::min(relative.z, GRID_SIZE - relative.z - 1);
                        if (has_windows && (corner_x != corner_z && relative.y % 3 == 2 && (relative.z + relative.x) % 3 == 0)) {

                        } else {
                            block_type = building_block;
                        }
                    }
                } else {
                    // Interior of building
                    if (level_story <= building_height && level_blocks % MAX_BUILDING_HEIGHT == MAX_BUILDING_HEIGHT - 1) {
                        block_type = building_block;
                    }
                }
            }

            chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = block_type;
            chunk->water[Chunk::position_to_index(Vector3i(x, y, z))] = position.y <= water_level ? 255 : 0;
        }
    }
    }
}

bool FlatsBiome::in_ring(Vector3i position, int ring_size, int grid_size) {
    return position.x < ring_size || position.z < ring_size || position.x >= grid_size - ring_size || position.z >= grid_size - ring_size;
}

int64_t FlatsBiome::get_ground_level(Vector3i position, Vector3i biome_coordinate) {
    int biome_height_offset = world->generator->Y_PER_BIOME_Y * (biome_coordinate.y - 6);
    int ground_level = biome_height_offset + 36;
    return ground_level - ground_level % 3;
}