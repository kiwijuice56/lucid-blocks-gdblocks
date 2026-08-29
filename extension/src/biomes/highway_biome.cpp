#include "../../include/biomes/highway_biome.h"
#include "../../include/world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

DEFINE_CONSTRUCTORS_AND_BINDINGS(HighwayBiome);

void HighwayBiome::initialize() {
    Biome::initialize();
    road_noise = world->generator->noise[25];
}

void HighwayBiome::generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate) {
    uint16_t rng_seed = world->generator->seed;
    Random::scramble_rng_seed(&rng_seed, chunk_position);

    for (int64_t z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
    for (int64_t x = 0; x < Chunk::CHUNK_SIZE_X; x++) {
        int64_t ground_level = get_ground_level(chunk_position + Vector3i(x, 0, z), biome_coordinate);
        int64_t water_level = get_water_level(chunk_position + Vector3i(x, 0, z), biome_coordinate);

        for (int64_t y = 0; y < Chunk::CHUNK_SIZE_Y; y++) {
            Vector3i local_position = Vector3i(x, y, z);
            Vector3i position = chunk_position + local_position;
            Random::scramble_rng_seed(&rng_seed, position);

            if (block_locked(chunk, local_position)) {
                continue;
            }

            int32_t block_type = 0;

            if (position.y > ground_level + 1) {
                // Air
            } else if (position.y == ground_level + 1) {
                // Foliage
                if (position.y > water_level && Random::randf(rng_seed) < 0.1 && is_foliage_safe(chunk, position, local_position) && !is_road(position - Vector3i(0, 1, 0), ground_level)) {
                    block_type = world->block_name_map["bright grass foliage"];
                }
            } else if (position.y == ground_level) {
                // Top level of ground
                block_type = is_road(position, ground_level) ? world->block_name_map["asphalt block"] : world->block_name_map["bright grass block"];
            } else if (position.y < ground_level && position.y > ground_level - dirt_height) {
                // Second layer of ground
                block_type = world->block_name_map["dirt block"];
            } else {
                // Underground
                block_type = world->block_name_map["stone block"];
            }

            // Biome floor
            if (is_on_vertical_border(chunk, position, local_position)) {
                block_type = world->block_name_map["stone block"];
            }

            chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = block_type;
            chunk->water[Chunk::position_to_index(Vector3i(x, y, z))] = position.y <= water_level ? 255 : 0;
        }
    }
    }
}

void HighwayBiome::generate_decorations(Vector3i chunk_position, Vector3i biome_coordinate) { }

int64_t HighwayBiome::get_ground_level(Vector3i position, Vector3i biome_coordinate) {
    int biome_height_offset = world->generator->Y_PER_BIOME_Y * (biome_coordinate.y - 6);
    return biome_height_offset + 32 + 40 * terrain_noise->get_noise_2d(position.x, position.z);
}

Vector2 HighwayBiome::road_direction(double x, double z) {
    float n = road_noise->get_noise_2d(x, z); 

    // bias heavily toward cardinal axes
    double angle = UtilityFunctions::roundf(n * 2.0) * (3.14159 / 2.0);
    float wiggle = road_noise->get_noise_2d(x * 0.02, z * 0.02) * 0.2; 

    return Vector2(cos(angle + wiggle), sin(angle + wiggle)).normalized();
}

bool HighwayBiome::is_road(Vector3i position, int ground_level) {
    if (position.y != ground_level) return false;

    const double road_width = 7.0;
    const double spacing = 110.0;

    Vector2 dir = road_direction(position.x, position.z);
    Vector2 perp = Vector2(-dir.y, dir.x); // perpendicular axis

    // project position into local frame
    float u = position.x * perp.x + position.z * perp.y;

    return UtilityFunctions::fmod(UtilityFunctions::absf(u), spacing) < road_width;
}