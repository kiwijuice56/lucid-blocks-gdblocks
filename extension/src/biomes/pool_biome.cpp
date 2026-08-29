#include "../../include/biomes/pool_biome.h"
#include "../../include/world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void PoolBiome::_bind_methods() {
    BIND_METHOD_NO_PARAMETERS(PoolBiome, get_decorations);
    BIND_METHOD(PoolBiome, set_decorations, "new_decorations");
    ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "decoration", PROPERTY_HINT_RESOURCE_TYPE, "Decoration"), "set_decorations", "get_decorations");
}

DEFINE_CONSTRUCTORS(PoolBiome);

void PoolBiome::initialize() {
    Biome::initialize();
    extra_void_noise = world->generator->noise[9];
}

void PoolBiome::generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate) {
    uint16_t rng_seed = world->generator->seed;
    Random::scramble_rng_seed(&rng_seed, chunk_position * Vector3i(chunk_position.x * chunk_position.z * 7, 13, 21));

    bool empty = extra_void_noise->get_noise_2d(chunk_position.x, chunk_position.z) > -0.038;

    Ref<Decoration> decoration = decorations[Random::randi(rng_seed) % decorations.size()];
    double r = Random::randf(rng_seed);
    DecorationState::Direction direction = r < 0.25 ? DecorationState::North : (r < 0.5 ? DecorationState::East : (r < 0.75 ? DecorationState::West : DecorationState::South));

    Random::scramble_rng_seed(&rng_seed, chunk_position * Vector3i(chunk_position.x * chunk_position.x * 7, 31, chunk_position.z * 21));
    bool window = Random::randf(rng_seed) < 0.5;

    for (int64_t y = 0; y < Chunk::CHUNK_SIZE_Y; y++) {
    for (int64_t z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
    for (int64_t x = 0; x < Chunk::CHUNK_SIZE_X; x++) {
        Vector3i position = chunk_position + Vector3i(x, y, z);
        int64_t water_level = get_water_level(chunk_position + Vector3i(x, 0, z), biome_coordinate);
        Random::scramble_rng_seed(&rng_seed, position);
        if (block_locked(chunk, Vector3i(x, y, z))) {
            continue;
        }

        int32_t block_index = 0;

        if (empty) {
            if (position.y <= water_level - 32) {
                block_index = world->block_name_map["pool tile block"];
            }
        } else if (chunk_position.y == water_level) {
            if (y >= 8) {
                block_index = 0;
            } else {
                // Correct the orientation
                int64_t rx = (direction == DecorationState::West) ? (16 - x - 1) : x;
                int64_t ry = y;
                int64_t rz = (direction == DecorationState::East || direction == DecorationState::South) ? (16 - z - 1) : z;
                if (direction == DecorationState::East || direction == DecorationState::West) {
                    int64_t t = rx;
                    rx = rz;
                    rz = t;
                }

                int64_t xi = (chunk_position.x + rx) % decoration->size.x;
                int64_t yi = (chunk_position.y + ry) % decoration->size.y;
                int64_t zi = (chunk_position.z + rz) % decoration->size.z;

                if (xi < 0) xi += decoration->size.x;
                if (yi < 0) yi += decoration->size.y;
                if (zi < 0) zi += decoration->size.z;

                block_index = world->block_id_to_index_map[decoration->blocks[uint64_t(xi) + uint64_t(zi) * decoration->size.x + uint64_t(yi) * decoration->size.x * decoration->size.z]];

                if (window && y == 7 && block_index == 0) {
                    block_index = world->block_name_map["mirage block"];
                }
            }
        } else if (chunk_position.y < water_level) {
            if (is_on_vertical_border(chunk, position, Vector3i(x, y, z)) || x == 0 || x == Chunk::CHUNK_SIZE_X - 1 || z == 0 || z == Chunk::CHUNK_SIZE_Z - 1) {
                block_index = world->block_name_map["pool tile block"];
            } else {
                block_index = 0;
            }
        }

        chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = block_index;
        chunk->water[Chunk::position_to_index(Vector3i(x, y, z))] = position.y <= water_level ? 255 : 0;
    }
    }
    }
}


void PoolBiome::generate_decorations(Vector3i chunk_position, Vector3i biome_coordinate) { }

DEFINE_PROPERTY_GETTER_SETTER(PoolBiome, TypedArray<Decoration>, decorations);
