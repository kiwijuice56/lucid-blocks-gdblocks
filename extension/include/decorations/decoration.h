#pragma once
#include "../include/macros.h"

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/classes/ref_counted.hpp>


namespace godot {

class World;
class Chunk;
class DecorationState;

class Decoration : public Resource {
	DECLARE_CLASS(Decoration, Resource);

    World* world; // Set by world

    DECLARE_PROPERTY(String, internal_name, "");

    DECLARE_PROPERTY(Vector3i, size, Vector3i(0, 0, 0));
    DECLARE_PROPERTY(Vector3i, center_offset, Vector3i(0, 0, 0));
    DECLARE_PROPERTY_NO_DEFAULT(PackedInt32Array, blocks);

    // Used by decorations within structures that contain a cutscene block --
    // Needs to be set manually
    DECLARE_PROPERTY(Vector3i, cutscene_block_position, Vector3i(0, 0, 0));

    DECLARE_PROPERTY(bool, has_cutscene_block, false);

    virtual int32_t replace(Ref<DecorationState> decoration_state, Chunk* chunk, int32_t replace_number, Vector3i position, Vector3i decoration_position);
};

}