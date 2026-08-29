#pragma once


#include "macros.h"
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/vector3i.hpp>
#include <godot_cpp/classes/texture2d.hpp>

#include "block.h"

using namespace godot;

class ItemLoader : public RefCounted {
    DECLARE_CLASS(ItemLoader, RefCounted);

    Ref<Block> generate_directional_variant(const Ref<Block> &block, Vector3i direction, bool generate_texture);
    Ref<ImageTexture> get_rotated_texture(const Ref<Block> &block, Vector3i direction);
};
