#include "../../include/biomes/babel_biome.h"
#include "../../include/world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

DEFINE_CONSTRUCTORS_AND_BINDINGS(BabelBiome);

void BabelBiome::generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate) {
    const int WIDTH = 22;
    const int HEIGHT = 6;

    for (int64_t y = 0; y < Chunk::CHUNK_SIZE_Y; y++) {
    for (int64_t z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
    for (int64_t x = 0; x < Chunk::CHUNK_SIZE_X; x++) {
        int64_t water_level = get_water_level(chunk_position + Vector3i(x, 0, z), biome_coordinate);
        Vector3i local_position = Vector3i(x, y, z);
        Vector3i position = chunk_position + local_position;
        if (block_locked(chunk, local_position)) {
            continue;
        }

        Vector3i corner_position = position - Vector3i(Math::posmod(position.x, WIDTH), Math::posmod(position.y, HEIGHT), Math::posmod(position.z, WIDTH));

        uint16_t rng_seed = world->generator->seed;
        Random::scramble_rng_seed(&rng_seed, corner_position);
        Ref<Decoration> decoration = random_decorations[Random::randi(rng_seed) % random_decorations.size()];

        int64_t xi = Math::posmod(position.x, decoration->size.x);
        int64_t yi = Math::posmod(position.y, decoration->size.y);
        int64_t zi = Math::posmod(position.z, decoration->size.z);

        int32_t block_index = world->block_id_to_index_map[decoration->blocks[uint64_t(xi) + uint64_t(zi) * decoration->size.x + uint64_t(yi) * decoration->size.x * decoration->size.z]];
        
        chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = block_index;
        chunk->water[Chunk::position_to_index(Vector3i(x, y, z))] = 0;
    }
    }
    }
}
