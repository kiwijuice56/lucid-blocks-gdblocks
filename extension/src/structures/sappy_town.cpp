#include "../../include/structures/sappy_town.h"
#include "../../include/world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void SappyTown::_bind_methods() {
    BIND_REF_ARRAY_PROPERTY(SappyTown, Decoration, decorations);
}

DEFINE_CONSTRUCTORS(SappyTown);

void SappyTown::generate_chunk_data(Chunk* chunk, Vector3i chunk_position) {
    uint16_t rng_seed = world->generator->seed;

    for (int64_t i = 0; i < placed_decorations.size(); i++) {
        Ref<DecorationState> ds = placed_decorations[i];
        Ref<Decoration> d = ds->decoration;
        Generator::fill_decoration(world, chunk, chunk_position, d, ds, false);
    }
}

void SappyTown::initialize() {
    uint16_t rng_seed = world->generator->seed;

    Random::scramble_rng_seed(&rng_seed, heart_position);
    int i_offset = Random::randi(rng_seed) % 3 - 1;

    Random::scramble_rng_seed(&rng_seed, heart_position);
    int j_offset = Random::randi(rng_seed) % 3 - 1;

    for (int i = 3; i < World::STRUCTURE_SIZE / Chunk::CHUNK_SIZE_X - 3; i++) {
    for (int j = 3; j < World::STRUCTURE_SIZE / Chunk::CHUNK_SIZE_Z - 3; j++) {
        Vector3i column_position = root_position + Vector3i(Chunk::CHUNK_SIZE_X * (i + i_offset) + Chunk::CHUNK_SIZE_X / 2, World::STRUCTURE_SIZE / 2, Chunk::CHUNK_SIZE_Z * (j + j_offset) + Chunk::CHUNK_SIZE_Z / 2);

        Random::scramble_rng_seed(&rng_seed, column_position);
        column_position.x += Random::randi(rng_seed) % 10 - 5;

        Random::scramble_rng_seed(&rng_seed, column_position);
        column_position.z += Random::randi(rng_seed) % 10 - 5;

        Random::scramble_rng_seed(&rng_seed, Vector3i(i, 0, j));
        if (Random::randf(rng_seed) < 0.75) {
            continue;
        }

        // Pick center
        Ref<Biome> biome = world->generator->get_biome_at(column_position, Generator::LAYER_BASE);
        Vector3i biome_coordinate = world->generator->get_true_biome_coordinate(column_position);
        int height_level = biome->get_ground_level(column_position, biome_coordinate);

        if (height_level < root_position.y || height_level >= root_position.y + World::STRUCTURE_SIZE) {
            continue;
        }

        Vector3i placement_position = Vector3i(column_position.x, height_level, column_position.z);

        // Pick decoration
        Random::scramble_rng_seed(&rng_seed, placement_position);
        uint8_t decoration_index = Random::randi(rng_seed) % decorations.size();

        // Check that the 4 corners aren't over the air
        bool skip = false;

        const int HEIGHT_ALLOWANCE = 3;
        const int BASE_CHECK_WIDTH = 6;
        for (int x = 0; x < 2; x++) {
        for (int z = 0; z < 2; z++) {
            Vector3i other_position = placement_position + BASE_CHECK_WIDTH * Vector3i(x == 0 ? -1 : 1, 0, z == 0 ? -1 : 1);
            Ref<Biome> biome = world->generator->get_biome_at(other_position, Generator::LAYER_BASE);
            Vector3i other_biome_coordinate = world->generator->get_true_biome_coordinate(other_position);
            int64_t other_height_level = biome->get_ground_level(other_position, other_biome_coordinate);

            if (UtilityFunctions::absi(other_height_level - height_level) > HEIGHT_ALLOWANCE) {
                skip = true;
            }
        }
        }

        if (skip) {
            continue;
        }

        Ref<DecorationState> d = memnew(DecorationState);
        d->decoration = decorations[decoration_index];
        d->position = placement_position;

        Random::scramble_rng_seed(&rng_seed, placement_position);
        int direction = Random::randi(rng_seed) % 4;

        if (direction == 0) {
            d->direction = DecorationState::North;
        } else if (direction == 1) {
            d->direction = DecorationState::East;
        } else if (direction == 2) {
            d->direction = DecorationState::South;
        } else if (direction == 3) {
            d->direction = DecorationState::West;
        }

        // Select the first eligible decoration as the cutscene block spawner
        if (d->decoration->has_cutscene_block && !cutscene_block_decoration_spawned) {
            d->has_cutscene_block = true;
            cutscene_block_decoration_spawned = true;
            
            cutscene_block_position = Generator::find_actual_block_position(
                            d->decoration->cutscene_block_position,
                            d->position, d->decoration->size, d->decoration->center_offset,
                            d->direction
            );
        }

        placed_decorations.append(d);
    }
    }
}

Vector3i SappyTown::get_cutscene_block_position() {
    return cutscene_block_position;
}

bool SappyTown::has_cutscene_block() {
    return cutscene_block_decoration_spawned;
}


bool SappyTown::is_within_structure(Vector3i position) {
    return false;
}

DEFINE_PROPERTY_GETTER_SETTER(SappyTown, TypedArray<Decoration>, decorations);
