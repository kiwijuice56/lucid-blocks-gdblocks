#include "../../include/decorations/simple_decoration.h"
#include "../../include/decoration_state.h"
#include "../../include/world.h"
#include "../../include/chunk.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void SimpleDecoration::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_replacement_blocks"), &SimpleDecoration::get_blocks);
    ClassDB::bind_method(D_METHOD("set_replacement_blocks", "new_blocks"), &SimpleDecoration::set_blocks);
    ADD_PROPERTY(
        PropertyInfo(
            Variant::ARRAY,
            "replacement_blocks",
            PROPERTY_HINT_TYPE_STRING,
            String::num(Variant::OBJECT) + "/" + String::num(PROPERTY_HINT_RESOURCE_TYPE) + ":Block"
        ),
        "set_replacement_blocks",
        "get_replacement_blocks"
    );

    BIND_PROPERTY(SimpleDecoration, double, replace_chance);
}

DEFINE_CONSTRUCTORS(SimpleDecoration);

DEFINE_PROPERTY_GETTER_SETTER(SimpleDecoration, double, replace_chance);

int32_t SimpleDecoration::replace(Ref<DecorationState> decoration_state, Chunk* chunk, int32_t replace_number, Vector3i position, Vector3i decoration_position) {
    uint16_t rng_seed = world->generator->seed;
    Random::scramble_rng_seed(&rng_seed, position);
    if (replace_number - 1 >= replacement_blocks.size() || Random::randf(rng_seed) > replace_chance) {
        return 0;
    }
    return Ref<Block>(replacement_blocks[replace_number - 1])->index;
}

TypedArray<Block> SimpleDecoration::get_blocks() const {
    return replacement_blocks;
}

void SimpleDecoration::set_blocks(TypedArray<Block> new_blocks) {
    replacement_blocks = new_blocks;
}
