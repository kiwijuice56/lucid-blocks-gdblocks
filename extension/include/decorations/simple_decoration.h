
#pragma once


#include "macros.h"
#include "decoration.h"
#include "block.h"

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/classes/ref_counted.hpp>

namespace godot {

class SimpleDecoration : public Decoration {
	DECLARE_CLASS(SimpleDecoration, Decoration);

	DECLARE_PROPERTY(double, replace_chance, 1.0);

	TypedArray<Block> replacement_blocks;

    int32_t replace(Ref<DecorationState> decoration_state, Chunk* chunk, int32_t replace_number, Vector3i position, Vector3i decoration_position);

	TypedArray<Block> get_blocks() const;
    void set_blocks(TypedArray<Block> new_blocks);
};

}