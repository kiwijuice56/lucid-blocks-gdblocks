#include "../../include/structures/structure.h"
#include "../../include/world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void Structure::_bind_methods() {
    BIND_PROPERTY(Structure, String, internal_name);
    BIND_PROPERTY(Structure, Color, sky_tint);
    BIND_REF_ARRAY_PROPERTY(Structure, AudioStream, songs);

    ADD_GROUP("Spawn", "sp_");
    BIND_PROPERTY_PREFIXED(Structure, float, sp_, spawn_rate);
    BIND_PROPERTY_PREFIXED(Structure, PackedFloat32Array, sp_, spawn_proportions);
    BIND_REF_ARRAY_PROPERTY_PREFIXED(Structure, PackedScene, sp_, spawns);
}

DEFINE_CONSTRUCTORS(Structure);

void Structure::generate_chunk_data(Chunk* chunk, Vector3i chunk_position) { }

void Structure::initialize() { }

bool Structure::is_within_structure(Vector3i position) {
    return false;
}

bool Structure::in_bounds_of_structure(Vector3i position) {
    return position.x >= root_position.x && position.x < root_position.x + World::STRUCTURE_SIZE &&
           position.y >= root_position.y && position.y < root_position.y + World::STRUCTURE_SIZE &&
           position.z >= root_position.z && position.z < root_position.z + World::STRUCTURE_SIZE;
}

bool Structure::on_chunk_border(Vector3i position, int offset) {
    return position.x == offset || position.y == offset || position.z == offset ||
           position.x == Chunk::CHUNK_SIZE_X - 1 - offset ||
           position.y == Chunk::CHUNK_SIZE_Y - 1 - offset ||
           position.z == Chunk::CHUNK_SIZE_Z - 1 - offset;
}

bool Structure::on_chunk_corner(Vector3i position, int offset) {
    bool x_side = position.x == offset || position.x == Chunk::CHUNK_SIZE_X - 1 - offset;
    bool z_side = position.z == offset || position.z == Chunk::CHUNK_SIZE_Z - 1 - offset;
    return x_side && z_side;
}

Vector3i Structure::get_cutscene_block_position() {
    return Vector3i();
}

bool Structure::has_cutscene_block() {
    return false;
}

DEFINE_PROPERTY_GETTER_SETTER(Structure, String, internal_name);
DEFINE_PROPERTY_GETTER_SETTER(Structure, TypedArray<AudioStream>, songs);
DEFINE_PROPERTY_GETTER_SETTER(Structure, float, spawn_rate);
DEFINE_PROPERTY_GETTER_SETTER(Structure, TypedArray<PackedScene>, spawns);
DEFINE_PROPERTY_GETTER_SETTER(Structure, PackedFloat32Array, spawn_proportions);
DEFINE_PROPERTY_GETTER_SETTER(Structure, Color, sky_tint);