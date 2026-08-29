#include "../../include/biomes/water_cube_biome.h"
#include "../../include/world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void WaterCubeBiome::_bind_methods() {
    BIND_PROPERTY(WaterCubeBiome, int, grid_size);
    BIND_PROPERTY(WaterCubeBiome, int, cube_size);
}

DEFINE_CONSTRUCTORS(WaterCubeBiome);

DEFINE_PROPERTY_GETTER_SETTER(WaterCubeBiome, int, grid_size);
DEFINE_PROPERTY_GETTER_SETTER(WaterCubeBiome, int, cube_size);

void WaterCubeBiome::generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate)  {
    uint16_t rng_seed = world->generator->seed;

    for (int64_t y = 0; y < Chunk::CHUNK_SIZE_Y; y++) {
    for (int64_t z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
    for (int64_t x = 0; x < Chunk::CHUNK_SIZE_X; x++) {
        int64_t water_level = get_water_level(chunk_position + Vector3i(x, 0, z), biome_coordinate);
        Vector3i position = chunk_position + Vector3i(x, y, z);

        if (block_locked(chunk, Vector3i(x, y, z))) {
            continue;
        }

        int mx = position.x % grid_size;
        int my = position.y % grid_size;
        int mz = position.z % grid_size;

        if (mx < 0) mx += grid_size;
        if (my < 0) my += grid_size;
        if (mz < 0) mz += grid_size;

        int32_t block_type = 0;

        bool in_cube = false;

        if (mx <= cube_size && my <= cube_size && mz <= cube_size || UtilityFunctions::posmod(biome_coordinate.y, world->generator->BIOME_MAP_SIZE_Y) == 6 && position.y <= water_level) {
            in_cube = true;
            chunk->water[Chunk::position_to_index(Vector3i(x, y, z))] = 255;
        } else {
            chunk->water[Chunk::position_to_index(Vector3i(x, y, z))] = 0;
        }

        if (mx <= cube_size && my <= cube_size && mz <= cube_size) {
            rng_seed = world->generator->seed;
            Random::scramble_rng_seed(&rng_seed, Vector3i(position.x / grid_size, position.y / grid_size, position.z / grid_size));
            int array_index = Random::randi(rng_seed) % building_blocks.size();

            if (is_on_vertical_border(chunk, position, Vector3i(x, y, z)) || is_on_horizontal_border(chunk, position, Vector3i(x, y, z)) || mx == cube_size || my == cube_size || mz == cube_size || mx == 0 || my == 0 || mz == 0) {
                block_type = Ref<Block>(building_blocks[array_index])->index;
            }
        }

        if (height == 6 && is_on_vertical_border(chunk, position, Vector3i(x, y, z))) {
            block_type = world->block_name_map["simblock"];
        }
        
        chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = block_type;
        if (height == 6 && position.y <= water_level) chunk->water[Chunk::position_to_index(Vector3i(x, y, z))] = 255;
        
    }
    }
    }
}