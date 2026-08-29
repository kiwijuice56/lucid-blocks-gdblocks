
#pragma once


#include "macros.h"
#include "decoration.h"

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/classes/ref_counted.hpp>

namespace godot {

class AmbiguousDecoration : public Decoration {
	DECLARE_CLASS(AmbiguousDecoration, Decoration);

    int32_t replace(Ref<DecorationState> decoration_state, Chunk* chunk, int32_t replace_number, Vector3i position, Vector3i decoration_position);
};

}