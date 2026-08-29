#include "../../include/biomes/brutal_biome.h"
#include "../../include/world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

DEFINE_CONSTRUCTORS_AND_BINDINGS(BrutalBiome);

void BrutalBiome::generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate) {
    uint16_t rng_seed = world->generator->seed;

    const int SIZE = 4;

    int64_t ground_level = get_ground_level(Vector3(chunk_position.x, 0, chunk_position.z), biome_coordinate);
    int64_t water_level = get_water_level(Vector3(chunk_position.x, 0, chunk_position.z), biome_coordinate);
    int64_t xi = (chunk_position.x / Chunk::CHUNK_SIZE_X) % SIZE;
    int64_t yi = (chunk_position.y - ground_level) / Chunk::CHUNK_SIZE_Y;
    int64_t zi = (chunk_position.z / Chunk::CHUNK_SIZE_Z) % SIZE;

    if (xi < 0) xi += SIZE;
    if (zi < 0) zi += SIZE;

    int type = 0;

    // Y is ignored
    Vector3i corner_position = chunk_position - Vector3i(Math::posmod(chunk_position.x, Chunk::CHUNK_SIZE_X * SIZE), 0, Math::posmod(chunk_position.z, Chunk::CHUNK_SIZE_Z * SIZE));
    corner_position.y = 0;

    Random::scramble_rng_seed(&rng_seed, corner_position);

    if (yi >= 0 && yi < 8) {
        if (Random::randf(rng_seed) < 0.5 || xi == 1 && zi == 1 || xi == 3 || zi == 3) {
            type = 0;
        } else {
            Random::scramble_rng_seed(&rng_seed, chunk_position);
            type = Random::randf(rng_seed) < 0.05 ? 2 : 1;
        }
    }

    Random::scramble_rng_seed(&rng_seed, chunk_position * Vector3(chunk_position.z + chunk_position.x, chunk_position.z, chunk_position.x));

    int32_t box_id = Random::randf(rng_seed) < 0.9 ? world->block_name_map["cardboard box"] : world->block_name_map["letter cube"];

    for (int64_t y = 0; y < Chunk::CHUNK_SIZE_Y; y++) {
    for (int64_t z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
    for (int64_t x = 0; x < Chunk::CHUNK_SIZE_X; x++) {
        Random::scramble_rng_seed(&rng_seed, chunk_position + Vector3i(x, y, z));
        Vector3i position = Vector3i(x, y, z);

        if (block_locked(chunk, position)) {
            continue;
        }

        Vector3i global_position = chunk_position + position;
        int32_t block_type = 0;

        // Grass room
        if (type == 0) {
            if (global_position.y == ground_level) {
                block_type = world->block_name_map["grass block"];
            } else if (global_position.y == ground_level + 1 && Random::randf(rng_seed) < 0.015) {
                block_type = world->block_name_map["grass foliage"];
            } else if (global_position.y < ground_level) {
                block_type = world->block_name_map["dirt block"];
            } else {
                block_type = 0;
            }
        }

        // Building
        if (type >= 1) {
            // Roof/floor
            if (y == 0 || y == Chunk::CHUNK_SIZE_Y - 1) {
                block_type = world->block_name_map["limestone tile block"];
            // Outermost walls
            } else if (Structure::on_chunk_border(position, 0)) {
                bool e_wall = x == 0 &&                       xi > 0 && zi != 1;
                bool w_wall = x == Chunk::CHUNK_SIZE_X - 1 && xi < 2 && zi != 1;
                bool s_wall = z == 0 &&                       zi > 0 && xi != 1;
                bool n_wall = z == Chunk::CHUNK_SIZE_Z - 1 && zi < 2 && xi != 1;

                // Outside ridges
                if ((y % 4 == 0 || y % 4 == 3) && (Structure::on_chunk_corner(position, 0) || !(e_wall || w_wall || s_wall || n_wall))) {
                    block_type = world->block_name_map["limestone tile block"];
                }

                // Ceiling and floor corners connecting buildings
                if ((y == 1 || y == Chunk::CHUNK_SIZE_Y - 2) && !Structure::on_chunk_corner(position,0) && (e_wall || n_wall || s_wall || w_wall)) {
                    block_type =  world->block_name_map["concrete block"];
                }

                // Internal corners
                if ((z == 1 || z == Chunk::CHUNK_SIZE_Z - 2) && e_wall ||
                    (z == 1 || z == Chunk::CHUNK_SIZE_Z - 2) && w_wall ||
                    (x == 1 || x == Chunk::CHUNK_SIZE_X - 2) && s_wall ||
                    (x == 1 || x == Chunk::CHUNK_SIZE_X - 2) && n_wall) {
                    block_type =  world->block_name_map["concrete block"];
                }
            // Innermost Walls
            } else if (Structure::on_chunk_border(position, 1)) {
                bool e_wall = x == 1 &&                       xi > 0 && zi != 1;
                bool w_wall = x == Chunk::CHUNK_SIZE_X - 2 && xi < 2 && zi != 1;
                bool s_wall = z == 1 &&                       zi > 0 && xi != 1;
                bool n_wall = z == Chunk::CHUNK_SIZE_Z - 2 && zi < 2 && xi != 1;

                bool on_ceil = y == 1 || y == Chunk::CHUNK_SIZE_Y - 2;

                if ((y % 16 == 5 || y % 16 == 6 || y % 16 == 9 || y % 16 == 10) && !n_wall && !e_wall && !s_wall && !w_wall && ((x == Chunk::CHUNK_SIZE_X - 2 || x == 1) && (z % 5 == 2 || z % 5 == 3) || (z == Chunk::CHUNK_SIZE_Z - 2 || z == 1) && (x % 5 == 2 || x % 5 == 3))) {
                    block_type = world->block_name_map["steel window block"];
                } else if (on_ceil || Structure::on_chunk_corner(position, 1)) {
                    block_type = world->block_name_map["concrete block"];
                } else if (e_wall || w_wall || s_wall || n_wall) {
                    block_type = 0;
                } else  {
                    block_type = world->block_name_map["concrete block"];
                }
            }
        }

        // Loot room
        if (type == 2) {
            // Place first level boxes
            if (!Structure::on_chunk_border(position, 0) && !Structure::on_chunk_border(position, 1) && y == 2 && Random::randf(rng_seed) < 0.10) {
                block_type = box_id;
            }

            // Place higher level boxes
            if (y >= 3) {
                int32_t under_id = chunk->blocks[Chunk::position_to_index(Vector3i(x, y - 1, z))];
                if (under_id == box_id && Random::randf(rng_seed) < 0.35) {
                    block_type = box_id;
                }
            }

        }

        // Pillar room
        if (type == 3) {
            if (x >= 5 && x <= 11 && z >= 5 && z <= 11) {
                if (x % 2 == 0 && z % 2 == 0) {
                    block_type = world->block_name_map["concrete block"];
                }
            }
        }

        // Stairway room room
        if (type == 4) {
            if (y >= 2) {
                bool on_edge = false;
                int64_t step = 0;
                if (x == 6) {
                    step = z - 6;
                    on_edge = z >= 6 && z <= 10;
                }
                if (z == 10) {
                    step = 4 + (x - 6);
                    on_edge = x >= 6 && x <= 10;
                }
                if (x == 10) {
                    step = 12 - (z - 6);
                    on_edge = z >= 6 && z <= 10;
                }
                if (z == 6) {
                    step = 16 - (x - 6);
                    on_edge = x >= 6 && x <= 10;
                }
                if (on_edge) {
                    if ((y - 2) % 16 <= step % 16) {
                        block_type = (y - 2) % 16 == step % 16 ? world->block_name_map["concrete block"] : world->block_name_map["limestone tile block"] ;
                    } else {
                        block_type = 0;
                    }
                }
                if (y >= 14 && (x == 6 || z == 10 && x < 10)) {
                    block_type = world->block_name_map["concrete block"];
                }
                if (x > 6 && x < 10 && z > 6 && z < 10) {
                    block_type = world->block_name_map["limestone tile block"];
                }
            }
        }

        chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = block_type;
        chunk->water[Chunk::position_to_index(Vector3i(x, y, z))] = global_position.y <= water_level ? 255 : 0;
    }
    }
    }
}

int64_t BrutalBiome::get_ground_level(Vector3i position, Vector3i biome_coordinate) {
    int biome_height_offset = world->generator->Y_PER_BIOME_Y * (biome_coordinate.y - 6);
    return biome_height_offset + 32;
}