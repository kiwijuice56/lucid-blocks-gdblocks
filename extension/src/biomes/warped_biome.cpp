#include "../../include/biomes/warped_biome.h"
#include "../../include/world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void WarpedBiome::_bind_methods() {
    BIND_PROPERTY(WarpedBiome, Vector3i, size);
    BIND_PROPERTY(WarpedBiome, double, frequency);
    BIND_PROPERTY(WarpedBiome, double, scale);
    BIND_PROPERTY(WarpedBiome, double, randomness);
}

DEFINE_CONSTRUCTORS(WarpedBiome);

DEFINE_PROPERTY_GETTER_SETTER(WarpedBiome, Vector3i, size);
DEFINE_PROPERTY_GETTER_SETTER(WarpedBiome, double, frequency);
DEFINE_PROPERTY_GETTER_SETTER(WarpedBiome, double, scale);
DEFINE_PROPERTY_GETTER_SETTER(WarpedBiome, double, randomness);

void WarpedBiome::initialize() {
    Biome::initialize();
}

void WarpedBiome::generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate) {
    if (random_decorations.size() == 0) {
        return;
    }

    for (int64_t y = 0; y < Chunk::CHUNK_SIZE_Y; y++) {
    for (int64_t z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
    for (int64_t x = 0; x < Chunk::CHUNK_SIZE_X; x++) {
        int64_t water_level = get_water_level(chunk_position + Vector3i(x, 0, z), biome_coordinate);
        Vector3i position = chunk_position + Vector3i(x, y, z);
        if (block_locked(chunk, Vector3i(x, y, z))) {
            continue;
        }

        int ax = int(position.x) % size.x;
        int ay = int(position.y) % size.y;
        int az = int(position.z) % size.z;

        if (ax < 0) ax += size.x;
        if (ay < 0) ay += size.y;
        if (az < 0) az += size.z;

        uint16_t rng_seed = world->generator->seed;
        Vector3i snapped_position = position - Vector3i(ax, ay, az);
        Random::scramble_rng_seed(&rng_seed, snapped_position);
        Ref<Decoration> decoration = random_decorations[Random::randi(rng_seed) % random_decorations.size()];

        int block_index = 0;

        // Snap to local space
        int xi = position.x % size.x;
        int yi = position.y % size.y;
        int zi = position.z % size.z;
        if (xi < 0) xi += size.x;
        if (yi < 0) yi += size.y;
        if (zi < 0) zi += size.z;

        // Warp transformations
        double wx = xi * (1.0 + 0.25 * UtilityFunctions::cos(randomness * snapped_position.y * snapped_position.z + frequency * xi));
        double wy = yi * (1.0 + 0.25 * UtilityFunctions::sin(randomness * snapped_position.z * snapped_position.x + frequency * yi));
        double wz = zi * (1.0 + 0.25 * UtilityFunctions::cos(randomness * snapped_position.x * snapped_position.y + frequency * zi));

        // Recenter
        wx -= size.x / 2;
        wy -= size.y / 2;
        wz -= size.z / 2;

        double rx = wx;
        double ry = wy;
        double rz = wz;

        // Rasterization 
        xi = (int) (scale * rx);
        yi = (int) (scale * ry);
        zi = (int) (scale * rz);

        if (xi < 0 || xi >= decoration->size.x || yi < 0 || yi >= decoration->size.y || zi < 0 || zi >= decoration->size.z) {
            block_index = 0;
        } else {
            block_index = world->block_id_to_index_map[decoration->blocks[uint64_t(xi) + uint64_t(zi) * decoration->size.x + uint64_t(yi) * decoration->size.x * decoration->size.z]];
        }

        chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = block_index;
        chunk->water[Chunk::position_to_index(Vector3i(x, y, z))] = 0;
    }
    }
    }
}

void WarpedBiome::generate_decorations(Vector3i chunk_position, Vector3i biome_coordinate) { }
