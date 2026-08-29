
#pragma once

#include "../macros.h"
#include "structure.h"

namespace godot {

class FridgeStructure : public Structure {
	DECLARE_CLASS(FridgeStructure, Structure);

    String words[52] = {
        "yam","chop","suffer","hill","broken","welcome","clean","thunder","boot","calc","battle","clutter","long","wreck","hesitant","consist","wine","fierce","measly","comfort","crush","warlike","sneeze","lucky","motion","knot","whip","shy","next","medical","offbeat","struct","rings",
        "tiamana","yhvh","god","qualia","alma","death","rebirth","cycle","leak","rosary","freedom","beads","bubble","manikin","bear","xyz","abc","def","qrs",
    };

    struct Word {
        String word = "";
        Vector3i root_position = Vector3i();
    };

    Vector3i lower_inner_min = Vector3i(1, 5, 1);
    Vector3i lower_inner_max = Vector3i(27, 60, 44);
    Vector3i upper_inner_min = Vector3i(1, 62, 1);
    Vector3i upper_inner_max = Vector3i(27, 104, 44);

    Vector3i upper_outer_min = Vector3i(33, 66, 3);
    Vector3i upper_outer_max = Vector3i(33, 104, 42);
    Vector3i lower_outer_min = Vector3i(33, 7, 3);
    Vector3i lower_outer_max = Vector3i(33, 58, 42);

    Ref<Noise> fridge_noise;
    std::vector<Word> placed_words;
    Ref<DecorationState> placed_decoration;
    DECLARE_PROPERTY_NO_DEFAULT(Ref<Decoration>, decoration);

    void initialize();
    void generate_chunk_data(Chunk* chunk, Vector3i chunk_position);
    bool is_within_structure(Vector3i position);
    bool has_cutscene_block();
    Vector3i get_cutscene_block_position();
};

}