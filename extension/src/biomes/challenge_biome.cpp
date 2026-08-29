#include "../../include/biomes/challenge_biome.h"
#include "../../include/world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void ChallengeBiome::_bind_methods() {
    BIND_REF_PROPERTY(ChallengeBiome, Decoration, room);
}

DEFINE_CONSTRUCTORS(ChallengeBiome);

void ChallengeBiome::initialize() {
    Biome::initialize();
    placed_room.instantiate();
    placed_room->decoration = room;
    placed_room->direction = DecorationState::North;
    placed_room->position = Vector3i();

}

void ChallengeBiome::generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate) {
    for (int y = 0; y < Chunk::CHUNK_SIZE_Y; y++) {
    for (int z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
    for (int x = 0; x < Chunk::CHUNK_SIZE_X; x++) {
        if (block_locked(chunk, Vector3i(x, y, z))) {
            continue;
        }
        chunk->water[Chunk::position_to_index(Vector3i(x, y, z))] = 0;
        chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = 0;
    }
    }
    }

    Generator::fill_large_decoration(world, chunk, chunk_position, room, placed_room, false);
}

DEFINE_PROPERTY_GETTER_SETTER(ChallengeBiome, Ref<Decoration>, room);