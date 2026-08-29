#pragma once

#include <vector>

#include "macros.h"

#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/cylinder_mesh.hpp>
#include <godot_cpp/classes/material.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

class Tesseract : public Node3D {
    DECLARE_CLASS(Tesseract, Node3D);

private:
    std::vector<Vector4> vertices;
    std::vector<Vector3> points;
    std::vector<MeshInstance3D*> lines;
    float time = 0.0f;

public:
    void _ready();
    void _process(double delta);

    DECLARE_PROPERTY(float, line_radius, 0.5);
    DECLARE_PROPERTY(float, point_radius, 0.1);
    DECLARE_PROPERTY(float, speed, 2.0);
    DECLARE_PROPERTY_NO_DEFAULT(Ref<Material>, cylinder_material);
    DECLARE_PROPERTY_NO_DEFAULT(Ref<Mesh>, mesh);
};
