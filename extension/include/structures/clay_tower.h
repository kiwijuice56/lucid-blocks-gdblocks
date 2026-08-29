
#pragma once


#include "macros.h"
#include "structure.h"

namespace godot {

class ClayTower : public Structure {
	DECLARE_CLASS(ClayTower, Structure);

    TypedArray<DecorationState> placed_decorations;

    DECLARE_PROPERTY_NO_DEFAULT(TypedArray<Decoration>, decorations);
    bool cutscene_block_decoration_spawned = false;
    Vector3i cutscene_block_position;

    void initialize();
    void generate_chunk_data(Chunk* chunk, Vector3i chunk_position);
    bool is_within_structure(Vector3i position);
    bool has_cutscene_block();
    Vector3i get_cutscene_block_position();
};

}