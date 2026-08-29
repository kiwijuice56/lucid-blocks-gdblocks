#include "../../include/biomes/temple_town_biome.h"
#include "../../include/world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

DEFINE_CONSTRUCTORS_AND_BINDINGS(TempleTownBiome);

void TempleTownBiome::generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate) {
    const int GRID_SIZE = 36;
    const int MAX_BUILDING_HEIGHT = 6;
    const int FLOOR_HEIGHT = 6;

    const int ROAD_SIZE = 3;
    const int SIDEWALK_SIZE = 2;
    const int GRASS_SIZE = 3;

    Ref<Decoration> bottom_decoration = random_decorations[0];
    Ref<Decoration> middle_decoration = random_decorations[1];
    Ref<Decoration> top_decoration = random_decorations[2];

    for (int z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
    for (int x = 0; x < Chunk::CHUNK_SIZE_X; x++) {
        int ground_level = get_ground_level(chunk_position + Vector3i(x, 0, z), biome_coordinate);
        int64_t water_level = get_water_level(chunk_position + Vector3i(x, 0, z), biome_coordinate);

        for (int y = 0; y < Chunk::CHUNK_SIZE_Y; y++) {
            Vector3i position = chunk_position + Vector3i(x, y, z);
            Vector3i building_corner = Vector3i(UtilityFunctions::posmod(position.x, BLOCK_SIZE),
                                                UtilityFunctions::posmod(position.y, BLOCK_SIZE),
                                                UtilityFunctions::posmod(position.z, BLOCK_SIZE));
            int ground_level_grid = (position.y - ground_level) / BLOCK_SIZE;

            uint16_t building_seed = world->generator->seed;
            Random::scramble_rng_seed(&building_seed, Vector3i(building_corner.x, 0, building_corner.z));

            int building_stories = 4 + UtilityFunctions::randi() % 8;

            int block_type = 0;
            if (ground_level_grid == 0) {
                // bottom story
            } else if (ground_level_grid < 0) {
                if (position.y == ground_level) {
                    block_type = world->block_name_map["grass block"];
                } else {
                    block_type = world->block_name_map["dirt block"];
                }
            } else if (ground_level_grid > 0 && ground_level_grid < building_stories) {
                // stories
            } else if (ground_level_grid == building_stories) {
                // top stories
            }

            chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = block_type;
            chunk->water[Chunk::position_to_index(Vector3i(x, y, z))] = position.y <= water_level ? 255 : 0;
            
        }
    }
    }
}

int64_t TempleTownBiome::get_ground_level(Vector3i position, Vector3i biome_coordinate) {
    int64_t snapped_x = BLOCK_SIZE * (position.x / BLOCK_SIZE);
    int64_t snapped_z = BLOCK_SIZE * (position.z / BLOCK_SIZE);
    if (!is_cliff) {
        int64_t y = (int64_t) (96.0 * (0.425 + terrain_noise->get_noise_2d(snapped_x, snapped_z)));
        int64_t snapped_y = BLOCK_SIZE * (y / BLOCK_SIZE);
        return snapped_y;
    } else {
        float height = terrain_noise->get_noise_2d(snapped_x, snapped_z);
        int64_t y = (int64_t) (96.0 * (0.15 + height / 2 + UtilityFunctions::pow(height, 8) + UtilityFunctions::pow(height + 0.5, 4)));
        int64_t snapped_y = BLOCK_SIZE * (y / BLOCK_SIZE);
        return snapped_y;
    }
}