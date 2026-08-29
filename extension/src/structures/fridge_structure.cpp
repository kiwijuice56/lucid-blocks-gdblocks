#include "../../include/structures/fridge_structure.h"
#include "../../include/world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void FridgeStructure::_bind_methods() {
    BIND_REF_PROPERTY(FridgeStructure, Decoration, decoration);
}

DEFINE_CONSTRUCTORS(FridgeStructure);

void FridgeStructure::generate_chunk_data(Chunk* chunk, Vector3i chunk_position) {
    uint16_t rng_seed = world->generator->seed;
    PackedInt32Array blocks = decoration->get_blocks();
    Vector3i size = decoration->get_size();

    for (int i = 0; i < placed_words.size(); i++) {
        Word word = placed_words[i];
        Vector3i root_position_global = placed_decoration->position - decoration->get_center_offset() + word.root_position;
        for (int j = 0; j < word.word.length(); j++) {
            Vector3i letter_position_global = root_position_global - Vector3i(0, 0, j);
            Vector3i letter_position_local = letter_position_global - chunk_position;
            if (!chunk->in_bounds(letter_position_local)) {
                continue;
            }

            Vector3i letter_position_decoration_local = letter_position_global - placed_decoration->position + decoration->get_center_offset();
            if (letter_position_decoration_local.z <= upper_outer_min.z || letter_position_decoration_local.z <= lower_outer_min.z) {
                continue;
            }

            int existing_index = chunk->get_block_index_at(letter_position_local);
            if (existing_index != 0) {
                continue;
            }

            String block_name = word.word.substr(j, 1) + " x+";
            chunk->blocks[Chunk::position_to_index(letter_position_local)] = world->block_name_map[block_name];
        }
    }

    for (int y = 0; y < Chunk::CHUNK_SIZE_Y; y++) {
    for (int z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
    for (int x = 0; x < Chunk::CHUNK_SIZE_X; x++) {
        Vector3i local_position = Vector3i(x, y, z);
        Vector3i world_position = chunk_position + local_position;
        Vector3i decoration_local = world_position - placed_decoration->position + decoration->get_center_offset();

        if (decoration_local.x < 0 || decoration_local.y < 0 || decoration_local.z < 0 ||
            decoration_local.x >= size.x || decoration_local.y >= size.y || decoration_local.z >= size.z) {
            continue;
        }

        int block_id = blocks[decoration_local.x + decoration_local.z * size.x + decoration_local.y * size.x * size.z];

        // We're allowed to compare IDs here, since these are replace blocks
        int index = 0;
        if (block_id == 3) {
            index = world->block_name_map["pool tile block"];
        } else if (block_id == 5 || block_id == 9) {
            index = world->block_name_map["metal block"];
        } else if (block_id == 10) {
            index = world->block_name_map["glass"];
        } else if (block_id == 12) {
            index = world->block_name_map["cutscene block"];
        } else if ( decoration_local.x >= lower_inner_min.x && decoration_local.y >= lower_inner_min.y && decoration_local.z >= lower_inner_min.z &&
                    decoration_local.x <= lower_inner_max.x && decoration_local.y <= lower_inner_max.y && decoration_local.z <= lower_inner_max.z ||
                    decoration_local.x >= upper_inner_min.x && decoration_local.y >= upper_inner_min.y && decoration_local.z >= upper_inner_min.z &&
                    decoration_local.x <= upper_inner_max.x && decoration_local.y <= upper_inner_max.y && decoration_local.z <= upper_inner_max.z) {

            double noise = fridge_noise->get_noise_3dv(world_position);
            index = noise < 0.1 ? 1 : (int) world->block_name_map["snow block"];
        }

        if (index == 0) continue;
        if (index == 1) index = 0;

        chunk->blocks[Chunk::position_to_index(local_position)] = index;
    }
    }
    }
}

void FridgeStructure::initialize() {
    uint16_t rng_seed = world->generator->seed;
    Random::scramble_rng_seed(&rng_seed, heart_position);

    fridge_noise = world->generator->noise[26];

    placed_decoration = Ref<DecorationState>(memnew(DecorationState));
    placed_decoration->decoration = decoration;
    placed_decoration->has_cutscene_block = true;
    placed_decoration->position = heart_position;

    int word_count = 4 + Random::randi(rng_seed) % 3;
    placed_words.resize(word_count);
    for (int i = 0; i < word_count; i++) {
        Word new_word;

        Random::scramble_rng_seed(&rng_seed, Vector3i(rng_seed, rng_seed, rng_seed) * 7 + heart_position * heart_position);
        new_word.word = words[Random::randi(rng_seed) % 52];

        Random::scramble_rng_seed(&rng_seed, Vector3i(rng_seed, rng_seed, rng_seed) * 7 + heart_position * heart_position);
        if (Random::randf(rng_seed) < 0.5) {
            new_word.root_position.x = upper_outer_max.x;
            Random::scramble_rng_seed(&rng_seed, Vector3i(rng_seed, rng_seed, rng_seed) * 7 + heart_position * heart_position);
            new_word.root_position.z = upper_outer_min.z + Random::randi(rng_seed) % (upper_outer_max.z - upper_outer_min.z);
            Random::scramble_rng_seed(&rng_seed, Vector3i(rng_seed, rng_seed, rng_seed) * 7 + heart_position * heart_position);
            new_word.root_position.y = upper_outer_min.y + Random::randi(rng_seed) % (upper_outer_max.y - upper_outer_min.y);
        } else {
            new_word.root_position.x = lower_outer_max.x;
            Random::scramble_rng_seed(&rng_seed, Vector3i(rng_seed, rng_seed, rng_seed) * 7 + heart_position * heart_position);
            new_word.root_position.z = lower_outer_min.z + Random::randi(rng_seed) % (lower_outer_max.z - lower_outer_min.z);
            Random::scramble_rng_seed(&rng_seed, Vector3i(rng_seed, rng_seed, rng_seed) * 7 + heart_position * heart_position);
            new_word.root_position.y = lower_outer_min.y + Random::randi(rng_seed) % (lower_outer_max.y - lower_outer_min.y);

        }

        placed_words[i] = new_word;
    }
}

Vector3i FridgeStructure::get_cutscene_block_position() {
    return Generator::find_actual_block_position(
        placed_decoration->decoration->cutscene_block_position,
        placed_decoration->position, placed_decoration->decoration->size, placed_decoration->decoration->center_offset,
        placed_decoration->direction
    );
}

bool FridgeStructure::has_cutscene_block() {
    return decoration->has_cutscene_block;
}

bool FridgeStructure::is_within_structure(Vector3i position) {
    return position.distance_to(heart_position) < 128.0;
}

DEFINE_PROPERTY_GETTER_SETTER(FridgeStructure, Ref<Decoration>, decoration);
