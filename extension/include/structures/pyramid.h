
#pragma once


#include "macros.h"
#include "structure.h"

namespace godot {

class World;

class Pyramid : public Structure {
	DECLARE_CLASS(Pyramid, Structure);

    const int64_t MAX_SIZE = 68;
    const int64_t MIN_SIZE = 33;

    int64_t size;

    void initialize();
    void generate_chunk_data(Chunk* chunk, Vector3i chunk_position);
    bool is_within_structure(Vector3i position);
};

}