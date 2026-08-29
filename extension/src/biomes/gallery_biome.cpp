#include "../../include/biomes/gallery_biome.h"
#include "../../include/world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

DEFINE_CONSTRUCTORS_AND_BINDINGS(GalleryBiome);

void GalleryBiome::generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate)  {
    uint16_t rng_seed = world->generator->seed;
    uint16_t base_seed = rng_seed;

    // respawn blocks because they add a spawn point
    int respawn_block_index = world->block_name_map["respawn block"];
    int heaven_block_index = world->block_name_map["heaven block"];
    int lucy_block_index = world->block_name_map["lucy block"];

    for (int64_t y = 0; y < Chunk::CHUNK_SIZE_Y; y++) {
    for (int64_t z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
    for (int64_t x = 0; x < Chunk::CHUNK_SIZE_X; x++) {
        int64_t water_level = get_water_level(chunk_position + Vector3i(x, 0, z), biome_coordinate);
        Vector3i position = chunk_position + Vector3i(x, y, z);

        Random::scramble_rng_seed(&rng_seed, position);

        if (block_locked(chunk, Vector3i(x, y, z))) {
            continue;
        }

        int64_t block_type = 0;
        if (position.x % 8 == 0 && position.y % 8 == 0 && position.z % 8 == 0) {
            int64_t index = Random::randi(rng_seed) % world->block_types.size();
            if (index == respawn_block_index || index == heaven_block_index || index == lucy_block_index || chunk->is_block_foliage[index] || chunk->is_block_internal[index]) {
                block_type = 0;
            } else {
                block_type = index;
            }
        }

        bool is_overworld = UtilityFunctions::posmod(biome_coordinate.y, world->generator->BIOME_MAP_SIZE_Y) == 6;

        if (is_overworld && is_on_vertical_border(chunk, position, Vector3i(x, y, z))) {
            block_type = world->block_name_map["simblock"];
        }

        chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = block_type;
        chunk->water[Chunk::position_to_index(Vector3i(x, y, z))] = is_overworld && position.y <= water_level ? 255 : 0;
    }
    }
    }
}
