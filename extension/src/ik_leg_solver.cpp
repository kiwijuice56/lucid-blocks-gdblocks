#include "../include/ik_leg_solver.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/node3d.hpp>

using namespace godot;

void IKLegSolver::_bind_methods() {
    ClassDB::bind_static_method("IKLegSolver", D_METHOD("look_at", "node", "target", "up", "use_model_front"), &IKLegSolver::look_at, Vector3(0, 1, 0), false);
    ClassDB::bind_static_method("IKLegSolver", D_METHOD("solve",
        "self", "root", "segments_parent",
        "segment_start", "segment_end",
        "segment_default_position", "segment_default_rotation",
        "final_target_position", "limit", "max_step"
    ), &IKLegSolver::solve, 4);
}

DEFINE_CONSTRUCTORS(IKLegSolver);

void IKLegSolver::look_at(Node3D *node, const Vector3 &target, const Vector3 &up, bool use_model_front) {
    Vector3 origin = node->get_global_position();
    if (origin.is_equal_approx(target)) {
        return;
    }

    Vector3 forward = (target - origin).normalized();
    Vector3 up_corrected = up;

    if (Math::abs(forward.dot(up_corrected)) > 0.99) {
        if (Math::abs(forward.dot(Vector3(1, 0, 0))) < 0.99) {
            up_corrected = Vector3(1, 0, 0);
        } else {
            up_corrected = Vector3(0, 0, 1);
        }
    }

    Basis lookat_basis = Basis::looking_at(forward * (target - origin).length(), up_corrected, use_model_front);
    Vector3 original_scale = node->get_scale();
    node->set_global_transform(Transform3D(lookat_basis, origin));
    node->set_scale(original_scale);
}

void IKLegSolver::solve(Node3D *self, Node3D *root, Node3D *segments_parent, const TypedArray<Node3D> &segment_start, const TypedArray<Node3D> &segment_end,
    const TypedArray<Vector3> &segment_default_position, const TypedArray<Vector3> &segment_default_rotation, Vector3 final_target_position, float limit, int max_step) {
    if (!self->is_visible()) {
        return;
    }

    Vector3 root_pos = root->get_global_position();

    if ((root_pos - final_target_position).length() > limit) {
        final_target_position = root_pos + (final_target_position - root_pos).normalized() * limit;
    }

    look_at(self, final_target_position);
    Vector3 rot = self->get_rotation();
    rot.x = 0.0;
    rot.z = 0.0;
    self->set_rotation(rot);
    self->set_global_position(root_pos);

    int child_count = segments_parent->get_child_count();
    for (int i = 0; i < child_count; i++) {
        Node3D *segment = Object::cast_to<Node3D>(segments_parent->get_child(i));
        segment->set_position(segment_default_position[i]);
        segment->set_rotation(segment_default_rotation[i]);
    }

    for (int step = 0; step < max_step; step++) {
        // Forward pass
        for (int i = child_count - 1; i >= 0; i--) {
            Node3D *seg_start = Object::cast_to<Node3D>(segment_start[i]);
            Node3D *seg_end = Object::cast_to<Node3D>(segment_end[i]);
            Node3D *segment = Object::cast_to<Node3D>(segments_parent->get_child(i));

            Vector3 target_position = (i == child_count - 1)
                ? final_target_position
                : Object::cast_to<Node3D>(segment_start[i + 1])->get_global_position();

            float length = (seg_end->get_global_position() - seg_start->get_global_position()).length();
            Vector3 dir = (target_position - seg_start->get_global_position()).normalized();

            // For the last segment, walk up the chain to find a stable up vector
            Vector3 up = Vector3(0, 1, 0);
            if (i == child_count - 1 && i > 0) {
                for (int j = i - 1; j >= 0; j--) {
                    Node3D *ancestor_seg = Object::cast_to<Node3D>(segments_parent->get_child(j));
                    Vector3 candidate_up = ancestor_seg->get_global_transform().basis.get_column(1).normalized();
                    if (Math::abs(dir.dot(candidate_up)) < 0.99) {
                        up = candidate_up;
                        break;
                    }
                }
            }

            look_at(segment, target_position, up);
            segment->set_global_position(target_position - dir * length);
        }

        // Backward pass
        for (int i = 0; i < child_count; i++) {
            Node3D *seg_start = Object::cast_to<Node3D>(segment_start[i]);
            Node3D *seg_end = Object::cast_to<Node3D>(segment_end[i]);
            Node3D *segment = Object::cast_to<Node3D>(segments_parent->get_child(i));

            Vector3 target_position = (i == 0) ? root_pos : Object::cast_to<Node3D>(segment_end[i - 1])->get_global_position();

            float length = (seg_end->get_global_position() - seg_start->get_global_position()).length();
            Vector3 dir = (target_position - seg_end->get_global_position()).normalized();

            if (i < child_count - 1) {
                look_at(segment, target_position - dir * 2.0 * length);
            }
            segment->set_global_position(target_position);
        }
    }
}