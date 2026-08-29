#include "../../include/structures/cube_temple.h"
#include "../../include/world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void CubeTemple::_bind_methods() {
    BIND_REF_ARRAY_PROPERTY(CubeTemple, Decoration, decorations);
}

DEFINE_CONSTRUCTORS(CubeTemple);

void CubeTemple::generate_chunk_data(Chunk* chunk, Vector3i chunk_position) {
    uint16_t rng_seed = world->generator->seed;

    for (int64_t y = 0; y < Chunk::CHUNK_SIZE_Y; y++) {
    for (int64_t z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
    for (int64_t x = 0; x < Chunk::CHUNK_SIZE_X; x++) {
        int64_t half_size = size / 2;
        Vector3i position = chunk_position + Vector3i(x, y, z);
        Vector3i local_position = position - center_position;
        Vector3i from_corner = local_position + Vector3(half_size, half_size, half_size);
        Random::scramble_rng_seed(&rng_seed, position);

        // In bounds of cube centered at heart_position with side length size
        if (-half_size <= local_position.y && local_position.y <= half_size - 16 &&
            -half_size <= local_position.x && local_position.x <= half_size &&
            -half_size <= local_position.z && local_position.z <= half_size) {

            bool x_edge = UtilityFunctions::absi(local_position.x) == half_size;
            bool y_edge = local_position.y == half_size - 16 || local_position.y == -half_size;
            bool z_edge = UtilityFunctions::absi(local_position.z) == half_size;

            int64_t a = from_corner.x % (size / 3);
            int64_t b = from_corner.z % (size / 3);

            // Center tower
            if (from_corner.x == half_size && from_corner.z == half_size && from_corner.y <= 64) {
                if (y == 64) {
                    chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = world->block_name_map["cutscene block"];
                } else {
                    chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = world->block_name_map["metal block 2"];
                }
            }

            // Gates and walls
            if (x_edge || y_edge || z_edge) {
                if (from_corner.y >= 1 && from_corner.y <= 12 && (z_edge && (a == 10 || a == 11 || a == 12) || x_edge && (b == 10 || b == 11 || b == 12))) {
                    chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = 0;
                } else {
                    chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = world->block_name_map["metal block 2"];
                }
            } else {
                chunk->water[Chunk::position_to_index(Vector3i(x, y, z))] = 0;
                chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = 0;
            }

            // Lake
            if (from_corner.y == 1 && from_corner.x >= 12 && from_corner.z >= 12 && from_corner.x < size - 12 && from_corner.z < size - 12) {
                bool x_edge_small = UtilityFunctions::absi(local_position.x) == half_size - 12;
                bool z_edge_small = UtilityFunctions::absi(local_position.z) == half_size - 12;
                if (x_edge_small || z_edge_small) {
                    chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = world->block_name_map["metal block 2"];
                } else {
                    chunk->water[Chunk::position_to_index(Vector3i(x, y, z))] = 255;
                }
            }
        }
    }
    }
    }


    for (int64_t i = 0; i < placed_decorations.size(); i++) {
        Ref<DecorationState> ds = placed_decorations[i];
        Ref<Decoration> d = ds->decoration;
        Generator::fill_decoration(world, chunk, chunk_position, d, ds, false);
    }

    for (int64_t y = 0; y < Chunk::CHUNK_SIZE_Y; y++) {
    for (int64_t z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
    for (int64_t x = 0; x < Chunk::CHUNK_SIZE_X; x++) {
        int64_t half_size = size / 2;
        Vector3i position = chunk_position + Vector3i(x, y, z);
        Vector3i local_position = position - center_position;
        Vector3i from_corner = local_position + Vector3(half_size, half_size, half_size);
        Random::scramble_rng_seed(&rng_seed, position);

        // In bounds of cube centered at heart_position with side length size
        if (-half_size <= local_position.y && local_position.y <= half_size &&
            -half_size <= local_position.x && local_position.x <= half_size &&
            -half_size <= local_position.z && local_position.z <= half_size) {

            int64_t other_size = 13;
            if (-other_size <= local_position.y && local_position.y <= other_size &&
                -other_size <= local_position.x && local_position.x <= other_size &&
                -other_size <= local_position.z && local_position.z <= other_size &&
                local_position.x % 2 == 0 && local_position.z % 2 == 0) {
                chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = world->block_name_map["metal block 2"];
            }

            if (from_corner.x == half_size && from_corner.z == half_size && from_corner.y <= 38) {
                if (from_corner.y == 38) {
                    chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = world->block_name_map["cutscene block"];
                } else {
                    chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = world->block_name_map["metal block 2"];
                }
            }
        }
    }
    }
    }
}

void CubeTemple::initialize() {
    uint16_t rng_seed = world->generator->seed;

    Random::scramble_rng_seed(&rng_seed, heart_position);

    if (MIN_SIZE == MAX_SIZE) {
        size = MIN_SIZE;
    } else {
        size = MIN_SIZE + Random::randi(rng_seed) % (MAX_SIZE - MIN_SIZE);
    }

    for (int i = 0; i < 24; i++) {
        int x = Random::randi(rng_seed) % (size - 32) - (size - 32) / 2;
        Random::scramble_rng_seed(&rng_seed, heart_position + Vector3(x * 2, x + 15, x * 2131));
        int y = Random::randi(rng_seed) % (size - 32) - (size - 32) / 2;
        Random::scramble_rng_seed(&rng_seed, heart_position + Vector3(y * 12 + x, x + 7 - 2 * y, x * 11 + y));
        int z = Random::randi(rng_seed) % (size - 32) - (size - 32) / 2;
        Random::scramble_rng_seed(&rng_seed, root_position + Vector3(x * 12 + x, z + 7 - 2 * y * x, y * x * 11 + y));

        // Skip decorations too close to the center pillar
        if (UtilityFunctions::absi(x) <= 4 || UtilityFunctions::absi(z) <= 4) {
            continue;
        }

        // Oops, y is useless, but I kept it for the RNG
        Vector3i position = center_position + Vector3i(x, 0, z);
        position.y = center_position.y - size / 2 + 1;

        Random::scramble_rng_seed(&rng_seed, position);
        uint8_t decoration_index = Random::randi(rng_seed) % decorations.size();

        // Place the decoration
        Ref<DecorationState> d = memnew(DecorationState);
        d->decoration = decorations[decoration_index];
        d->position = position;

        Random::scramble_rng_seed(&rng_seed, position * position - 241 * Vector3i(y, z, x));
        int64_t direction = Random::randi(rng_seed) % 4;

        if (direction == 0) {
            d->direction = DecorationState::North;
        } else if (direction == 1) {
            d->direction = DecorationState::East;
        } else if (direction == 2) {
            d->direction = DecorationState::South;
        } else if (direction == 3) {
            d->direction = DecorationState::West;
        }

        placed_decorations.append(d);
    }

    int64_t half_size = size / 2;
    Vector3i corner_position = center_position - Vector3i(half_size, half_size, half_size);
    cutscene_block_position = corner_position + Vector3i(half_size, 38, half_size);
}

bool CubeTemple::has_cutscene_block() {
    return true;
}

Vector3i CubeTemple::get_cutscene_block_position() {
    return cutscene_block_position;
}

bool CubeTemple::is_within_structure(Vector3i position) {
    Vector3i local_position = position - center_position;
    int64_t half_size = size / 2;

    // In bounds of cube centered at heart_position with side length size
    return -half_size <= local_position.y && local_position.y <= half_size &&
           -half_size <= local_position.x && local_position.x <= half_size &&
           -half_size <= local_position.z && local_position.z <= half_size;
}

DEFINE_PROPERTY_GETTER_SETTER(CubeTemple, TypedArray<Decoration>, decorations);
