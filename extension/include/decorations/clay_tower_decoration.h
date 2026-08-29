
#pragma once


#include "macros.h"
#include "decoration.h"
#include "block.h"

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/classes/ref_counted.hpp>

namespace godot {

class ClayTowerDecoration : public Decoration {
	DECLARE_CLASS(ClayTowerDecoration, Decoration);

	DECLARE_PROPERTY_NO_DEFAULT(Ref<Block>, default_block);

    int32_t replace(Ref<DecorationState> decoration_state, Chunk* chunk, int32_t replace_number, Vector3i position, Vector3i decoration_position);
};

}