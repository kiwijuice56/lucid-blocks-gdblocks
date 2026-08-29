#include "../include/tesseract.h"

// This code is very broken in terms of being an actual tesseract, but
// the artifacts end up looking very cool and unique :) 

void Tesseract::_bind_methods() {
    BIND_PROPERTY(Tesseract, float, line_radius);
    BIND_PROPERTY(Tesseract, float, point_radius);
    BIND_PROPERTY(Tesseract, float, speed);
    BIND_REF_PROPERTY(Tesseract, Material, cylinder_material);
    BIND_REF_PROPERTY(Tesseract, Mesh, mesh);
}

DEFINE_CONSTRUCTORS(Tesseract);

DEFINE_PROPERTY_GETTER_SETTER(Tesseract, float, line_radius);
DEFINE_PROPERTY_GETTER_SETTER(Tesseract, float, point_radius);
DEFINE_PROPERTY_GETTER_SETTER(Tesseract, float, speed);
DEFINE_PROPERTY_GETTER_SETTER(Tesseract, Ref<Material>, cylinder_material);
DEFINE_PROPERTY_GETTER_SETTER(Tesseract, Ref<Mesh>, mesh);

void Tesseract::_ready() {
    vertices.resize(16);
    points.resize(16);

    for (int i = 0; i < 16; i++) {
        float x = get_scale().x * (2.0 * (i / 8) - 1);
        float y = get_scale().y * (2.0 * ((i / 4) % 2) - 1);
        float z = get_scale().z * (2.0 * ((i / 2) % 2) - 1);
        float w = get_scale().z * (2.0 * (i % 2) - 1);
        vertices[i] = Vector4(x, y, z, w);
        points[i] = Vector3(); 
    }

    for (int i = 0; i < 16; i++) {
        Vector4 v1 = vertices[i];
        for (int j = i + 1; j < 16; j++) {
            if (v1.distance_to(vertices[j]) > get_scale().x * 2.0) {
                continue;
            }

            MeshInstance3D* line = memnew(MeshInstance3D);
            line->set_mesh(mesh);
            line->set_scale(get_scale());
            line->set_material_override(cylinder_material);
            add_child(line);
            lines.push_back(line);
        }
    }
}

void Tesseract::_process(double delta) {
    time += speed * delta;
    int li = 0;
    std::vector<float> radii;
    radii.resize(16);

    // Update points
    for (int i = 0; i < 16; i++) {
        Vector4 base = vertices[i];
        Vector2 sub(base.x, base.w);
        sub = sub.rotated(time);
        base.x = sub.x;
        base.w = sub.y;

        float scaling = 3.0 / (3.0 - base.w);
        points[i] = scaling * Vector3(base.x, base.y, base.z);
        radii[i] = scaling * scaling * point_radius;
    }

    // Update edges
    for (int i = 0; i < 16; i++) {
        Vector3 p1 = points[i];
        for (int j = i + 1; j < 16; j++) {
            if (vertices[i].distance_to(vertices[j]) > get_scale().x * 2.0)
                continue;

            Vector3 p2 = points[j];
            MeshInstance3D* line = lines[li];

            line->set_position((p1 + p2) / 2.0);

            Vector3 up = Vector3(0, 1, 0);

            line->look_at(p2, up);

            Vector3 scale = line->get_scale();
            scale.y = p1.distance_to(p2);
            line->set_scale(scale);

            Ref<CylinderMesh> cyl = Object::cast_to<CylinderMesh>(line->get_mesh().ptr());
            if (cyl.is_valid()) {
                cyl->set_bottom_radius(radii[i] * line_radius);
                cyl->set_top_radius(radii[j] * line_radius);
            }

            li++;
        }
    }
}
