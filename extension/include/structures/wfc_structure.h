
#pragma once


#include "macros.h"
#include "structure.h"
#include "../decorations/wfc_tile.h"

namespace godot {

class WfcStructure : public Structure {
	DECLARE_CLASS(WfcStructure, Structure);

    // Holds potential tiles while collapsing wf
    struct Cell {
        uint64_t mask = 0;
        bool collapsed = false;

        int entropy() const;
        bool has(int i) const;
    };

    enum Side : int {
        TOP = 0, BOTTOM = 1, NORTH = 2, SOUTH = 3, EAST = 4, WEST = 5
    };

    struct ConstraintDirection {
        Vector3i offset;
        int side;
        int opposite;
    };

    static const ConstraintDirection directions[6];

    // All potential tiles to select from
    DECLARE_PROPERTY_NO_DEFAULT(TypedArray<WfcTile>, tiles);
    DECLARE_PROPERTY_NO_DEFAULT(Ref<WfcTile>, default_tile);
    Ref<WfcTile> seed_tile;

    // 3D flatenned array of placed tiles
    TypedArray<WfcTile> placed_tiles;

    // Bounding box (tiles)
    Vector3i wfc_position = Vector3i(2, 6, 2);
    Vector3i wfc_size = Vector3i(12, 7, 12);

    DECLARE_PROPERTY(Vector3i, tile_size, Vector3i(0, 0, 0));

    void initialize();
    void generate_chunk_data(Chunk* chunk, Vector3i chunk_position);

    // Helper methods
    static int popcount(uint64_t x);
    static uint32_t get_side_mask(Ref<WfcTile> tile, int side);
    static int mask_to_index(uint64_t mask);
    static int nth_set_bit(uint64_t mask, int n);
};

}