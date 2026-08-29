#include "../include/world.h"
#include "../include/chunk.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/physics_server3d.hpp>
#include <godot_cpp/classes/world3d.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void Chunk::_bind_methods() {
    BIND_METHOD_NO_PARAMETERS(Chunk, make_visible);
    BIND_METHOD(Chunk, initialize_fire_visuals, "position");
    BIND_METHOD(Chunk, validate_fire_at, "position", "deep");
}

Chunk::Chunk() {
    // Initialize all data arrays
    visited = new bool[CHUNK_SIZE_X * CHUNK_SIZE_Z * CHUNK_SIZE_Y];
    biome.resize(CHUNK_SIZE_X * CHUNK_SIZE_Y * CHUNK_SIZE_Z);
    blocks.resize(CHUNK_SIZE_X * CHUNK_SIZE_Y * CHUNK_SIZE_Z);
    water.resize(CHUNK_SIZE_X * CHUNK_SIZE_Y * CHUNK_SIZE_Z);
    fire.resize(CHUNK_SIZE_X * CHUNK_SIZE_Y * CHUNK_SIZE_Z);
    water_chunk_awake.resize(CHUNK_SIZE_X * CHUNK_SIZE_Y * CHUNK_SIZE_Z / WATER_CHUNK_SIZE_X / WATER_CHUNK_SIZE_Y / WATER_CHUNK_SIZE_Z);
    water_chunk_awake_buffer.resize(water_chunk_awake.size());   

    // Initialize foliage RNG
    foliage_rng.instantiate();
}

void Chunk::_ready() {
    // Initialize physics server stuff
    Ref<World3D> world = get_world_3d();
    RID scenario = world->get_scenario();
    RID space = world->get_space();
    ps = PhysicsServer3D::get_singleton();

    // Initialize water mesh (separate child)
    water_mesh = memnew(MeshInstance3D);
    water_mesh->set_layer_mask(0);
    water_mesh->set_layer_mask_value(2, true);
    water_mesh->set_cast_shadows_setting(SHADOW_CASTING_SETTING_OFF);
    add_child(water_mesh);

    // Initialize foliage mesh (separate child)
    foliage_mesh = memnew(MeshInstance3D);
    foliage_mesh->set_cast_shadows_setting(SHADOW_CASTING_SETTING_OFF);
    add_child(foliage_mesh);

    // Initialize water surface mesh (separate child)
    water_mesh_surface = memnew(MeshInstance3D);
    water_mesh_surface->set_layer_mask(0);
    water_mesh_surface->set_layer_mask_value(3, true);
    water_mesh_surface->set_cast_shadows_setting(SHADOW_CASTING_SETTING_OFF);
    add_child(water_mesh_surface);

    // Initialize collision data
    collision_body = ps->body_create();
    collision_shape = ps->concave_polygon_shape_create();
    ps->body_set_mode(collision_body, PhysicsServer3D::BODY_MODE_STATIC);
    ps->body_set_space(collision_body, space);
    ps->body_add_shape(collision_body, collision_shape, Transform3D(), false);
   
    ps->body_set_collision_layer(collision_body, (1 << 0) | (1 << 13)); // solid + block interactions
    ps->body_set_collision_mask(collision_body, 0);
}

Chunk::~Chunk() {
    delete [] visited;
    if (ps && collision_body.is_valid()) {
        ps->body_set_space(collision_body, RID());
        ps->free_rid(collision_body);
        collision_body = RID();
    }

    if (ps && collision_shape.is_valid()) {
        ps->free_rid(collision_shape);
        collision_shape = RID();
    }
}

void Chunk::mark_as_garbage() {
    garbage = true;
}

void Chunk::mark_as_used() {
    garbage = false;
}

// Uses local position
uint64_t Chunk::get_block_index_at(Vector3i position) {
    return blocks[position_to_index(position)];
}

// Uses local position
uint64_t Chunk::position_to_index(Vector3i position) {
    return position.x + position.z * CHUNK_SIZE_X + position.y * CHUNK_SIZE_Z * CHUNK_SIZE_X;
}

// Uses local position
Vector3i Chunk::index_to_position(uint64_t index) {
    return Vector3i(index % CHUNK_SIZE_X, index / (CHUNK_SIZE_X * CHUNK_SIZE_Z), (index / CHUNK_SIZE_X) % CHUNK_SIZE_Z);
}

// Uses local position
bool Chunk::in_bounds(Vector3i position) {
    return
    0 <= position.x && position.x < CHUNK_SIZE_X &&
    0 <= position.y && position.y < CHUNK_SIZE_Y &&
    0 <= position.z && position.z < CHUNK_SIZE_Z;
}

uint64_t Chunk::get_block_index_at_global(Vector3i g_position) {
    g_position -= Vector3i(get_global_position());
    return blocks[uint64_t(g_position.x) + uint64_t(g_position.z) * CHUNK_SIZE_X + uint64_t(g_position.y) * CHUNK_SIZE_Z * CHUNK_SIZE_X];
}

uint64_t Chunk::get_block_index_at_safe(Vector3i local_position) {
    if (in_bounds(local_position)) {
        return get_block_index_at(local_position);
    } else {
        Vector3i global_position = local_position + Vector3i(get_global_position());
        Chunk* chunk = world->get_chunk_at(global_position);
        return chunk->get_block_index_at(global_position - chunk->get_global_position());
    }
}

uint8_t Chunk::get_biome_at(Vector3i local_position) {
    return biome[position_to_index(local_position)];
}

void Chunk::remove_block_at(Vector3i global_position, bool remesh_later) {
    Vector3i block_position = global_position - Vector3i(get_global_position());
    uint64_t array_index = position_to_index(block_position);

    if (blocks[array_index] == 0) {
        return;
    }

    Ref<Block> block_type = block_types[blocks[array_index]];

    // This block has a living component
    if (block_type->living_block_path != "") {
        world->kill_block(global_position);
    }

    // Kill foliage above this block as well, unless they have the float exception
    bool above_must_break = false;
    if (world->is_position_loaded(global_position + Vector3(0, 1, 0))) {
        int above_index = get_block_index_at_safe(block_position + Vector3(0, 1, 0));
        above_must_break = is_block_foliage[above_index] && !is_block_floating_foliage[above_index];
    }

    if (above_must_break) {
        // Living blocks
        if (world->get_living_block_at(global_position + Vector3(0, 1, 0)) != nullptr) {
            world->kill_block(global_position + Vector3(0, 1, 0));
        }

        // Remeshing
        if (in_bounds(block_position + Vector3(0, 1, 0))) {
            // Do not call remove_block_at again, as this would generate the mesh twice
            blocks[position_to_index(block_position + Vector3(0, 1, 0))] = 0;
            block_count--;
        } else {
            world->get_chunk_at(global_position + Vector3(0, 1, 0))->remove_block_at(global_position + Vector3(0, 1, 0), false);
        }
    }

    water_chunk_wake_set(block_position, true, true);

    blocks[array_index] = 0;
    block_count--;
    modified = true;
    dirty = true;

    call_deferred("validate_fire_at", block_position, true);

    if (!remesh_later) {
        generate_mesh(true, get_global_position());
        generate_water_surface_mesh(false, get_global_position(), true);
    }
}

bool Chunk::place_block_at(Vector3i global_position, uint32_t block_index, bool immediate_remesh) {
    Vector3i block_position = global_position - Vector3i(get_global_position());
    uint64_t existing_block_index = blocks[position_to_index(block_position)];

    if (existing_block_index != 0 && !is_block_foliage[existing_block_index] || block_index == 0) {
        return false;
    }

    water_chunk_wake_set(block_position, true, true);

    if (!is_block_foliage[block_index]) {
        set_water_at(block_position, 0);
    }

    if (world->get_living_block_at(global_position) != nullptr) {
        world->kill_block(global_position);
    }

    blocks[position_to_index(block_position)] = block_index;
    block_count++;

    modified = true;
    dirty = true;

    call_deferred("validate_fire_at", block_position, true);

    if (immediate_remesh) {
        generate_mesh(true, get_global_position());
        generate_water_surface_mesh(false, get_global_position(), true);
    }

    return true;
}

void Chunk::generate_mesh(bool immediate, Vector3 global_position) {
    // Zeroth pass (collision)
    vertices.clear();
    greedy_mesh_generation(false, false, true, global_position);
    collision_vertices = to_godot_array(vertices);

    // First pass (opaque objects)
    vertices.clear();
    normals.clear();
    uvs.clear();
    uvs2.clear();

    if (block_count > 0) {
        greedy_mesh_generation(false, false, false, global_position);
    }

    // Package data into an ArrayMesh
    opaque_mesh_data.clear();
    if (vertices.size() > 0) {
        opaque_mesh_data.resize(ArrayMesh::ARRAY_MAX);
        opaque_mesh_data[ArrayMesh::ARRAY_VERTEX] =  to_godot_array(vertices);
        opaque_mesh_data[ArrayMesh::ARRAY_NORMAL] =  to_godot_array(normals);
        opaque_mesh_data[ArrayMesh::ARRAY_TEX_UV] =  to_godot_array(uvs);
        opaque_mesh_data[ArrayMesh::ARRAY_TEX_UV2] = to_godot_array(uvs2);
    }
    
    // Second pass (transparent objects)

    vertices.clear();
    normals.clear();
    uvs.clear();
    uvs2.clear();

    if (block_count > 0) {
        greedy_mesh_generation(true, false, false, global_position);
    }

    transparent_mesh_data.clear();
    if (vertices.size() > 0) {
        transparent_mesh_data.resize(ArrayMesh::ARRAY_MAX);
        transparent_mesh_data[ArrayMesh::ARRAY_VERTEX] =  to_godot_array(vertices);
        transparent_mesh_data[ArrayMesh::ARRAY_NORMAL] =  to_godot_array(normals);
        transparent_mesh_data[ArrayMesh::ARRAY_TEX_UV] =  to_godot_array(uvs);
        transparent_mesh_data[ArrayMesh::ARRAY_TEX_UV2] = to_godot_array(uvs2);
    }

    // Third pass (foliage)
    vertices.clear();
    normals.clear();
    uvs.clear();
    uvs2.clear();

    if (block_count > 0) {
        foliage_mesh_generation(global_position);
    }

    foliage_mesh_data.clear();
    if (vertices.size() > 0) {
        foliage_mesh_data.resize(ArrayMesh::ARRAY_MAX);
        foliage_mesh_data[ArrayMesh::ARRAY_VERTEX] =  to_godot_array(vertices);
        foliage_mesh_data[ArrayMesh::ARRAY_NORMAL] =  to_godot_array(normals);
        foliage_mesh_data[ArrayMesh::ARRAY_TEX_UV] =  to_godot_array(uvs);
        foliage_mesh_data[ArrayMesh::ARRAY_TEX_UV2] = to_godot_array(uvs2);
    }

    if (immediate) {
        finalize_main_mesh_loading();
    }
}

void Chunk::generate_water_mesh(bool clear, Vector3 global_position, bool immediate) {
    // Water mesh
    water_mesh_data.instantiate();

    vertices.clear();
    normals.clear();
    uvs.clear();
    uvs2.clear();

    if (water_count > 0) {
        greedy_mesh_generation(false, true, false, global_position);
    }

    Array arrays;
    arrays.resize(ArrayMesh::ARRAY_MAX);

    PackedVector3Array godot_vertices = to_godot_array(vertices);
    PackedVector3Array godot_normals = to_godot_array(normals);
    PackedVector2Array godot_uvs = to_godot_array(uvs);
    PackedVector2Array godot_uvs2 = to_godot_array(uvs2);

    arrays[ArrayMesh::ARRAY_VERTEX] = godot_vertices;
    arrays[ArrayMesh::ARRAY_NORMAL] = godot_normals;
    arrays[ArrayMesh::ARRAY_TEX_UV] = godot_uvs;
    arrays[ArrayMesh::ARRAY_TEX_UV2] = godot_uvs2;
    if (vertices.size() > 0) {
        water_mesh_data->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);
    }

    if (immediate) {
        finalize_water_mesh_loading();
    }
    
}

void Chunk::generate_water_surface_mesh(bool remove, Vector3 global_position, bool immediate) {
    // Check that chunk above is loaded for regular meshing
    if (!remove && !world->is_position_loaded(global_position + Vector3(8, 8 + 16, 8))) {
        return;
    }

    // Surface mesh
    water_surface_mesh_data.instantiate();

    vertices.clear();
    normals.clear();

    face_count = 0;

    if (!remove && water_count > 0) {
        for (uint64_t y = 0; y < CHUNK_SIZE_Y; y++) {
        for (uint64_t z = 0; z < CHUNK_SIZE_Z; z++) {
        for (uint64_t x = 0; x < CHUNK_SIZE_Y; x++) {
            if (get_water_at(Vector3i(x, y, z)) == 0) {
                continue;
            }

            if (get_water_at_safe(Vector3i(x, y + 1, z)) != 0) {
                continue;
            }

            if (get_water_at(Vector3i(x, y, z)) == 255 && get_block_index_at_safe(Vector3i(x, y + 1, z)) != 0) {
                continue;
            }

            Vector3 start = Vector3(x, y, z);
            Vector3 size = Vector3(1, get_water_at(start) / 255., 1);

            vertices.resize(vertices.size() + 6);
            normals.resize(normals.size() + 6);

            // Y, facing up
            vertices[face_count * 6 + 0] = start + Vector3(0, size.y, size.z);
            vertices[face_count * 6 + 1] = start + Vector3(0, size.y, 0);
            vertices[face_count * 6 + 2] = start + Vector3(size.x, size.y, 0);
            vertices[face_count * 6 + 3] = start + Vector3(0, size.y, size.z);
            vertices[face_count * 6 + 4] = start + Vector3(size.x, size.y, 0);
            vertices[face_count * 6 + 5] = start + Vector3(size.x, size.y, size.z);
            add_face_normals(Vector3(0, -1, 0));

            face_count++;
        }
        }
        }
    }

    Array arrays;

    PackedVector3Array godot_vertices = to_godot_array(vertices);
    PackedVector3Array godot_normals = to_godot_array(normals);

    arrays.resize(ArrayMesh::ARRAY_MAX);
    arrays[ArrayMesh::ARRAY_VERTEX] = godot_vertices;
    arrays[ArrayMesh::ARRAY_NORMAL] = godot_normals;

    if (vertices.size() > 0) {
        water_surface_mesh_data->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);
    }

    water_surface_meshed = !remove;

    if (immediate) {
        finalize_water_surface_mesh_loading();
    }
}

PackedVector3Array Chunk::to_godot_array(const std::vector<Vector3> &vec) {
    PackedVector3Array packed;
    packed.resize(vec.size());
    for (int i = 0; i < vec.size(); i++) {
        packed[i] = vec[i];
    }
    return packed;
}

PackedVector2Array Chunk::to_godot_array(const std::vector<Vector2> &vec) {
    PackedVector2Array packed;
    packed.resize(vec.size());
    for (int i = 0; i < vec.size(); i++) {
        packed.set(i, vec[i]);
    }
    return packed;
}

void Chunk::clear_collision() {
    Dictionary empty;
    empty["faces"] = PackedVector3Array();
    empty["backface_collision"] = false;
    ps->shape_set_data(collision_shape, empty);
}

// Keep track of block count and other state
void Chunk::calculate_block_statistics() {
    int32_t main_block_type = blocks[0];
    uniform = true;
    block_count = 0;
    water_count = 0;
    fire_count = 0;
    for (uint64_t i = 0; i < blocks.size(); i++) {
        int32_t index = blocks[i];
        uniform &= index == main_block_type;
        if (index > 0) {
            block_count++;
        }

        // Copied is_block_foliage from World here... was causing strange crashes
        if (index != 0 && !is_block_foliage[index]) {
            water[i] = 0;
        }
        water_count += water[i];
        fire_count += fire[i];
    }
}

void Chunk::make_visible() {
    call_deferred("set_visible", true);
}

void Chunk::finalize_main_mesh_loading() {
    Dictionary data;
    data["faces"] = collision_vertices;
    data["backface_collision"] = false;
    ps->shape_set_data(collision_shape, data);
    ps->body_set_state(collision_body, PhysicsServer3D::BODY_STATE_TRANSFORM, Transform3D(Basis(), get_position()));

    Ref<ArrayMesh> main_array_mesh;
    Ref<ArrayMesh> foliage_array_mesh;
    main_array_mesh.instantiate();
    foliage_array_mesh.instantiate();

    int material_idx = 0;
    if (opaque_mesh_data.size() > 0) {
        main_array_mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, opaque_mesh_data);
        main_array_mesh->surface_set_material(material_idx, block_material);
        material_idx += 1;
    }

    if (transparent_mesh_data.size() > 0) {
        main_array_mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, transparent_mesh_data);
        main_array_mesh->surface_set_material(material_idx, transparent_block_material);
        material_idx += 1;
    }

    if (foliage_mesh_data.size() > 0) {
        foliage_array_mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, foliage_mesh_data);
    } 

    foliage_mesh->set_mesh(foliage_array_mesh);
    set_mesh(main_array_mesh);
}

void Chunk::finalize_water_mesh_loading() {
    water_mesh->set_mesh(water_mesh_data);
}

void Chunk::finalize_water_surface_mesh_loading() {
    water_mesh_surface->set_mesh(water_surface_mesh_data); 
}

// Add all cross-faces for foliage to our mesh
void Chunk::foliage_mesh_generation(Vector3 global_position) {
    face_count = 0;

    foliage_rng->set_seed((int32_t) (global_position.x * 134352.419 + global_position.y * 15392.14 + global_position.z * 49120.0));

    for (uint64_t y = 0; y < CHUNK_SIZE_Y; y++) {
    for (uint64_t z = 0; z < CHUNK_SIZE_Z; z++) {
    for (uint64_t x = 0; x < CHUNK_SIZE_Y; x++) {
        int32_t index = get_block_index_at(Vector3i(x, y, z));

        Vector3 offset = (1.0 - 0.707) * Vector3(foliage_rng->randf(), 0, foliage_rng->randf());

        if (!is_block_foliage[index]) {
            continue;
        }

        add_cross_face(offset + Vector3(x, y, z), index);
    }
    }
    }
}

// Fill vertex, normal, and uv arrays with proper triangles (using the greedy meshing algorithm)
void Chunk::greedy_mesh_generation(bool transparent, bool water_pass, bool collision_pass, Vector3 global_position) {
    for (uint64_t i = 0; i < CHUNK_SIZE_X * CHUNK_SIZE_Z * CHUNK_SIZE_Y; i++) {
        visited[i] = false;
    }

    face_count = 0;

    for (uint64_t y = 0; y < CHUNK_SIZE_Y; y++) {
    for (uint64_t z = 0; z < CHUNK_SIZE_Z; z++) {
    for (uint64_t x = 0; x < CHUNK_SIZE_Y; x++) {
        int32_t array_index = position_to_index(Vector3(x, y, z));

        if (visited[array_index]) {
            continue;
        }

        int32_t index = blocks[array_index];

        if (!water_pass) {
            // Skip air and foliage
            if (index == 0 || is_block_foliage[index]) {
                continue;
            }

            // Skip transparent blocks during a solid pass and vice versa
            if (!collision_pass) {
                current_greedy_block = index;
                current_greedy_block_real = index;
                bool block_transparent = is_block_transparent[index];
                if (block_transparent != transparent)  {
                    continue;
                }
            } else {
                // Skip non collision blocks in a collision pass
                if (is_block_non_collision[index]) {
                    continue;
                }
                current_greedy_block = 1; // Only distinguish between solid and not solid for collision pass
                current_greedy_block_real = index;
            }
        } else {
            current_greedy_block = water[array_index];
            current_greedy_block_real = water[array_index];

            // Skip empty water cells
            if (current_greedy_block == 0) {
                continue;
            }

            if (y < CHUNK_SIZE_Y - 1 && water[position_to_index(Vector3(x, y + 1, z))] > 0) {
                current_greedy_block = 255;
            }
        }

        Vector3i start = Vector3i(x, y, z);
        Vector3 size;

        // Use greedy meshing to find a rectangular prism size
        if (water_pass) {
            uint8_t water_level = current_greedy_block;

            if (water_level == 255) {
                size = greedy_scan(start, water_pass, collision_pass);
            } else {
                size = Vector3(1, water_level / 255., 1);
            }
        } else if (transparent) {
            size = Vector3(1, 1, 1);
        } else {
            size = greedy_scan(start, water_pass, collision_pass);
        }

        add_rectangular_prism(start, size, water_pass, collision_pass, current_greedy_block_real);
    }
    }
    }
}

// Greedily find the size of the largest prism we can add to our mesh
Vector3i Chunk::greedy_scan(Vector3i start, bool water_pass, bool collision_pass) {
    Vector3i size = Vector3i(1, 1, 1);
    while (!greedy_invalid(start + Vector3i(size.x, 0, 0), water_pass, collision_pass)) {
        visited[position_to_index(start + Vector3i(size.x, 0, 0))] = true;
        size.x++;
    }

    bool axis_done = false;
    while (!axis_done) {
        for (uint8_t x = 0; x < uint8_t(size.x); x++) {
            if (greedy_invalid(start + Vector3i(x, 0, size.z), water_pass, collision_pass)) {
                axis_done = true;
                break;
            }
        }
        if (axis_done) break;
        for (uint8_t x = 0; x < uint8_t(size.x); x++) {
            visited[position_to_index(start + Vector3i(x, 0, size.z))] = true;
        }
        size.z++;
    }

    axis_done = false;
    while (!axis_done) {
        for (uint8_t x = 0; x < uint8_t(size.x); x++) {
            for (uint8_t z = 0; z < uint8_t(size.z); z++) {
                if (greedy_invalid(start + Vector3i(x, size.y, z), water_pass, collision_pass)) {
                    axis_done = true;
                    break;
                }
            }
        }
        if (axis_done) break;
        for (uint8_t x = 0; x < uint8_t(size.x); x++) {
            for (uint8_t z = 0; z < uint8_t(size.z); z++) {
                visited[position_to_index(start + Vector3i(x, size.y, z))] = true;
            }
        }
        size.y++;
    }
    return size;
}

// Check if a position contains a block that can be merged with our current greedy scan
bool Chunk::greedy_invalid(Vector3i position, bool water_pass, bool collision_pass) {
    if (!in_bounds(position) || visited[position_to_index(position)]) {
        return true;
    }

    if (water_pass) {
        uint8_t water_level = water[position_to_index(position)];

        return water_level == 0 || water_level != current_greedy_block;
    } else if (collision_pass) {
        int index = get_block_index_at(position);

        return index == 0 || is_block_foliage[index] || is_block_non_collision[index];
    } else {
        int index = get_block_index_at(position);

        if (index == 0) {
            return true;
        }

        if (index == current_greedy_block) {
            return false;
        }

        bool fully_covered =
            in_bounds(Vector3i(+1, 0, 0) + position) && !is_block_transparent[get_block_index_at(Vector3i(+1, 0, 0) + position)] &&
            in_bounds(Vector3i(-1, 0, 0) + position) && !is_block_transparent[get_block_index_at(Vector3i(-1, 0, 0) + position)] &&
            in_bounds(Vector3i(0, +1, 0) + position) && !is_block_transparent[get_block_index_at(Vector3i(0, +1, 0) + position)] &&
            in_bounds(Vector3i(0, -1, 0) + position) && !is_block_transparent[get_block_index_at(Vector3i(0, -1, 0) + position)] &&
            in_bounds(Vector3i(0, 0, +1) + position) && !is_block_transparent[get_block_index_at(Vector3i(0, 0, +1) + position)] &&
            in_bounds(Vector3i(0, 0, -1) + position) && !is_block_transparent[get_block_index_at(Vector3i(0, 0, -1) + position)];

        return !fully_covered;
    }
}

// Adds vertices, uvs, and normals for a rectangular prism to our mesh
void Chunk::add_rectangular_prism(Vector3 start, Vector3 size, bool water_pass, bool collision_pass, uint64_t data) {
    vertices.resize(vertices.size() + 36);
    if (!collision_pass) {
        normals.resize(normals.size() + 36);
        uvs.resize(uvs.size() + 36);
        uvs2.resize(uvs2.size() + 36);
    }

    // Y, facing up
    vertices[face_count * 6 + 0] = start + Vector3(0, size.y, size.z);
    vertices[face_count * 6 + 1] = start + Vector3(0, size.y, 0);
    vertices[face_count * 6 + 2] = start + Vector3(size.x, size.y, 0);
    vertices[face_count * 6 + 3] = start + Vector3(0, size.y, size.z);
    vertices[face_count * 6 + 4] = start + Vector3(size.x, size.y, 0);
    vertices[face_count * 6 + 5] = start + Vector3(size.x, size.y, size.z);
    if (!collision_pass) {
        add_face_normals(Vector3(0, 1, 0));
        uint64_t id_2 = 0;
        if (!water_pass) {
            id_2 = is_block_top_face_randomized[data] ? 1 : 0;
        }
        add_face_uvs(water_pass ? data : data * 6, id_2, Vector2i(size.x, size.z));
    }

    face_count++;

    // Y, facing down
    vertices[face_count * 6 + 0] = start + Vector3(0, 0, 0);
    vertices[face_count * 6 + 1] = start + Vector3(0, 0, size.z);
    vertices[face_count * 6 + 2] = start + Vector3(size.x, 0, size.z);
    vertices[face_count * 6 + 3] = start + Vector3(0, 0, 0);
    vertices[face_count * 6 + 4] = start + Vector3(size.x, 0, size.z);
    vertices[face_count * 6 + 5] = start + Vector3(size.x, 0, 0);
    if (!collision_pass) {
        add_face_normals(Vector3(0, -1, 0));
        add_face_uvs(water_pass ? data : data * 6 + 1, 0, Vector2i(size.x, size.z));
    }

    face_count++;

    // Z, facing back
    vertices[face_count * 6 + 0] = start + Vector3(size.x, 0, 0);
    vertices[face_count * 6 + 1] = start + Vector3(size.x, size.y, 0);
    vertices[face_count * 6 + 2] = start + Vector3(0, size.y, 0);
    vertices[face_count * 6 + 3] = start + Vector3(size.x, 0, 0);
    vertices[face_count * 6 + 4] = start + Vector3(0, size.y, 0);
    vertices[face_count * 6 + 5] = start + Vector3(0, 0, 0);
    if (!collision_pass) {
        add_face_normals(Vector3(0, 0, -1));
        add_face_uvs(water_pass ? data : data * 6 + 2, 0, Vector2i(size.x, size.y));
    }

    face_count++;

    // Z, facing forward
    vertices[face_count * 6 + 0] = start + Vector3(0, 0, size.z);
    vertices[face_count * 6 + 1] = start + Vector3(0, size.y, size.z);
    vertices[face_count * 6 + 2] = start + Vector3(size.x, size.y, size.z);
    vertices[face_count * 6 + 3] = start + Vector3(0, 0, size.z);
    vertices[face_count * 6 + 4] = start + Vector3(size.x, size.y, size.z);
    vertices[face_count * 6 + 5] = start + Vector3(size.x, 0, size.z);
    if (!collision_pass) {
        add_face_normals(Vector3(0, 0, 1));
        add_face_uvs(water_pass ? data : data * 6 + 3, 0, Vector2i(size.x, size.y));
    }

    face_count++;

    // X, facing left
    vertices[face_count * 6 + 0] = start + Vector3(0, 0, 0);
    vertices[face_count * 6 + 1] = start + Vector3(0, size.y, 0);
    vertices[face_count * 6 + 2] = start + Vector3(0, size.y, size.z);
    vertices[face_count * 6 + 3] = start + Vector3(0, 0, 0);
    vertices[face_count * 6 + 4] = start + Vector3(0, size.y, size.z);
    vertices[face_count * 6 + 5] = start + Vector3(0, 0, size.z);
    if (!collision_pass) {
        add_face_normals(Vector3(-1, 0, 0));
        add_face_uvs(water_pass ? data : data * 6 + 4, 0, Vector2i(size.z, size.y));
    }

    face_count++;

    // X, facing right
    vertices[face_count * 6 + 0] = start + Vector3(size.x, 0, size.z);
    vertices[face_count * 6 + 1] = start + Vector3(size.x, size.y, size.z);
    vertices[face_count * 6 + 2] = start + Vector3(size.x, size.y, 0);
    vertices[face_count * 6 + 3] = start + Vector3(size.x, 0, size.z);
    vertices[face_count * 6 + 4] = start + Vector3(size.x, size.y, 0);
    vertices[face_count * 6 + 5] = start + Vector3(size.x, 0, 0);
    if (!collision_pass) {
        add_face_normals(Vector3(1, 0, 0));
        add_face_uvs(water_pass ? data : data * 6 + 5, 0, Vector2i(size.z, size.y));
    }

    face_count++;
}

// Adds vertices, uvs, and normals for two perpendicular planes to our mesh
// Each plane has both front and back faces for proper culling
void Chunk::add_cross_face(Vector3 start, int32_t block_index) {
    vertices.resize(vertices.size() + 6 * 4);
    normals.resize(normals.size() + 6 * 4);
    uvs.resize(uvs.size() + 6 * 4);
    uvs2.resize(uvs2.size() + 6 * 4);
    Vector3 size = Vector3(0.707, 1, 0.707);

    // Face 1 (front)
    vertices[face_count * 6 + 0] = start + Vector3(0, 0, 0);
    vertices[face_count * 6 + 1] = start + Vector3(0, size.y, 0);
    vertices[face_count * 6 + 2] = start + Vector3(size.x, size.y, size.z);
    vertices[face_count * 6 + 3] = start + Vector3(0, 0, 0);
    vertices[face_count * 6 + 4] = start + Vector3(size.x, size.y, size.z);
    vertices[face_count * 6 + 5] = start + Vector3(size.x, 0, size.z);
    add_face_normals(Vector3(0, 1, 0));
    add_face_uvs(block_index * 6 + 0, 0, Vector2i(1, 1));
    face_count++;

    // Face 1 (back)
    vertices[face_count * 6 + 0] = start + Vector3(size.x, size.y, size.z);
    vertices[face_count * 6 + 1] = start + Vector3(0, size.y, 0);
    vertices[face_count * 6 + 2] = start + Vector3(0, 0, 0);
    vertices[face_count * 6 + 3] = start + Vector3(size.x, 0, size.z);
    vertices[face_count * 6 + 4] = start + Vector3(size.x, size.y, size.z);
    vertices[face_count * 6 + 5] = start + Vector3(0, 0, 0);
    add_face_normals(Vector3(0, 1, 0));
    add_face_uvs_reversed(block_index * 6 + 0, 0, Vector2i(1, 1));
    face_count++;

    // Face 2 (front)
    vertices[face_count * 6 + 0] = start + Vector3(0, 0, size.z);
    vertices[face_count * 6 + 1] = start + Vector3(0, size.y, size.z);
    vertices[face_count * 6 + 2] = start + Vector3(size.x, size.y, 0);
    vertices[face_count * 6 + 3] = start + Vector3(0, 0, size.z);
    vertices[face_count * 6 + 4] = start + Vector3(size.x, size.y, 0);
    vertices[face_count * 6 + 5] = start + Vector3(size.x, 0, 0);
    add_face_normals(Vector3(0, 1, 0));
    add_face_uvs(block_index * 6 + 1, 0, Vector2i(1, 1));
    face_count++;

    // Face 2 (back)
    vertices[face_count * 6 + 0] = start + Vector3(size.x, size.y, 0);
    vertices[face_count * 6 + 1] = start + Vector3(0, size.y, size.z);
    vertices[face_count * 6 + 2] = start + Vector3(0, 0, size.z);
    vertices[face_count * 6 + 3] = start + Vector3(size.x, 0, 0);
    vertices[face_count * 6 + 4] = start + Vector3(size.x, size.y, 0);
    vertices[face_count * 6 + 5] = start + Vector3(0, 0, size.z);
    add_face_normals(Vector3(0, 1, 0));
    add_face_uvs_reversed(block_index * 6 + 1, 0, Vector2i(1, 1));
    face_count++;
}

// Adds UVs for a single quad
void Chunk::add_face_uvs(uint64_t id, uint64_t id_2, Vector2i scale) {
    Vector2i up = Vector2i(0, scale.y);
    Vector2i side = Vector2i(scale.x, 0);
    uvs[face_count * 6 + 0] = up;
	uvs[face_count * 6 + 1] = Vector2i(0, 0);
	uvs[face_count * 6 + 2] = side;
	uvs[face_count * 6 + 3] = up;
    uvs[face_count * 6 + 4] = side;
	uvs[face_count * 6 + 5] = scale;
    Vector2i id_vector = Vector2i(id_2, id);
    for (uint8_t i = 0; i < 6; i++) uvs2[face_count * 6 + i] = id_vector;
}


// Adds UVs for a single quad with reversed winding
void Chunk::add_face_uvs_reversed(uint64_t id, uint64_t id_2, Vector2i scale) {
    Vector2i up = Vector2i(0, scale.y);
    Vector2i side = Vector2i(scale.x, 0);
    // Reversed order to match reversed vertices
    uvs[face_count * 6 + 0] = side;      // was up
    uvs[face_count * 6 + 1] = Vector2i(0, 0);  // stays same
    uvs[face_count * 6 + 2] = up;        // was side
    uvs[face_count * 6 + 3] = scale;     // was up
    uvs[face_count * 6 + 4] = side;      // stays same
    uvs[face_count * 6 + 5] = up;        // was scale
    Vector2i id_vector = Vector2i(id_2, id);
    for (uint8_t i = 0; i < 6; i++) uvs2[face_count * 6 + i] = id_vector;
}

// Adds normals for a single quad
void Chunk::add_face_normals(Vector3i normal) {
    for (uint8_t i = 0; i < 6; i++) normals[face_count * 6 + i] = normal;
}

void Chunk::water_chunk_wake_set(Vector3i local_position, bool awake, bool surround) {
    if (!in_bounds(local_position)) {
        Vector3i global_position = local_position + get_global_position();

        if (!world->is_position_loaded(global_position)) {
            return;
        }

        Chunk* chunk = world->get_chunk_at(global_position);
        local_position = global_position - chunk->get_global_position();

        Vector3i w = local_position / Vector3i(WATER_CHUNK_SIZE_X, WATER_CHUNK_SIZE_Y, WATER_CHUNK_SIZE_Z);
        chunk->water_chunk_awake[uint64_t(w.x) + uint64_t(w.z) * CHUNK_SIZE_X / WATER_CHUNK_SIZE_X + uint64_t(w.y) * CHUNK_SIZE_X / WATER_CHUNK_SIZE_X * CHUNK_SIZE_Z / WATER_CHUNK_SIZE_Z] = awake ? 5 : 0;

        // If this chunk's water doesn't update, we'd still like its surface to be remeshed
        if (chunk->get_global_position().y < get_global_position().y) {
            chunk->water_surface_meshed = false;
        }
    } else {
        Vector3i w = local_position / Vector3i(WATER_CHUNK_SIZE_X, WATER_CHUNK_SIZE_Y, WATER_CHUNK_SIZE_Z);
        water_chunk_awake[uint64_t(w.x) + uint64_t(w.z) * CHUNK_SIZE_X / WATER_CHUNK_SIZE_X + uint64_t(w.y) * CHUNK_SIZE_X / WATER_CHUNK_SIZE_X * CHUNK_SIZE_Z / WATER_CHUNK_SIZE_Z] = awake ? 5 : 0;
    }

    if (surround) {
        water_chunk_wake_set(local_position + Vector3(+1, 0, 0), awake, false);
        water_chunk_wake_set(local_position + Vector3(-1, 0, 0), awake, false);
        water_chunk_wake_set(local_position + Vector3(0, +1, 0), awake, false);
        water_chunk_wake_set(local_position + Vector3(0, -1, 0), awake, false);
        water_chunk_wake_set(local_position + Vector3(0, 0, +1), awake, false);
        water_chunk_wake_set(local_position + Vector3(0, 0, -1), awake, false);
    }
}

uint8_t Chunk::get_water_at(Vector3i local_position) {
    return water[position_to_index(local_position)];
}

uint8_t Chunk::get_water_at_safe(Vector3i local_position) {
    if (in_bounds(local_position)) {
        return get_water_at(local_position);
    } else {
        Vector3i global_position = local_position + get_global_position();
        if (!world->is_position_loaded(global_position)) {
            return 0;
        }
        Chunk* chunk = world->get_chunk_at(global_position);
        return chunk->get_water_at(global_position - chunk->get_global_position());
    }
}

void Chunk::set_water_at(Vector3i local_position, uint8_t water_level) {
    if (in_bounds(local_position)) {
        uint64_t array_index = position_to_index(local_position);

        // Do not place water over non-air and non-foliage blocks
        if (water_level > 0 && world->is_block_index_solid(blocks[array_index])) {
            return;
        }

        if (water[array_index] != water_level) {
            water_chunk_wake_set(local_position, true, true);
            modified = true;
            dirty = true;
            water_updated = 3;
            water_count += water_level - (int16_t) water[array_index];
        }

        water[array_index] = water_level;

        validate_fire_at(local_position, true);
    } else {
        Vector3i global_position = local_position + get_global_position();

        if (!world->is_position_loaded(global_position)) {
            return;
        }

        Chunk* chunk = world->get_chunk_at(global_position);
        chunk->set_water_at(global_position - chunk->get_global_position(), water_level);
    }
}

void Chunk::simulate_water() {
    // Stop simulating if too many subchunks have already been simulated this frame
    if (world->simulated_water_subchunks >= World::MAX_WATER_SUBCHUNKS_PER_FRAME) {
        return;
    }

    for (uint16_t i = 0; i < water_chunk_awake_buffer.size(); i++) {
        water_chunk_awake_buffer[i] = water_chunk_awake[i];
        if (water_chunk_awake[i] > 0) {
            water_chunk_awake[i]--;
        }
    }

    Vector3 global_position = get_global_position();

    const int subchunks_x = CHUNK_SIZE_X / WATER_CHUNK_SIZE_X;
    const int subchunks_y = CHUNK_SIZE_Y / WATER_CHUNK_SIZE_Y;
    const int subchunks_z = CHUNK_SIZE_Z / WATER_CHUNK_SIZE_Z;
    const int subchunk_stride_x = 1;
    const int subchunk_stride_z = subchunks_x;
    const int subchunk_stride_y = subchunks_x * subchunks_z;

    // Diffusion step
    for (int8_t cy = 0; cy < subchunks_y; cy++) {
    for (int8_t cz = 0; cz < subchunks_z; cz++) {
    for (int8_t cx = 0; cx < subchunks_x; cx++) {
        if (water_chunk_awake_buffer[cx * subchunk_stride_x + cz * subchunk_stride_z + cy * subchunk_stride_y] == 0) continue;

        for (uint8_t y = cy * WATER_CHUNK_SIZE_Y; y < (cy + 1) * WATER_CHUNK_SIZE_Y; y++) {
        for (uint8_t z = cz * WATER_CHUNK_SIZE_Z; z < (cz + 1) * WATER_CHUNK_SIZE_Z; z++) {
        for (uint8_t x = cx * WATER_CHUNK_SIZE_X; x < (cx + 1) * WATER_CHUNK_SIZE_X; x++) {
            if ((x + 3 * z) % 5 != water_shuffle) {
                continue;
            }

            if (world->is_block_index_solid(get_block_index_at(Vector3i(x, y, z)))) {
                continue;
            }

            if (world->is_position_loaded(global_position + Vector3i(x, y - 1, z)) && get_water_at_safe(Vector3i(x, y - 1, z)) < 255 && !world->is_block_index_solid(get_block_index_at_safe(Vector3i(x, y - 1, z)))) {
                continue;
            }

            // Measuring water 
            uint8_t water_count = 1;
            uint8_t heavy_count = 0;
            uint16_t w_0 = get_water_at(Vector3i(x, y, z));

            uint16_t w_1 = 0;
            uint16_t w_2 = 0;
            uint16_t w_3 = 0;
            uint16_t w_4 = 0;

            bool valid_right = world->is_position_loaded(global_position + Vector3i(x + 1, y, z + 0)) && !world->is_block_index_solid(get_block_index_at_safe(Vector3i(x + 1, y, z + 0)));
            bool valid_left  = world->is_position_loaded(global_position + Vector3i(x - 1, y, z + 0)) && !world->is_block_index_solid(get_block_index_at_safe(Vector3i(x - 1, y, z + 0)));
            bool valid_up    = world->is_position_loaded(global_position + Vector3i(x + 0, y, z + 1)) && !world->is_block_index_solid(get_block_index_at_safe(Vector3i(x + 0, y, z + 1)));
            bool valid_down  = world->is_position_loaded(global_position + Vector3i(x + 0, y, z - 1)) && !world->is_block_index_solid(get_block_index_at_safe(Vector3i(x + 0, y, z - 1)));

            if (valid_right) { w_1 = get_water_at_safe(Vector3i(x + 1, y, z + 0)); water_count++; }
            if (valid_left)  { w_2 = get_water_at_safe(Vector3i(x - 1, y, z + 0)); water_count++; }
            if (valid_up)    { w_3 = get_water_at_safe(Vector3i(x + 0, y, z + 1)); water_count++; }
            if (valid_down)  { w_4 = get_water_at_safe(Vector3i(x + 0, y, z - 1)); water_count++; }

            if (w_0 > HEAVY) heavy_count++;
            if (w_1 > HEAVY) heavy_count++;
            if (w_2 > HEAVY) heavy_count++;
            if (w_3 > HEAVY) heavy_count++;
            if (w_4 > HEAVY) heavy_count++;

            // Averaging 
            float average_f = UtilityFunctions::round((w_0 + w_1 + w_2 + w_3 + w_4 + EXTRA * heavy_count) / water_count);
            if (average_f > 255.0f) average_f = 255.0f;
            if (average_f < EVAPORATE) average_f = 0.0f;

            // Move toward average
            const float diffusion_rate = 0.45f;
            auto diffuse = [&](uint16_t w, Vector3i pos, bool valid) {
                if (!valid) return;
                float new_val_f = w + diffusion_rate * (average_f - w);
                if (new_val_f < EVAPORATE) new_val_f = 0.0f;
                uint8_t new_val = (uint8_t) new_val_f;
                if (new_val >= 253) {
                    new_val = 255; // Fixes little gaps
                }
                set_water_at(pos, new_val);
            };

            diffuse(w_0, Vector3i(x, y, z), true);
            diffuse(w_1, Vector3i(x + 1, y, z + 0), valid_right);
            diffuse(w_2, Vector3i(x - 1, y, z + 0), valid_left);
            diffuse(w_3, Vector3i(x + 0, y, z + 1), valid_up);
            diffuse(w_4, Vector3i(x + 0, y, z - 1), valid_down);

        }
        }
        }
    }
    }
    }


    // Gravity step
    for (int8_t cy = 0; cy < subchunks_y; cy++) {
    for (int8_t cz = 0; cz < subchunks_z; cz++) {
    for (int8_t cx = 0; cx < subchunks_x; cx++) {
        if (water_chunk_awake_buffer[cx * subchunk_stride_x + cz * subchunk_stride_z + cy * subchunk_stride_y] == 0) continue;

        world->simulated_water_subchunks++;

        for (uint8_t y = cy * WATER_CHUNK_SIZE_Y; y < (cy + 1) * WATER_CHUNK_SIZE_Y; y++) {
        for (uint8_t z = cz * WATER_CHUNK_SIZE_Z; z < (cz + 1) * WATER_CHUNK_SIZE_Z; z++) {
        for (uint8_t x = cx * WATER_CHUNK_SIZE_X; x < (cx + 1) * WATER_CHUNK_SIZE_X; x++) {
            if (world->is_block_index_solid(get_block_index_at(Vector3i(x, y, z)))) {
                continue;
            }

            uint8_t w0 = get_water_at(Vector3i(x, y, z));

            Chunk* source_chunk = this;
            Vector3i source_coord = Vector3i(x, y + 1, z);
            uint8_t ws;
            if (in_bounds(source_coord)) {
                ws = get_water_at(source_coord);
            } else {
                Vector3i water_global_position = source_coord + global_position;
                if (!world->is_position_loaded(water_global_position)) {
                    continue;
                } else {
                    source_chunk = world->get_chunk_at(water_global_position);
                    source_coord = water_global_position - source_chunk->get_global_position();
                    ws = source_chunk->get_water_at(source_coord);
                }
            }

            if (ws == 0) {
                continue;
            }

            if (world->is_block_index_solid(source_chunk->get_block_index_at(source_coord))) {
                continue;
            }

            uint16_t w0n = (uint16_t) (w0 + (ws > 32 ? ws * 0.45 : ws));
            if (w0n > 255) {
                w0n = 255;
            }

            uint8_t w0ni = (uint8_t) w0n;

            source_chunk->set_water_at(source_coord, ws - (w0ni - w0));

            if (w0n > 255) {
                w0n = 255;
            }

            w0ni = (uint8_t) w0n;

            set_water_at(Vector3i(x, y, z), w0ni);
        }
        }
        }

        // Floor chunk level
        if (cy == 0) {
            if (!world->is_position_loaded(global_position - Vector3(0, 1, 0))) {
                continue;
            }
            Chunk* below_chunk = world->get_chunk_at(global_position - Vector3(0, 1, 0));
            Vector3 below_global_position = below_chunk->get_global_position();

            for (uint8_t z = cz * WATER_CHUNK_SIZE_Z; z < (cz + 1) * WATER_CHUNK_SIZE_Z; z++) {
            for (uint8_t x = cx * WATER_CHUNK_SIZE_X; x < (cx + 1) * WATER_CHUNK_SIZE_X; x++) {
                Vector3i floor_local = Vector3i(x, -1, z);
                Vector3i floor_global = global_position + floor_local;
                floor_local = floor_global - below_global_position;
                if (get_water_at(Vector3i(x, 0, z)) > 0 && !world->is_block_index_solid(below_chunk->get_block_index_at(floor_local)) && below_chunk->get_water_at(floor_local) < 255) {
                    water_chunk_wake_set(Vector3i(x, 0, z), true, false);
                    below_chunk->water_chunk_wake_set(floor_local, true, false);
                    break;
                }
            }
            }
        }
    }
    }
    }
    water_shuffle = (water_shuffle + 1) % 5;
}

// Not thread safe
void Chunk::initialize_fire_visuals(Vector3 global_position) {
    for (uint64_t y = 0; y < CHUNK_SIZE_Y; y++) {
       for (uint64_t z = 0; z < CHUNK_SIZE_Z; z++) {
            for (uint64_t x = 0; x < CHUNK_SIZE_X; x++) {
                update_fire_visual(Vector3i(x, y, z), global_position, false);
            }
        }
    }
}

// Not thread safe
void Chunk::update_fire_visual(Vector3i local_position, Vector3 global_chunk_position, bool animate) {
    uint8_t fire = get_fire_at(local_position);
    Vector3i global_position = Vector3i(global_chunk_position) + local_position;

    if (fire == 0 && owned_fire_visuals.has(local_position)) {
        Node3D* fire_visual = Object::cast_to<Node3D>(owned_fire_visuals[local_position]);

        if (animate) {
            fire_visual->call_deferred("exit");
        } else {
            fire_visual->call_deferred("queue_free");
        }

        owned_fire_visuals.erase(local_position);
    } else if (fire > 0 && !owned_fire_visuals.has(local_position)) {
        Node3D* fire_visual = Object::cast_to<Node3D>(world->fire_visual_scene->instantiate());
        call_deferred("add_child", fire_visual);

        fire_visual->call_deferred("set_global_position", global_position);
        fire_visual->call_deferred("initialize");

        if (animate) {
            fire_visual->call_deferred("enter");
        }

        owned_fire_visuals[local_position] = fire_visual;
    }

    if (fire > 0 && owned_fire_visuals.has(local_position)) {
        Node3D* fire_visual = Object::cast_to<Node3D>(owned_fire_visuals[local_position]);
        fire_visual->call_deferred("update_faces");
    }
}

uint8_t Chunk::get_fire_at(Vector3i local_position) {
    return fire[position_to_index(local_position)];
}

// Not thread safe
void Chunk::set_fire_at(Vector3i local_position, uint8_t new_fire) {
    if (new_fire > 0 && !fire_eligible(local_position)) {
        return;
    }
    int16_t old_fire = fire[position_to_index(local_position)];
    fire_count += ((int16_t) new_fire) - old_fire;

    if (new_fire != old_fire) {
        modified = true;
        dirty = true;
    }

    fire[position_to_index(local_position)] = new_fire;
    update_fire_visual(local_position, get_global_position(), true);

    if (modified && new_fire > old_fire) {
        world->emit_signal("fire_spread", local_position + get_global_position());
    }
}

// Not thread safe
void Chunk::spread_fire(Vector3i local_position, uint8_t new_fire, bool force) {
    if (fire_count >= FIRE_LIMIT && new_fire > 0) {
        return;
    }
    if (in_bounds(local_position)) {
        Ref<Block> block_type = world->block_types[get_block_index_at(local_position)];
        if (fire_eligible(local_position) && (force || UtilityFunctions::randf() < FIRE_SPREAD_BASE * block_type->flammability)) {
            set_fire_at(local_position, new_fire);
        }
    } else {
        Vector3i global_position = local_position + get_global_position();
        if (!world->is_position_loaded(global_position)) {
            return;
        }
        Chunk* chunk = world->get_chunk_at(global_position);
        chunk->spread_fire(global_position - chunk->get_global_position(), new_fire, force);
    }
}

bool Chunk::fire_neighbor(Vector3i local_position) {
    if (in_bounds(local_position)) {
        return !world->is_block_index_solid(get_block_index_at(local_position));
    } else {
        Vector3i global_position = local_position + get_global_position();
        if (!world->is_position_loaded(global_position)) {
            return false;
        }
        Chunk* chunk = world->get_chunk_at(global_position);
        return !world->is_block_index_solid(chunk->get_block_index_at(global_position - chunk->get_global_position()));
    }
}

bool Chunk::fire_eligible(Vector3i local_position) {
    // Fire only exists on solid blocks with no water
    if (!world->is_block_index_solid(get_block_index_at(local_position))
    || get_water_at_safe(local_position + Vector3i(+1, 0, 0)) > 0
    || get_water_at_safe(local_position + Vector3i(-1, 0, 0)) > 0
    || get_water_at_safe(local_position + Vector3i(0, +1, 0)) > 0
    || get_water_at_safe(local_position + Vector3i(0, -1, 0)) > 0
    || get_water_at_safe(local_position + Vector3i(0, 0, +1)) > 0
    || get_water_at_safe(local_position + Vector3i(0, 0, -1)) > 0) {
        return false;
    }

    // Fire must be exposed to open air

    bool has_fire_neighbor = false;

    has_fire_neighbor = has_fire_neighbor || fire_neighbor(local_position + Vector3i(+1, 0, 0));
    has_fire_neighbor = has_fire_neighbor || fire_neighbor(local_position + Vector3i(-1, 0, 0));
    has_fire_neighbor = has_fire_neighbor || fire_neighbor(local_position + Vector3i(0, +1, 0));
    has_fire_neighbor = has_fire_neighbor || fire_neighbor(local_position + Vector3i(0, -1, 0));
    has_fire_neighbor = has_fire_neighbor || fire_neighbor(local_position + Vector3i(0, 0, +1));
    has_fire_neighbor = has_fire_neighbor || fire_neighbor(local_position + Vector3i(0, 0, -1));

    return has_fire_neighbor;
}

// Not thread safe
void Chunk::validate_fire_at(Vector3i local_position, bool deep) {
    if (in_bounds(local_position)) {
         if (fire_count > 0) {
            if (get_fire_at(local_position) > 0 && !fire_eligible(local_position)) {
                set_fire_at(local_position, 0);
            } else {
                update_fire_visual(local_position, get_global_position(), true);
            }
        }
    } else {
        Vector3i global_position = local_position + get_global_position();
        if (!world->is_position_loaded(global_position)) {
            return;
        }
        Chunk* chunk = world->get_chunk_at(global_position);
        chunk->validate_fire_at(global_position - chunk->get_global_position(), false);
    }

    if (deep) {
        validate_fire_at(local_position + Vector3i(+1, 0, 0), false);
        validate_fire_at(local_position + Vector3i(-1, 0, 0), false);
        validate_fire_at(local_position + Vector3i(0, +1, 0), false);
        validate_fire_at(local_position + Vector3i(0, -1, 0), false);
        validate_fire_at(local_position + Vector3i(0, 0, +1), false);
        validate_fire_at(local_position + Vector3i(0, 0, -1), false);
    }

}

// Not thread safe
void Chunk::simulate_fire() {
    if (fire_count == 0) {
        return;
    }

    for (int8_t cy = 0; cy < CHUNK_SIZE_Y / WATER_CHUNK_SIZE_Y; cy++) {
    for (int8_t cz = 0; cz < CHUNK_SIZE_Z / WATER_CHUNK_SIZE_Z; cz++) {
    for (int8_t cx = 0; cx < CHUNK_SIZE_X / WATER_CHUNK_SIZE_X; cx++) {
        if (UtilityFunctions::randf() >= SIMULATE_FIRE_CHUNK) {
            continue;
        }

        for (uint8_t y = cy * WATER_CHUNK_SIZE_Y; y < (cy + 1) * WATER_CHUNK_SIZE_Y; y++) {
        for (uint8_t z = cz * WATER_CHUNK_SIZE_Z; z < (cz + 1) * WATER_CHUNK_SIZE_Z; z++) {
        for (uint8_t x = cx * WATER_CHUNK_SIZE_X; x < (cx + 1) * WATER_CHUNK_SIZE_X; x++) {
            if (UtilityFunctions::randf() >= SIMULATE_FIRE) {
                continue;
            }

            Vector3i local_position = Vector3i(x, y, z);
            Ref<Block> block_type = world->block_types[get_block_index_at(local_position)];

            if (get_fire_at(local_position) == 0) {
                continue;
            }

            bool combusted = false;
            if (!block_type->unbreakable && !block_type->sustain_fire && block_type->flammability > 0.6 && UtilityFunctions::randf() < block_type->flammability * FIRE_COMBUST_BASE) {
                combusted = true;
                remove_block_at(get_global_position() + local_position, false);
            }

            if (!fire_eligible(local_position) || (!block_type->sustain_fire && UtilityFunctions::randf() < FIRE_DISSOLVE_BASE)) {
                set_fire_at(local_position, 0);
            }

            if (get_fire_at(local_position) > 0 || combusted) {
                spread_fire(local_position + Vector3i(+1, 0, 0), 1, combusted);
                spread_fire(local_position + Vector3i(-1, 0, 0), 1, combusted);
                spread_fire(local_position + Vector3i(0, +1, 0), 1, combusted);
                spread_fire(local_position + Vector3i(0, -1, 0), 1, combusted);
                spread_fire(local_position + Vector3i(0, 0, +1), 1, combusted);
                spread_fire(local_position + Vector3i(0, 0, -1), 1, combusted);
            }
        }
        }
        }
    }
    }
    }
}
