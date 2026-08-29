#pragma once

#include "macros.h"
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/classes/curve_texture.hpp>
#include <godot_cpp/classes/curve.hpp>

namespace godot {

class IKLegSolver : public Object {
    DECLARE_CLASS(IKLegSolver, Object);

    static void look_at(Node3D *node, const Vector3 &target, const Vector3 &up = Vector3(0, 1, 0), bool use_model_front = false);
    static void solve(
        Node3D *self, Node3D *root, Node3D *segments_parent, const TypedArray<Node3D> &segment_start, const TypedArray<Node3D> &segment_end,
        const TypedArray<Vector3> &segment_default_position, const TypedArray<Vector3> &segment_default_rotation, Vector3 final_target_position, float limit, int max_step = 4
    );
};

} 