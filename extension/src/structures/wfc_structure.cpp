#include "../../include/structures/wfc_structure.h"
#include "../../include/world.h"

using namespace godot;

void WfcStructure::_bind_methods() {
    BIND_PROPERTY(WfcStructure, Vector3i, tile_size);
    BIND_REF_ARRAY_PROPERTY(WfcStructure, WfcTile, tiles);
    BIND_REF_PROPERTY(WfcStructure, WfcTile, default_tile);
}

DEFINE_CONSTRUCTORS(WfcStructure);

int WfcStructure::Cell::entropy() const {
    return WfcStructure::popcount(mask);
}

bool WfcStructure::Cell::has(int i) const {
    return mask & ((uint64_t) 1 << i);
}

const WfcStructure::ConstraintDirection WfcStructure::directions[6] = {
    { Vector3i( 1,  0,  0), EAST, WEST },   // east
    { Vector3i(-1,  0,  0), WEST, EAST },   // west
    { Vector3i( 0,  1,  0), TOP, BOTTOM },  // top
    { Vector3i( 0, -1,  0), BOTTOM, TOP },  // bottom
    { Vector3i( 0,  0, -1), NORTH, SOUTH }, // north
    { Vector3i( 0,  0, +1), SOUTH, NORTH }  // south
};

inline int WfcStructure::popcount(uint64_t x) {
    int count = 0;
    while (x) {
        x &= (x - 1);
        count++;
    }
    return count;
}

inline uint32_t WfcStructure::get_side_mask(Ref<WfcTile> tile, int side) {
    switch (side) {
        case TOP:    return tile->get_t_mask();
        case BOTTOM: return tile->get_b_mask();
        case NORTH:  return tile->get_n_mask();
        case SOUTH:  return tile->get_s_mask();
        case EAST:   return tile->get_e_mask();
        case WEST:   return tile->get_w_mask();
    }
    return 0;
}

inline int WfcStructure::mask_to_index(uint64_t mask) {
    for (int i = 0; i < 64; i++) {
        if (mask & ((uint64_t) 1 << i)){
            return i;
        }
    }
    return -1;
}

// Get index of n-th set bit (0-based)
inline int WfcStructure::nth_set_bit(uint64_t mask, int n) {
    for (int i = 0; i < 64; i++) {
        if (mask & ((uint64_t) 1 << i)) {
            if (n == 0) {
                return i;
            }
            n -= 1;
        }
    }
    return -1;
}

void WfcStructure::initialize() {
    // Create directional variants
    TypedArray<WfcTile> to_expand;
    for (int i = 0; i < tiles.size(); i++) {
        Ref<WfcTile> tile = tiles[i];
        if (tile->directional_variants) {
            to_expand.push_back(tile);
        }
    }

    for (int i = 0; i < to_expand.size(); i++) {
        Ref<WfcTile> tile = to_expand[i];

        for (int j = 0; j < 3; j++) {
            Ref<WfcTile> new_tile = tile->duplicate(false);

            DecorationState::Direction new_direction = DecorationState::Direction::North;
            switch (j) {
                case 0: new_direction = DecorationState::Direction::East;  break;
                case 1: new_direction = DecorationState::Direction::South; break;
                case 2: new_direction = DecorationState::Direction::West;  break;
            }

            new_tile->direction = new_direction;

            // Rotate the mask
            for (int r = 0; r <= j; r++) {
                uint32_t temp_mask = new_tile->n_mask;
                new_tile->n_mask = new_tile->w_mask;
                new_tile->w_mask = new_tile->s_mask;
                new_tile->s_mask = new_tile->e_mask;
                new_tile->e_mask = temp_mask;
            }

            tiles.push_back(new_tile);
        }
    }

    // Generation algorithm

    int width = wfc_size.x;
    int height = wfc_size.y;
    int depth = wfc_size.z;

    // Each cell starts with all possibilities
    std::vector<Cell> superposition;
    superposition.resize(width * height * depth);
    for (int i = 0; i < superposition.size(); i++) {
        Cell new_cell;
        for (int j = 0; j < tiles.size(); j++) {
            new_cell.mask = new_cell.mask | ((uint64_t) 1 << j);
        }
        superposition[i] = new_cell;
    }

    // Now collapse loop
    uint16_t rng_seed = world->generator->seed;
    Random::scramble_rng_seed(&rng_seed, root_position);

    while (true) {
        int chosen_idx = -1;
        int min_entropy = INT_MAX;

        for (int i = 0; i < superposition.size(); i++) {
            Cell &cell = superposition[i];
            if (!cell.collapsed) {
                int entropy = cell.entropy();
                if (entropy > 1 && entropy < min_entropy) {
                    min_entropy = entropy;
                    chosen_idx = i;
                }
            }
        }
        if (chosen_idx == -1) {
            break;
        }

        Random::scramble_rng_seed(&rng_seed, root_position * chosen_idx);

        // Collapse a single cell
        Cell &chosen = superposition[chosen_idx];
        int choice = Random::randi(rng_seed) % chosen.entropy();
        int picked = nth_set_bit(chosen.mask, choice);
        chosen.mask = (uint64_t) 1 << picked;
        chosen.collapsed = true;

        // Propagation
        std::vector<int> frontier;
        frontier.push_back(chosen_idx);
        while (!frontier.empty()) {
            int idx = frontier.back();
            frontier.pop_back();

            int x = idx % width;
            int y = idx / (width * depth);
            int z = (idx / width) % depth;

            Cell &cell = superposition[idx];

            for (const ConstraintDirection &dir : directions) {
                int nx = x + dir.offset.x;
                int ny = y + dir.offset.y;
                int nz = z + dir.offset.z;

                if (nx < 0 || ny < 0 || nz < 0 || nx >= width || ny >= height || nz >= depth) {
                    continue;
                }

                int nidx = nx + nz * width + ny * width * depth;
                Cell &neighbor = superposition[nidx];

                uint64_t allowed = 0;

                // For each tile possibility in this cell
                for (int t = 0; t < tiles.size(); t++) {
                    if (!(cell.mask & ((uint64_t) 1 << t))) continue;

                    Ref<WfcTile> tile = tiles[t];
                    uint32_t mask = get_side_mask(tile, dir.side);

                    // For each candidate tile in the neighbor
                    for (int u = 0; u < tiles.size(); u++) {
                        if (!(neighbor.mask & ((uint64_t) 1 << u))) continue;

                        Ref<WfcTile> neighbor_tile = tiles[u];
                        uint32_t opp_mask = get_side_mask(neighbor_tile, dir.opposite);

                        if (mask & opp_mask) {
                            allowed |= ((uint64_t) 1 << u);
                        }
                    }
                }

                uint64_t before = neighbor.mask;
                neighbor.mask &= allowed;

                if (neighbor.mask != before) {
                    frontier.push_back(nidx);
                }
            }
        }
    }

    // Convert indices to tiles
    placed_tiles.clear();
    for (int i = 0; i < superposition.size(); i++) {
        Cell cell = superposition[i];
        Ref<WfcTile> tile_to_place;

        int index = mask_to_index(cell.mask);
        if (index >= tiles.size() || index < 0) {
            tile_to_place = default_tile;
        } else {
            tile_to_place = tiles[index];
        }

        placed_tiles.push_back(tile_to_place);
    }
}

void WfcStructure::generate_chunk_data(Chunk* chunk, Vector3i chunk_position) {
    for (int y = 0; y < Chunk::CHUNK_SIZE_Y; y++) {
    for (int z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
    for (int x = 0; x < Chunk::CHUNK_SIZE_X; x++) {
        Vector3i local_position = Vector3i(x, y, z);
        Vector3i position = local_position + chunk_position;
        Vector3i structure_local_position = position - root_position;

        Vector3i cell_position = structure_local_position / tile_size;
        cell_position -= wfc_position;

        if (cell_position.x < 0 || cell_position.y < 0 || cell_position.z < 0 || cell_position.x >= wfc_size.x || cell_position.y >= wfc_size.y || cell_position.z >= wfc_size.z) {
            continue;
        }

        int cell_index = cell_position.x + cell_position.z * wfc_size.x + cell_position.y * wfc_size.x * wfc_size.z;
        if (cell_index < 0 || cell_index >= placed_tiles.size()) {
            continue;
        }

        Ref<WfcTile> tile = placed_tiles[cell_index];
        Ref<Decoration> decoration = tile->decorations[0];
        DecorationState::Direction direction = tile->direction;

        Vector3i cell_corner_position = (cell_position + wfc_position) * tile_size;

        // Rotation (only work assuming a tile is a cube)
        Vector3i decoration_local_position = structure_local_position - cell_corner_position;
        int dx = decoration_local_position.x;
        int dy = decoration_local_position.y;
        int dz = decoration_local_position.z;

        int sx = decoration->size.x;
        int sz = decoration->size.z;

        int rx, ry, rz;
        switch (direction) {
            case DecorationState::North:
                rx = dx;
                ry = dy;
                rz = dz;
                break;
            case DecorationState::East:
                rx = dz;
                ry = dy;
                rz = sx - dx - 1;
                break;
            case DecorationState::South:
                rx = sx - dx - 1;
                ry = dy;
                rz = sz - dz - 1;
                break;
            case DecorationState::West:
                rx = sz - dz - 1;
                ry = dy;
                rz = dx;
                break;
            default:
                rx = dx;
                ry = dy;
                rz = dz;
                break;
        }
        int decoration_index = rx + rz * decoration->size.x + ry * decoration->size.x * decoration->size.z;

        if (decoration_index >= decoration->blocks.size() || decoration_index < 0) {
            continue;
        }

        chunk->blocks[Chunk::position_to_index(local_position)] = (int) world->block_id_to_index_map[(int) decoration->blocks[decoration_index]];
    }
    }
    }
}

DEFINE_PROPERTY_GETTER_SETTER(WfcStructure, Vector3i, tile_size);
DEFINE_PROPERTY_GETTER_SETTER(WfcStructure, TypedArray<WfcTile>, tiles);
DEFINE_PROPERTY_GETTER_SETTER(WfcStructure, Ref<WfcTile>, default_tile);