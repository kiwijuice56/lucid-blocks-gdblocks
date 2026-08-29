
#pragma once

#include "../macros.h"
#include "../decoration_state.h"
#include "../chunk.h"
#include "../random.h"

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/noise.hpp>
#include <godot_cpp/classes/fast_noise_lite.hpp>
#include <godot_cpp/classes/packed_scene.hpp>

namespace godot {

class World;

class Structure : public Resource {
	DECLARE_CLASS(Structure, Resource);

    // Set by generator
    World* world;

    // Corner of this structure
    Vector3i root_position;

    // Center of this structure (with some noise to the x/z axes)
    Vector3i center_position;

    // Center with noise and ground snapping
    Vector3i heart_position;

    // Center + ground adjustment only
    Vector3i grid_center_grounded_position;

    DECLARE_PROPERTY(String, internal_name, "");

    DECLARE_PROPERTY(float, spawn_rate, 1.);
    DECLARE_PROPERTY_NO_DEFAULT(TypedArray<PackedScene>, spawns);
    DECLARE_PROPERTY_NO_DEFAULT(PackedFloat32Array, spawn_proportions);
    DECLARE_PROPERTY_NO_DEFAULT(TypedArray<AudioStream>, songs);
    DECLARE_PROPERTY(Color, sky_tint, Color(0, 0, 0, 1));

    virtual void initialize();
    virtual void generate_chunk_data(Chunk* chunk, Vector3i chunk_position);
    virtual bool is_within_structure(Vector3i position);
    virtual Vector3i get_cutscene_block_position();
    virtual bool has_cutscene_block();

    bool in_bounds_of_structure(Vector3i position);
    static bool on_chunk_border(Vector3i position, int offset);
    static bool on_chunk_corner(Vector3i position, int offset);

};

}