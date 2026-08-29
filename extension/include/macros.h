#pragma once

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

template<typename T> struct GodotVariantType;
template<> struct GodotVariantType<float>              { static constexpr auto value = Variant::FLOAT;                };
template<> struct GodotVariantType<double>             { static constexpr auto value = Variant::FLOAT;                };
template<> struct GodotVariantType<int>                { static constexpr auto value = Variant::INT;                  };
template<> struct GodotVariantType<int64_t>            { static constexpr auto value = Variant::INT;                  };
template<> struct GodotVariantType<uint32_t>           { static constexpr auto value = Variant::INT;                  };
template<> struct GodotVariantType<bool>               { static constexpr auto value = Variant::BOOL;                 };
template<> struct GodotVariantType<String>             { static constexpr auto value = Variant::STRING;               };
template<> struct GodotVariantType<Vector2>            { static constexpr auto value = Variant::VECTOR2;              };
template<> struct GodotVariantType<Vector3>            { static constexpr auto value = Variant::VECTOR3;              };
template<> struct GodotVariantType<Vector3i>           { static constexpr auto value = Variant::VECTOR3I;             };
template<> struct GodotVariantType<Color>              { static constexpr auto value = Variant::COLOR;                };
template<> struct GodotVariantType<PackedInt32Array>   { static constexpr auto value = Variant::PACKED_INT32_ARRAY;   };
template<> struct GodotVariantType<PackedFloat32Array> { static constexpr auto value = Variant::PACKED_FLOAT32_ARRAY; };
template<> struct GodotVariantType<PackedFloat64Array> { static constexpr auto value = Variant::PACKED_FLOAT64_ARRAY; };
template<typename T> struct GodotVariantType<Ref<T>>   { static constexpr auto value = Variant::OBJECT;               };

#define BIND_SIMPLE_PROPERTY(CLASS, TYPE, NAME)                                 \
ClassDB::bind_method(D_METHOD("get_" #NAME), &CLASS::get_##NAME);               \
ClassDB::bind_method(D_METHOD("set_" #NAME, "value"), &CLASS::set_##NAME);      \
ADD_PROPERTY(PropertyInfo(TYPE, #NAME), "set_" #NAME, "get_" #NAME)             \

#define BIND_PROPERTY(CLASS, TYPE, NAME)                                        \
ClassDB::bind_method(D_METHOD("get_" #NAME), &CLASS::get_##NAME);               \
ClassDB::bind_method(D_METHOD("set_" #NAME, "value"), &CLASS::set_##NAME);      \
ADD_PROPERTY(PropertyInfo(GodotVariantType<TYPE>::value, #NAME), "set_" #NAME, "get_" #NAME)

#define BIND_PROPERTY_PREFIXED(CLASS, TYPE, PREFIX, NAME)                       \
ClassDB::bind_method(D_METHOD("get_" #NAME), &CLASS::get_##NAME);               \
ClassDB::bind_method(D_METHOD("set_" #NAME, "value"), &CLASS::set_##NAME);      \
ADD_PROPERTY(PropertyInfo(GodotVariantType<TYPE>::value, #PREFIX #NAME), "set_" #NAME, "get_" #NAME)

#define BIND_REF_PROPERTY(CLASS, REF_TYPE, NAME)                                \
ClassDB::bind_method(D_METHOD("get_" #NAME), &CLASS::get_##NAME);               \
ClassDB::bind_method(D_METHOD("set_" #NAME, "value"), &CLASS::set_##NAME);      \
ADD_PROPERTY(PropertyInfo(Variant::OBJECT, #NAME, PROPERTY_HINT_RESOURCE_TYPE, #REF_TYPE), "set_" #NAME, "get_" #NAME)

#define BIND_REF_ARRAY_PROPERTY(CLASS, REF_TYPE, NAME)                          \
ClassDB::bind_method(D_METHOD("get_" #NAME), &CLASS::get_##NAME);               \
ClassDB::bind_method(D_METHOD("set_" #NAME, "value"), &CLASS::set_##NAME);      \
ADD_PROPERTY(PropertyInfo(Variant::ARRAY, #NAME, PROPERTY_HINT_RESOURCE_TYPE, #REF_TYPE), "set_" #NAME, "get_" #NAME)

#define BIND_REF_ARRAY_PROPERTY_PREFIXED(CLASS, REF_TYPE, PREFIX, NAME)         \
ClassDB::bind_method(D_METHOD("get_" #NAME), &CLASS::get_##NAME);               \
ClassDB::bind_method(D_METHOD("set_" #NAME, "value"), &CLASS::set_##NAME);      \
ADD_PROPERTY(PropertyInfo(Variant::ARRAY, #PREFIX #NAME, PROPERTY_HINT_RESOURCE_TYPE, #REF_TYPE), "set_" #NAME, "get_" #NAME)

#define BIND_BITMASK_PROPERTY(CLASS, TYPE, NAME)                                \
ClassDB::bind_method(D_METHOD("get_" #NAME), &CLASS::get_##NAME);               \
ClassDB::bind_method(D_METHOD("set_" #NAME, "value"), &CLASS::set_##NAME);      \
ADD_PROPERTY(PropertyInfo(TYPE, #NAME, PROPERTY_HINT_LAYERS_AVOIDANCE, ""), "set_" #NAME, "get_" #NAME)

#define BIND_ENUM_PROPERTY(CLASS, NAME, LABELS) \
ClassDB::bind_method(D_METHOD("get_" #NAME), &CLASS::get_##NAME);               \
ClassDB::bind_method(D_METHOD("set_" #NAME, "value"), &CLASS::set_##NAME);      \
ADD_PROPERTY(PropertyInfo(Variant::INT, #NAME, PROPERTY_HINT_ENUM, LABELS), "set_" #NAME, "get_" #NAME);

#define DEFINE_PROPERTY_GETTER_SETTER(CLASS, TYPE, NAME)                        \
TYPE CLASS::get_##NAME() const { return NAME; }                                 \
void CLASS::set_##NAME(TYPE value) { NAME = value; }

#define DECLARE_PROPERTY(TYPE, NAME, DEFAULT_VALUE)                             \
public:                                                                         \
    TYPE NAME = DEFAULT_VALUE;                                                  \
    TYPE get_##NAME() const;                                                    \
    void set_##NAME(TYPE value);

#define DECLARE_PROPERTY_NO_DEFAULT(TYPE, NAME)                                 \
public:                                                                         \
    TYPE NAME;                                                                  \
    TYPE get_##NAME() const;                                                    \
    void set_##NAME(TYPE value);

#define DECLARE_CLASS(CLASS, EXTENDS)                                           \
    GDCLASS(CLASS, EXTENDS);                                                    \
    protected:                                                                  \
        static void _bind_methods();                                            \
    public:                                                                     \
        CLASS();                                                                \
        ~CLASS();

#define DEFINE_CONSTRUCTORS(CLASS)                                              \
CLASS::CLASS() { }                                                              \
CLASS::~CLASS() { }

#define DEFINE_CONSTRUCTORS_AND_BINDINGS(CLASS)                                 \
void CLASS::_bind_methods() { }                                                 \
CLASS::CLASS() { }                                                              \
CLASS::~CLASS() { }

#define BIND_METHOD(CLASS, METHOD, ...)                                         \
ClassDB::bind_method(D_METHOD(#METHOD, __VA_ARGS__), &CLASS::##METHOD);

#define BIND_METHOD_NO_PARAMETERS(CLASS, METHOD)                                \
ClassDB::bind_method(D_METHOD(#METHOD), &CLASS::##METHOD);