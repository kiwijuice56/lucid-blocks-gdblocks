
#pragma once


#include "macros.h"
#include "structure.h"

namespace godot {

class World;

class CubeTemple : public Structure {
	DECLARE_CLASS(CubeTemple, Structure);

    TypedArray<DecorationState> placed_decorations;
    DECLARE_PROPERTY_NO_DEFAULT(TypedArray<Decoration>, decorations);

    Vector3i cutscene_block_position;

    const int64_t MAX_SIZE = 69;
    const int64_t MIN_SIZE = 69;

    int64_t size;

    void initialize();
    void generate_chunk_data(Chunk* chunk, Vector3i chunk_position);
    bool is_within_structure(Vector3i position);
    bool has_cutscene_block();
    Vector3i get_cutscene_block_position();
};

}