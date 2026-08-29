#include "../../include/structures/cellular_structure.h"
#include "../../include/world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void CellularStructure::_bind_methods() {
    BIND_REF_ARRAY_PROPERTY(CellularStructure, Texture2D, templates);
    BIND_REF_ARRAY_PROPERTY(CellularStructure, Block, cell_to_block_map);
    BIND_PROPERTY(CellularStructure, bool, override_terrain);
}

DEFINE_CONSTRUCTORS(CellularStructure);

void CellularStructure::generate_chunk_data(Chunk* chunk, Vector3i chunk_position) {
    uint16_t rng_seed = world->generator->seed;  
    for (int y = 0; y < Chunk::CHUNK_SIZE_Y; y++) {
    for (int z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
    for (int x = 0; x < Chunk::CHUNK_SIZE_X; x++) {
        Vector3i local_position = chunk_position - root_position + Vector3i(x, y, z) - Vector3i(64, 0, 64);
        if (local_position.x < 0 || local_position.y < 0 || local_position.z < 0 || 
            local_position.x >= BOUND_SIZE || local_position.y >= BOUND_SIZE || local_position.z >= BOUND_SIZE) {
                continue;
            }

        if (!override_terrain) {
            int base_block = chunk->get_block_index_at(Vector3i(x, y, z));
            if (base_block != 0 && !chunk->is_block_foliage[base_block]) {
                continue;
            }
        }
        
        int cell_id = cells[local_position.x + local_position.z * BOUND_SIZE + local_position.y * BOUND_SIZE * BOUND_SIZE];
        if (cell_id < 0) {
            continue;
        }

        Ref<Block> cell_block = cell_to_block_map[cell_id];
        chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = cell_block->index;
    }  
    }  
    }  
}

void CellularStructure::initialize() {
    uint16_t rng_seed = world->generator->seed;
    Random::scramble_rng_seed(&rng_seed, heart_position);

    Ref<Texture2D> template_texture = templates[Random::randi(rng_seed) % templates.size()];
    selected_template = template_texture->get_image();

    cells.resize(BOUND_SIZE * BOUND_SIZE * BOUND_SIZE);

    for (int i = 0; i < BOUND_SIZE; i++) {
    for (int j = 0; j < BOUND_SIZE; j++) {
        Color pixel = selected_template->get_pixel(i, j);
        if (pixel.a <= 0.5) {
            cells[i + j * BOUND_SIZE] = AIR;
        } else {
            cells[i + j * BOUND_SIZE] = 255 * pixel.r;
        }
    }
    }

    for (int k = 1; k < BOUND_SIZE; k++) {
        iterate(k);
    }
}

void CellularStructure::iterate(int layer) {
    for (int i = 0; i < BOUND_SIZE; i++) {
    for (int j = 0; j < BOUND_SIZE; j++) {
        int index = i + j * BOUND_SIZE + layer * BOUND_SIZE * BOUND_SIZE;
        cells[index] = update_rule(i, j, layer - 1, get_cell(i, j, layer - 1));
    }
    }
}

bool CellularStructure::in_bounds(int x, int y) {
    return x >= 0 && y >= 0 && x < BOUND_SIZE && y < BOUND_SIZE;
}

int CellularStructure::get_cell(int x, int y, int z) {
    return cells[x + y * BOUND_SIZE + z * BOUND_SIZE * BOUND_SIZE];
}

int CellularStructure::touch_count(int x, int y, int z, int type) {
    int count = 0;
    for(int ox = -1; ox <= 1; ox++) {
    for(int oy = -1; oy <= 1; oy++) {
        if (ox == 0 && oy == 0) continue;
        if (in_bounds(x + ox, y + oy) && get_cell(x + ox, y + oy, z) == type) {
            count += 1;
        }
    }
    }

    return count;
}

int CellularStructure::update_rule(int x, int y, int z, int type) {
    return type;
}

DEFINE_PROPERTY_GETTER_SETTER(CellularStructure, bool, override_terrain);
DEFINE_PROPERTY_GETTER_SETTER(CellularStructure, TypedArray<Texture2D>, templates);
DEFINE_PROPERTY_GETTER_SETTER(CellularStructure, TypedArray<Block>, cell_to_block_map);