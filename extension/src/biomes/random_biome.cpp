#include "../../include/biomes/random_biome.h"
#include "../../include/biomes/ambiguous_biome.h"
#include "../../include/biomes/highway_biome.h"
#include "../../include/world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void RandomBiome::_bind_methods() {
    BIND_PROPERTY(RandomBiome, bool, is_aether);
}

DEFINE_CONSTRUCTORS(RandomBiome);

void RandomBiome::initialize() {
    Biome::initialize();
    random_noise = world->generator->noise[3];
    cloud_noise = world->generator->noise[30];

    respawn_block_index = world->block_name_map["respawn block"];
    heaven_block_index = world->block_name_map["heaven block"];
    alma_block_index = world->block_name_map["alma block"];
    lavender_block_index = world->block_name_map["lavender block"];
    dirt_block_index = world->block_name_map["dirt block"];
    gold_block_index = world->block_name_map["gold block"];
}

uint16_t RandomBiome::get_biome_seed(Vector3i position, Vector3i biome_coordinate) {
    position.y = 0;

    float cell_scale_xz = 0.5; 
    float voronoi_value = random_noise->get_noise_2d(position.x * cell_scale_xz + biome_coordinate.y * 16, position.z * cell_scale_xz + biome_coordinate.y * 16);
    uint16_t seed = UtilityFunctions::floori((voronoi_value + 1.0) * 10000.0);
    Random::scramble_rng_seed(&seed, Vector3(1, 1, 1));
    
    return seed;
}

RandomBiome::RandomBiomeIdentity RandomBiome::randomize(Chunk* chunk, Vector3i position, Vector3i biome_coordinate) {
    seed = get_biome_seed(position, biome_coordinate);
    uint16_t modified_seed = seed;

	RandomBiomeIdentity r;

    r.terrain_type = Random::randi(modified_seed) % 4; // terrain types: regular, stepped, flat-like, flat
    Random::scramble_rng_seed(&modified_seed, Vector3(7, 11, modified_seed));

	r.grid_size = 5 + Random::randi(modified_seed) % 9;
    Random::scramble_rng_seed(&modified_seed, Vector3(3, -7, 23));

	r.offset_size = Random::randi(modified_seed) % 9;
    Random::scramble_rng_seed(&modified_seed, Vector3(-3, 0, 129));

	r.height_multiplier = 0.75 + 0.5 * Random::randf(modified_seed);
    Random::scramble_rng_seed(&modified_seed, Vector3(-319, modified_seed, 1291));

	r.frequency_multiplier = 0.75 + 0.5 * Random::randf(modified_seed);
    Random::scramble_rng_seed(&modified_seed, Vector3(-modified_seed, 983, 17));

	r.ground_type = Random::randi(modified_seed) % 1; // ground types: stacked
    Random::scramble_rng_seed(&modified_seed, Vector3(239, 2, -18));

    r.corrupt = Random::randf(modified_seed) < 0.06f;
    Random::scramble_rng_seed(&modified_seed, Vector3(74, 8, modified_seed));

    r.grid = Random::randf(modified_seed) < 0.25f;
    Random::scramble_rng_seed(&modified_seed, Vector3(74, 8, modified_seed));

    r.checkerboard = Random::randf(modified_seed) < 0.25f;
    Random::scramble_rng_seed(&modified_seed, Vector3(74, 8, modified_seed));

    r.cliff = Random::randf(modified_seed) < 0.1f;
    Random::scramble_rng_seed(&modified_seed, Vector3(74, 8, 124));

    r.ground_type_1 = get_random_block(chunk, modified_seed, false, false, false, false);
    Random::scramble_rng_seed(&modified_seed, Vector3(-12, 983, 17));

    r.ground_type_2 = get_random_block(chunk, modified_seed, false, false, false, false);
    Random::scramble_rng_seed(&modified_seed, Vector3(-12, modified_seed, 17));

    r.ground_type_3 = get_random_block(chunk, modified_seed, false, false, false, false);
    Random::scramble_rng_seed(&modified_seed, Vector3(-12, 983, 17));

    r.ground_type_4 = get_random_block(chunk, modified_seed, false, false, false, false);
    Random::scramble_rng_seed(&modified_seed, Vector3(-12, modified_seed, 17));

    r.ground_type_5 = get_random_block(chunk, modified_seed, false, false, false, false);
    Random::scramble_rng_seed(&modified_seed, Vector3(-12, 983, modified_seed));

    r.ground_type_6 = get_random_block(chunk, modified_seed, false, false, false, false);
    Random::scramble_rng_seed(&modified_seed, Vector3(-12, 983, 17));

    r.wacky_block_1 = get_random_block(chunk, modified_seed, false, true, true, false);
    Random::scramble_rng_seed(&modified_seed, Vector3(-modified_seed, 983, 17));

    r.foliage_1 = get_random_block(chunk, modified_seed, true, false, true, true);
    Random::scramble_rng_seed(&modified_seed, Vector3(-12, 983, 17));

    r.foliage_2 = get_random_block(chunk, modified_seed, true, false, true, true);
    Random::scramble_rng_seed(&modified_seed, Vector3(-12, modified_seed, 17));

    r.foliage_3 = get_random_block(chunk, modified_seed, true, false, true, true);
    Random::scramble_rng_seed(&modified_seed, Vector3(-12, modified_seed, 17));

    r.decoration_index_1 = Random::randi(modified_seed) % random_decorations.size(); 
    Random::scramble_rng_seed(&modified_seed, Vector3(modified_seed, 7, 2));

    r.decoration_index_2 = Random::randi(modified_seed) % random_decorations.size(); 
    Random::scramble_rng_seed(&modified_seed, Vector3(modified_seed, 7, 2));

    r.decoration_index_3 = Random::randi(modified_seed) % random_decorations.size(); 
    Random::scramble_rng_seed(&modified_seed, Vector3(modified_seed, 7, 2));

    r.water_decoration_index_1 = Random::randi(modified_seed) % random_decorations.size(); 
    Random::scramble_rng_seed(&modified_seed, Vector3(modified_seed, 7, 2));

    r.water_decoration_index_2 = Random::randi(modified_seed) % random_decorations.size(); 
    Random::scramble_rng_seed(&modified_seed, Vector3(modified_seed, 7, 2));

    r.decoration_frequency_multiplier = 0.75 + 0.5 * Random::randf(modified_seed);

    return r;
}

int RandomBiome::get_random_block(Chunk* chunk, uint16_t seed, bool allow_air, bool allow_living, bool allow_transparent, bool allow_foliage) {    
    if (chunk == nullptr) {
        return 0;
    }
    
    int max_attempts = 16;
    for (int i = 0; i < max_attempts; i++) {
        if (allow_air && Random::randf(seed) < 0.07) {
            return 0;
        }

        int64_t block_index = Random::randi(seed) % chunk->block_types.size();
        Random::scramble_rng_seed(&seed, Vector3(-7, seed, 3));

        // always forbidden
        if (chunk->is_block_internal[block_index] || block_index == gold_block_index || block_index == respawn_block_index || block_index == heaven_block_index || block_index == lavender_block_index || block_index == alma_block_index) {
            continue;
        }

        if (!allow_transparent && chunk->is_block_transparent[block_index]) {
            continue;
        }

        if (!allow_living && chunk->is_block_living[block_index]) {
            continue;
        }

        if (!allow_foliage && chunk->is_block_foliage[block_index]) {
            continue;
        }

        if (allow_foliage && !chunk->is_block_foliage[block_index] && Random::randf(seed) < 0.85) {
            continue;
        }
        Random::scramble_rng_seed(&seed, Vector3(-7, 7, seed));
        
        return block_index;
    } 
    
    return dirt_block_index;
}

void RandomBiome::generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate) {
    uint16_t rng_seed = world->generator->seed;
    Random::scramble_rng_seed(&rng_seed, chunk_position);

    bool melon_chunk = Random::randf(rng_seed) < rare_chunk_chance;

    for (int64_t z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
    for (int64_t x = 0; x < Chunk::CHUNK_SIZE_X; x++) {
        Vector3i column_position = Vector3i(chunk_position.x, 0, chunk_position.z) + Vector3i(x, 0, z);
        RandomBiomeIdentity r = randomize(chunk, column_position, biome_coordinate);

        int64_t ground_level = get_specialized_ground_level(r, column_position, biome_coordinate);
        int64_t water_level = get_water_level(column_position, biome_coordinate);        

        for (int64_t y = 0; y < Chunk::CHUNK_SIZE_Y; y++) {
            Vector3i local_position = Vector3i(x, y, z);
            Vector3i position = chunk_position + local_position;
            Random::scramble_rng_seed(&rng_seed, position);
            if (block_locked(chunk, local_position)) {
                continue;
            }

            int32_t block_type = 0;

            int x_local = UtilityFunctions::posmod(position.x, r.grid_size);
            int y_local = UtilityFunctions::posmod(position.y, r.grid_size);
            int z_local = UtilityFunctions::posmod(position.z, r.grid_size);

            bool x_aligned = x_local == 0;
            bool y_aligned = y_local == 0;
            bool z_aligned = z_local == 0;

            int snapped_x = r.grid_size * (position.x / r.grid_size);
            int snapped_z = r.grid_size * (position.z / r.grid_size);
            bool checker = r.checkerboard && UtilityFunctions::posmod((snapped_x + snapped_z) / r.grid_size, 2) == 0;

            int aligned = 0;
            if (x_aligned) aligned++;
            if (y_aligned) aligned++;
            if (z_aligned) aligned++;

            if (r.ground_type == 0) {
                if (r.grid && aligned >= 2 && position.y >= ground_level && position.y <= ground_level + 16 + r.offset_size) {
                    block_type = checker ? r.ground_type_3 : r.ground_type_4;
                }
                else if (position.y > ground_level + 1) {
                    // Air
                } else if (position.y == ground_level + 1) {
                    // Foliage
                    if (water_level < position.y && position.y <= water_level + beach_height + 1) {
                        // No foliage on beaches
                    } else {
                        block_type = 0;

                        // Prevents grass from spawning with no ground under it
                        if (is_foliage_safe(chunk, position, local_position)) {
                            // Pick between common/uncommon foliage
                            if (Random::randf(rng_seed) < 0.015) {
                                if (melon_chunk) {
                                    block_type = r.wacky_block_1;
                                } else {
                                    Random::scramble_rng_seed(&rng_seed, position);
                                    block_type = Random::randf(rng_seed) < 0.8 ? r.foliage_2 : r.foliage_3;
                                }
                            } else if (Random::randf(rng_seed) < 0.18) {
                                block_type = checker ? r.foliage_2 : r.foliage_1;
                            }
                        }
                    }
                } else if (position.y == ground_level) {
                    // Top level of ground
                    int randomized_beach_height = beach_height + (int) (6 * sand_noise->get_noise_2d(position.x, position.z));
                    block_type = position.y <= water_level + randomized_beach_height ? (checker ? r.ground_type_1 : r.ground_type_5) : (checker ? r.ground_type_5 : r.ground_type_1);
                } else if (position.y < ground_level && position.y > ground_level - dirt_height) {
                    // Second layer of ground
                    int randomized_beach_height = beach_height + (int) (6 * sand_noise->get_noise_2d(position.x, position.z));
                    block_type = position.y <= water_level + randomized_beach_height ? (checker ? r.ground_type_3 : r.ground_type_5) : (checker ? r.ground_type_5 : r.ground_type_3);
                } else {
                    // Underground
                    block_type = checker ? r.ground_type_4 : r.ground_type_2;
                }
            }

            // Biome floor
            if (is_on_vertical_border(chunk, position, local_position)) {
                block_type = r.ground_type_4;
            }

            if (is_aether) {
                int biome_height_offset = world->generator->Y_PER_BIOME_Y * (biome_coordinate.y - 6);
                float close_to_border_penalty = UtilityFunctions::clampf((40 + biome_height_offset - position.y) / 32.0f, 0.0f, 1.0f);
                if (position.y <= ground_level + 1 && cloud_noise->get_noise_3d(position.x, position.y, position.z) - close_to_border_penalty < 0) {
                    block_type = 0;
                }
            }
            

            chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = block_type;
            chunk->water[Chunk::position_to_index(Vector3i(x, y, z))] = !is_aether && position.y <= water_level ? 255 : 0;
        }
    }
    }
}

// rewrite code for masking feature
void RandomBiome::generate_decorations(Vector3i chunk_position, Vector3i biome_coordinate) {
    if (random_decorations.size() == 0) {
        return;
    }

    uint16_t rng_seed = world->generator->seed;

    for (int64_t z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
    for (int64_t x = 0; x < Chunk::CHUNK_SIZE_X; x++) {
        Vector3i column_position = Vector3i(x, 0, z) + Vector3i(chunk_position.x, 0, chunk_position.z);
        RandomBiomeIdentity r = randomize(nullptr, column_position, biome_coordinate);

        Vector3i position = Vector3i(x, 0, z) + chunk_position;

        // Check if decoration should exist here
        Random::scramble_rng_seed(&rng_seed, position);
        if (Random::randf(rng_seed) > r.decoration_frequency_multiplier * random_decoration_frequency) {
            continue;
        }

        // Check if this vertical column contains the block above ground-level
        int64_t ground_level = get_specialized_ground_level(r, column_position, biome_coordinate);
        int64_t y = 1 + ground_level - chunk_position.y;
        if (y < 0 || y > Chunk::CHUNK_SIZE_Y) {
            continue;
        }

        Vector3i new_position = chunk_position + Vector3i(x, y, z);
        if (is_aether) {
            int biome_height_offset = world->generator->Y_PER_BIOME_Y * (biome_coordinate.y - 6);
            float close_to_border_penalty = UtilityFunctions::clampf((40 + biome_height_offset - new_position.y) / 32.0f, 0.0f, 1.0f);
            if (cloud_noise->get_noise_3d(new_position.x, new_position.y, new_position.z) - close_to_border_penalty < 0) {
                continue;
            }
        }

        // Find decoration type
        Random::scramble_rng_seed(&rng_seed, position);
        Ref<DecorationState> d = memnew(DecorationState);

        int64_t water_level = get_water_level(chunk_position + Vector3i(x, 0, z), biome_coordinate);
        if (ground_level <= water_level) {
            d->decoration = random_decorations[Random::randf(rng_seed) < 0.5 ? r.water_decoration_index_1 : r.water_decoration_index_2];
        } else {
            d->decoration = random_decorations[Random::randf(rng_seed) < 0.666 ? (Random::randf(rng_seed) < 0.3333 ? r.decoration_index_1 : r.decoration_index_2) : r.decoration_index_3];
        }

        position = chunk_position + Vector3i(x, y, z);

        // Place the decoration
        d->position = position;
        d->direction = Random::randdir(rng_seed);

        world->place_decoration(d);
    }
    }
}

int RandomBiome::get_specialized_ground_level(RandomBiomeIdentity r, Vector3i column_position, Vector3i biome_coordinate) {
    int biome_height_offset = world->generator->Y_PER_BIOME_Y * (biome_coordinate.y - 6);
    int64_t ground_level = biome_height_offset;
    if (r.terrain_type == 0) {
        int river_offset = 0;
        double presence_noise = river_noise_1->get_noise_2d(column_position.x, column_position.z);
        if (-0.4 < presence_noise && presence_noise < 0.4) {
            double angle = river_noise_2->get_noise_2d(column_position.x, column_position.z) * 2 * Math_PI;
            Vector2 warp = 6.0 * Vector2(UtilityFunctions::cos(angle), UtilityFunctions::sin(angle));
            double warped_noise = river_noise_1->get_noise_2d(column_position.x + warp.x, column_position.z + warp.y);
            river_offset = (int) (-22 * (1.0 - UtilityFunctions::smoothstep(0, 0.14, UtilityFunctions::absf(warped_noise))));
        }    

        if (r.cliff) {
            const float e = 2.71828;
            float height = terrain_noise->get_noise_2d(r.frequency_multiplier * column_position.x, r.frequency_multiplier * column_position.z);
            float v = (1 + UtilityFunctions::pow(e, -60 * (height - 0.32)));
            if (UtilityFunctions::is_zero_approx(v)) {
                v = 0.01;
            }
            ground_level = (is_aether ? 24 : 0) + biome_height_offset + river_offset + (int64_t) (r.height_multiplier * (32.0 + 90.0 / v + height * 42.0 + 4 * UtilityFunctions::sin(height * 64.0)));
        } else {
            float height = terrain_noise->get_noise_2d(r.frequency_multiplier * column_position.x, r.frequency_multiplier * column_position.z);
            ground_level = (is_aether ? 24 : 0) + biome_height_offset + river_offset + 6 + (int64_t) (r.height_multiplier * (96.0 * (0.15 + height / 2 + UtilityFunctions::pow(height, 8) + UtilityFunctions::pow(height + 0.5, 4))));
        }
    } else if (r.terrain_type == 1) {
        int64_t snapped_x = r.grid_size * (column_position.x / r.grid_size);
        int64_t snapped_z = r.grid_size * (column_position.z / r.grid_size);
        if (!is_cliff) {
            int64_t y = (int64_t) (r.height_multiplier * 96.0 * (0.425 + terrain_noise->get_noise_2d(r.frequency_multiplier * snapped_x, r.frequency_multiplier * snapped_z)));
            int64_t snapped_y = r.grid_size * (y / r.grid_size);
            ground_level = (is_aether ? 24 : 0) + biome_height_offset + snapped_y;
        } else {
            float height = terrain_noise->get_noise_2d(r.frequency_multiplier * snapped_x, r.frequency_multiplier * snapped_z);
            int64_t y = (int64_t) (r.height_multiplier * 96.0 * (0.15 + height / 2 + UtilityFunctions::pow(height, 8) + UtilityFunctions::pow(height + 0.5, 4)));
            int64_t snapped_y = r.grid_size * (y / r.grid_size);
            ground_level = (is_aether ? 24 : 0) + biome_height_offset + snapped_y;
        }
    } else if (r.terrain_type == 2) {
        ground_level = (is_aether ? 24 : 0) + (int64_t) (biome_height_offset + 24 + r.height_multiplier * 40.0 * terrain_noise->get_noise_2d(r.frequency_multiplier * column_position.x, r.frequency_multiplier * column_position.z));
    } else if (r.terrain_type == 3) {
        ground_level = (is_aether ? 24 : 0) + get_water_level(column_position, biome_coordinate) + r.offset_size; 
    }
    return ground_level;
}

DEFINE_PROPERTY_GETTER_SETTER(RandomBiome, bool, is_aether);
