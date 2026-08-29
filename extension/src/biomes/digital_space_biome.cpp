#include "../../include/biomes/digital_space_biome.h"
#include "../../include/world.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void DigitalSpaceBiome::_bind_methods() {
    BIND_PROPERTY(DigitalSpaceBiome, int, grid_size);
}

DEFINE_CONSTRUCTORS(DigitalSpaceBiome);

DEFINE_PROPERTY_GETTER_SETTER(DigitalSpaceBiome, int, grid_size);

void DigitalSpaceBiome::initialize() {
    Biome::initialize();
    ground_noise = world->generator->noise[32];
    Ref<Block> main_block = building_blocks[0];
    main_block_index = main_block->index;
}

void DigitalSpaceBiome::generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate) {
    uint16_t rng_seed = world->generator->seed;
    for (int64_t z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
    for (int64_t x = 0; x < Chunk::CHUNK_SIZE_X; x++) {
        float height = terrain_noise->get_noise_2d(x + chunk_position.x, z + chunk_position.z);
        int64_t water_level = get_water_level(chunk_position + Vector3i(x, 0, z), biome_coordinate);
        for (int64_t y = 0; y < Chunk::CHUNK_SIZE_Y; y++) {
            Random::scramble_rng_seed(&rng_seed, chunk_position + Vector3i(x, y, z));
            if (block_locked(chunk, Vector3i(x, y, z))) {
                continue;
            }
            int64_t block_type = get_block_at(chunk, chunk_position + Vector3i(x, y, z), biome_coordinate, 0);
            chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = block_type;
            chunk->water[Chunk::position_to_index(Vector3i(x, y, z))] = 0;
        }
    }
    }
}

int64_t DigitalSpaceBiome::get_block_at(Chunk* chunk, Vector3i position, Vector3i biome_coordinate, int64_t ground_level) {
    // Snap position to grid for noise sampling
    Vector3i snapped_position = Vector3i(
        (position.x / grid_size) * grid_size,
        (position.y / grid_size) * grid_size,
        (position.z / grid_size) * grid_size
    );
    
    // Sample noise at the snapped position
    double terrain = ground_noise->get_noise_3d(snapped_position.x, snapped_position.y, snapped_position.z);
    
    if (terrain < 0.0) {
        int biome_height_offset = world->generator->Y_PER_BIOME_Y * (biome_coordinate.y - 6);
        float close_to_border_penalty_1 = UtilityFunctions::clampf((40 + biome_height_offset - snapped_position.y) / 32.0f, 0.0f, 1.0f);
        float close_to_border_penalty_2 = UtilityFunctions::clampf((snapped_position.y - (biome_height_offset + world->generator->Y_PER_BIOME_Y - 76) ) / 48.0f, 0.0f, 1.0f);
        
        if (terrain + close_to_border_penalty_1 + close_to_border_penalty_2 > 0.0) {
            return 0;
        }
        
        // Only place block if position is on a grid line (corner of grid cube)
        bool on_grid_x = (position.x % grid_size) == 0;
        bool on_grid_y = (position.y % grid_size) == 0;
        bool on_grid_z = (position.z % grid_size) == 0;
        
        if (on_grid_x && on_grid_y && on_grid_z) {
            return main_block_index;
        } else {
            return 0;
        }
    } else {
        return 0;
    }
}

int64_t DigitalSpaceBiome::get_ground_level(Vector3i position, Vector3i biome_coordinate) {
    return GROUND_LEVEL_INVALID;
}