#pragma once


#include "macros.h"
#include <vector>

#include "block.h"
#include "decoration.h"

#include <godot_cpp/variant/typed_dictionary.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/material.hpp>
#include <godot_cpp/classes/shader_material.hpp>
#include <godot_cpp/classes/concave_polygon_shape3d.hpp>
#include <godot_cpp/classes/static_body3d.hpp>
#include <godot_cpp/classes/collision_shape3d.hpp>
#include <godot_cpp/classes/box_mesh.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/random_number_generator.hpp>


namespace godot {

class World;

class Chunk : public MeshInstance3D {
	DECLARE_CLASS(Chunk, MeshInstance3D);

    PhysicsServer3D *ps; 
    RID collision_shape;
    RID collision_body;

    // The dimensions of individual chunks
    static const int64_t CHUNK_SIZE_X = 16;
    static const int64_t CHUNK_SIZE_Y = 16;
    static const int64_t CHUNK_SIZE_Z = 16;

    // The dimensions of individual chunks
    static const int64_t WATER_CHUNK_SIZE_X = 4;
    static const int64_t WATER_CHUNK_SIZE_Y = 4;
    static const int64_t WATER_CHUNK_SIZE_Z = 4;

    // Water behavior constants
    const uint8_t HEAVY = 212;
    const uint8_t EXTRA = 4;
    const uint8_t EVAPORATE = 3;

    // Fire behavior constants
    const float FIRE_SPREAD_BASE = 0.225;
    const float FIRE_DISSOLVE_BASE = 0.37;
    const float FIRE_COMBUST_BASE = 0.3;
    const float SIMULATE_FIRE_CHUNK = 0.009;
    const float SIMULATE_FIRE = 0.75;
    const int FIRE_LIMIT = 32;

    // Block + water + fire index data
    PackedInt32Array blocks;
    PackedByteArray fire;
    TypedDictionary<Vector3i, Ref<Node3D>> owned_fire_visuals; // Vector3i : Node3d pairs
    PackedByteArray water; // Stores numerical density 0-255
    PackedByteArray water_chunk_awake; // Stores whether each water subchunk should be awake
    PackedByteArray water_chunk_awake_buffer; // Stores whether each water subchunk should be awake
    PackedByteArray biome; // Stores the biome id of each block

    bool will_be_remeshed = false;

    bool consistent_biome = true;
    bool consistent_void = true;

    bool water_surface_meshed = false;
    uint8_t water_updated = 0;
    uint8_t water_shuffle = 0;
    uint16_t water_render_wait = 0;

    // Allows deterministic random offsets of foliage
    Ref<RandomNumberGenerator> foliage_rng;

    // Mesh data
    std::vector<Vector3> vertices;
    std::vector<Vector3> normals;
    std::vector<Vector2> uvs;
    std::vector<Vector2> uvs2;
    PackedVector3Array collision_vertices;
    Array opaque_mesh_data;
    Array transparent_mesh_data;
    Array foliage_mesh_data;
    Ref<ArrayMesh> water_mesh_data;
    Ref<ArrayMesh> water_surface_mesh_data;
    
    // Child mesh nodes
    MeshInstance3D* water_mesh;
    MeshInstance3D* water_mesh_surface;
    MeshInstance3D* foliage_mesh;

    // Resources set by World
    World* world;
    TypedArray<Block> block_types;
    PackedByteArray is_block_transparent;
    PackedByteArray is_block_foliage;
    PackedByteArray is_block_floating_foliage;
    PackedByteArray is_block_living;
    PackedByteArray is_block_internal;
    PackedByteArray is_block_top_face_randomized;
    PackedByteArray is_block_non_collision;
    Ref<ShaderMaterial> block_material;
	Ref<ShaderMaterial> transparent_block_material;
    TypedDictionary<int, int> biome_height_map; // Maps biome ID to height

    // Greedy meshing state variables
    bool* visited;
    uint64_t current_greedy_block = 0;
    uint64_t current_greedy_block_real = 0;

    // Calculated state variables
    uint64_t face_count = 0; // Quads, not tris
    uint64_t block_count = 0;
    uint64_t water_count = 0;
    uint64_t fire_count = 0;
    bool uniform = false; // Used to optimize chunks of one block type
    bool modified = false; // Whether this chunk has been disturbed at all
    bool dirty = false; // (for modified chunks only) whether this chunk has been disturbed since being loaded
    bool never_initialized = true;
    bool garbage = true;

    void _ready();

    // External interfacing methods
    void remove_block_at(Vector3i global_position, bool remesh_later);
    bool place_block_at(Vector3i global_position, uint32_t block_index, bool immediate_remesh);
    uint64_t get_block_index_at_global(Vector3i global_position);
    uint64_t get_block_index_at_safe(Vector3i local_position);
    uint8_t get_biome_at(Vector3i local_position);

    // Internal interfacing methods
    void mark_as_garbage();
    void mark_as_used();
    uint64_t get_block_index_at(Vector3i position);
    static uint64_t position_to_index(Vector3i position);
    Vector3i index_to_position(uint64_t index);
    static bool in_bounds(Vector3i position);

    // Mesh generation methods
    void generate_mesh(bool immediate, Vector3 global_position);
    void generate_water_mesh(bool clear, Vector3 global_position, bool immediate);
    void generate_water_surface_mesh(bool remove, Vector3 global_position, bool immediate);
    void calculate_block_statistics();
    void clear_collision();
    void make_visible();
    void finalize_main_mesh_loading();
    void finalize_water_mesh_loading();
    void finalize_water_surface_mesh_loading();

    // Helper methods to generate chunk mesh
    void add_face_uvs(uint64_t id, uint64_t id_2, Vector2i scale);
    void add_face_uvs_reversed(uint64_t id, uint64_t id_2, Vector2i scale);
    void add_face_normals(Vector3i normal);
    void add_rectangular_prism(Vector3 start, Vector3 size, bool water_pass, bool collision_pass, uint64_t data);
    void add_cross_face(Vector3 start, int32_t block_index);
    PackedVector3Array to_godot_array(const std::vector<Vector3> &vec);
    PackedVector2Array to_godot_array(const std::vector<Vector2> &vec);

    void foliage_mesh_generation(Vector3 global_position);
    void greedy_mesh_generation(bool transparent, bool water_pass, bool collision_pass, Vector3 global_position);
    Vector3i greedy_scan(Vector3i start, bool water_pass, bool collision_pass);
    bool greedy_invalid(Vector3i position, bool water_pass, bool collision_pass);

    // Water simulation
    uint8_t get_water_at(Vector3i local_position);
    uint8_t get_water_at_safe(Vector3i local_position);
    void set_water_at(Vector3i local_position, uint8_t water);
    void simulate_water();
    void water_chunk_wake_set(Vector3i local_position, bool awake, bool surround);

    // Fire simulation
    uint8_t get_fire_at(Vector3i local_position);
    void set_fire_at(Vector3i local_position, uint8_t new_fire);
    void spread_fire(Vector3i local_position, uint8_t new_fire, bool force);
    bool fire_eligible(Vector3i local_position);
    bool fire_neighbor(Vector3i local_position);
    void validate_fire_at(Vector3i local_position, bool deep);
    void update_fire_visual(Vector3i local_position, Vector3 global_position, bool animate);
    void initialize_fire_visuals(Vector3 global_position);
    void simulate_fire();
};
}

