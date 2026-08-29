#include "../../include/biomes/julia_biome.h"
#include "../../include/world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

DEFINE_CONSTRUCTORS_AND_BINDINGS(JuliaBiome);

void JuliaBiome::initialize() {
    Biome::initialize();
    julia_noise = world->generator->noise[22];
}

void JuliaBiome::generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate)  {
    uint16_t rng_seed = world->generator->seed;

    for (int64_t y = 0; y < Chunk::CHUNK_SIZE_Y; y++) {
    for (int64_t z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
    for (int64_t x = 0; x < Chunk::CHUNK_SIZE_X; x++) {
        int64_t water_level = get_water_level(chunk_position + Vector3i(x, 0, z), biome_coordinate);
        Vector3i local_position = Vector3i(x, y, z);
        Vector3i position = chunk_position + local_position;

        Random::scramble_rng_seed(&rng_seed, position);

        if (block_locked(chunk, Vector3i(x, y, z))) {
            continue;
        }

        int block_type = 0;

        if (gyroid_density(position) > 0.0 || is_on_vertical_border(chunk, position, local_position)) {
            block_type = Ref<Block>(building_blocks[0])->index;
        }

        chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = block_type;
        chunk->water[Chunk::position_to_index(Vector3i(x, y, z))] = position.y <= water_level ? 255 : 0;
    }
    }
    }
}

void JuliaBiome::generate_decorations(Vector3i chunk_position, Vector3i biome_coordinate) { }

inline double JuliaBiome::noise3D(const Vector3 &p) {
    return UtilityFunctions::sin(p.x * 0.5 + UtilityFunctions::cos(p.y * 0.5) + p.z * 0.5);
}

inline double JuliaBiome::gyroid(const Vector3 &p) {
    return    UtilityFunctions::sin(p.x) * UtilityFunctions::cos(p.y)
            + UtilityFunctions::sin(p.y) * UtilityFunctions::cos(p.z)
            + UtilityFunctions::sin(p.z) * UtilityFunctions::cos(p.x);
}

inline double JuliaBiome::gyroid_density(const Vector3i &block_pos) {
    Vector3 p = Vector3(block_pos) * 0.05;

    double warp_x = noise3D(Vector3(p.y, p.z, p.x)) * 0.3;
    double warp_y = noise3D(Vector3(p.z, p.x, p.y)) * 0.3;
    double warp_z = noise3D(Vector3(p.x, p.z, p.y)) * 0.3;

    Vector3 warped_p = p + Vector3(warp_x, warp_y, warp_z);

    double d = gyroid(warped_p);

    d += julia_noise->get_noise_3dv(block_pos);

    return d;
}