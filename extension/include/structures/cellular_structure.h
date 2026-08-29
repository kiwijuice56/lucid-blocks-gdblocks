
#pragma once


#include "macros.h"
#include "structure.h"

namespace godot {

class CellularStructure : public Structure {
	DECLARE_CLASS(CellularStructure, Structure);

    static const int AIR = -1;
    static const int64_t BOUND_SIZE = 128; // (In blocks)

    PackedInt32Array cells;

    DECLARE_PROPERTY_NO_DEFAULT(TypedArray<Block>, cell_to_block_map);
    DECLARE_PROPERTY_NO_DEFAULT(TypedArray<Texture2D>, templates);
    Ref<Image> selected_template;
    DECLARE_PROPERTY(bool, override_terrain, true);

    void initialize() override;
    void generate_chunk_data(Chunk* chunk, Vector3i chunk_position) override;
    void iterate(int layer);
    bool in_bounds(int x, int y);
    int touch_count(int x, int y, int z, int type);
    int get_cell(int x, int y, int z);
    virtual int update_rule(int x, int y, int z, int type);
};

}