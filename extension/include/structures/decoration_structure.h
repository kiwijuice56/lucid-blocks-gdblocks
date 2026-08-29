
#pragma once

#include "../macros.h"
#include "structure.h"

namespace godot {

class DecorationStructure : public Structure {
	DECLARE_CLASS(DecorationStructure, Structure);

    Ref<DecorationState> placed_decoration;
    DECLARE_PROPERTY_NO_DEFAULT(Ref<Decoration>, decoration);

    void initialize();
    void generate_chunk_data(Chunk* chunk, Vector3i chunk_position);
    bool is_within_structure(Vector3i position);
    bool has_cutscene_block();
    Vector3i get_cutscene_block_position();

    DECLARE_PROPERTY(bool, perfectly_centered, false);
    DECLARE_PROPERTY(bool, ignore_ground_level, false);
};

}