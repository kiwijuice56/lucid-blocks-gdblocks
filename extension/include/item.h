
#pragma once

#include "../include/essence.h"
#include "macros.h"

#include <chrono>
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/mesh.hpp>

namespace godot {

class Item : public Resource {
	DECLARE_CLASS(Item, Resource);

protected:
    static int stack_size_increase;

public:
    DECLARE_PROPERTY(uint32_t, id, 0);
    DECLARE_PROPERTY(uint32_t, max_durability, 0);
    DECLARE_PROPERTY(String, internal_name, "");
    DECLARE_PROPERTY(String, display_name, "");
    DECLARE_PROPERTY(Color, color, Color(0, 0, 0, 1));
    DECLARE_PROPERTY_NO_DEFAULT(Ref<Texture2D>, icon);
    DECLARE_PROPERTY_NO_DEFAULT(Ref<Mesh>, mesh);
    DECLARE_PROPERTY(String, held_item_path, "");
    DECLARE_PROPERTY_NO_DEFAULT(Ref<PackedScene>, held_item_scene);
    DECLARE_PROPERTY_NO_DEFAULT(Ref<Essence>, essence);
    uint32_t stack_size = 50;
    DECLARE_PROPERTY(bool, internal, false);

    virtual uint32_t get_stack_size() const;
    void set_stack_size(uint32_t new_stack_size);

    void set_stack_size_increase(int increase);
};

}