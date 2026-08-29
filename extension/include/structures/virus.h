
#pragma once


#include "macros.h"
#include "structure.h"

namespace godot {

class Virus : public Structure {
	DECLARE_CLASS(Virus, Structure);

    Ref<Noise> sponge_noise;
    bool invalid = false;

    void initialize();
    void generate_chunk_data(Chunk* chunk, Vector3i chunk_position);
    bool is_within_structure(Vector3i position);
};

}