#include "../../include/biomes/dungeon_biome.h"
#include "../../include/world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void DungeonBiome::_bind_methods() {
    BIND_REF_ARRAY_PROPERTY(DungeonBiome, Texture2D, noise_textures);
    BIND_PROPERTY(DungeonBiome, PackedFloat64Array, thresholds);
    BIND_PROPERTY(DungeonBiome, PackedFloat64Array, scales);
    BIND_PROPERTY(DungeonBiome, int, level_height);
    BIND_PROPERTY(DungeonBiome, float, foliage_frequency);
    BIND_PROPERTY(DungeonBiome, bool, has_water);
    BIND_PROPERTY(DungeonBiome, bool, has_exits);
}

DEFINE_CONSTRUCTORS(DungeonBiome);

DEFINE_PROPERTY_GETTER_SETTER(DungeonBiome, int, level_height);
DEFINE_PROPERTY_GETTER_SETTER(DungeonBiome, float, foliage_frequency);
DEFINE_PROPERTY_GETTER_SETTER(DungeonBiome, bool, has_water);
DEFINE_PROPERTY_GETTER_SETTER(DungeonBiome, bool, has_exits);
DEFINE_PROPERTY_GETTER_SETTER(DungeonBiome, TypedArray<Texture2D>, noise_textures);
DEFINE_PROPERTY_GETTER_SETTER(DungeonBiome, PackedFloat64Array, thresholds);
DEFINE_PROPERTY_GETTER_SETTER(DungeonBiome, PackedFloat64Array, scales);

void DungeonBiome::initialize() {
    Biome::initialize();

    presence_noise = world->generator->noise[31];

    Ref<Texture2D> texture_0 = noise_textures[0];
    Ref<Image> image_0 = texture_0->get_image();
    
    Ref<Texture2D> texture_1 = noise_textures[1];
    Ref<Image> image_1 = texture_1->get_image();

    Ref<Texture2D> texture_2 = noise_textures[2];
    Ref<Image> image_2 = texture_2->get_image();

    width_0 = image_0->get_width();
    width_1 = image_1->get_width();
    width_2 = image_2->get_width();
    
    height_0 = image_0->get_height();
    height_1 = image_1->get_height();
    height_2 = image_2->get_height();

    data_0.clear();
    for (int64_t x = 0; x < image_0->get_width(); x++) {
    for (int64_t z = 0; z < image_0->get_height(); z++) {
        Color sample_0 = image_0->get_pixel(x, z);
        data_0.append(sample_0.r);
    }
    }
    
    data_1.clear();
    for (int64_t x = 0; x < image_1->get_width(); x++) {
    for (int64_t z = 0; z < image_1->get_height(); z++) {
        Color sample_1 = image_1->get_pixel(x, z);
        data_1.append(sample_1.r);
    }
    }

    data_2.clear();
    for (int64_t x = 0; x < image_2->get_width(); x++) {
    for (int64_t z = 0; z < image_2->get_height(); z++) {
        Color sample_2 = image_2->get_pixel(x, z);
        data_2.append(sample_2.r);
    }
    }
}

void DungeonBiome::generate_chunk_data(Chunk* chunk, Vector3i chunk_position, Vector3i biome_coordinate) {
    uint16_t rng_seed = world->generator->seed;

    Ref<Block> block_0 = building_blocks[0];
    int block_index_0 = block_0->index;

    Ref<Block> block_1 = building_blocks[1];
    int block_index_1 = block_1->index;

    uint16_t chunk_seed = rng_seed;
    Random::scramble_rng_seed(&chunk_seed, chunk_position);
    bool exit_chunk = has_exits && Random::randf(chunk_seed) < 0.001;

    for (int64_t z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
    for (int64_t x = 0; x < Chunk::CHUNK_SIZE_X; x++) {
        
        int64_t ground_level = get_ground_level(chunk_position + Vector3i(x, 0, z), biome_coordinate);
        int64_t water_level = get_water_level(chunk_position + Vector3i(x, 0, z), biome_coordinate);

        for (int64_t y = 0; y < Chunk::CHUNK_SIZE_Y; y++) {
            Vector3i local_position = Vector3i(x, y, z);
            Vector3i position = chunk_position + local_position;
            Random::scramble_rng_seed(&rng_seed, position);

            if (block_locked(chunk, local_position)) {
                continue;
            }

            int32_t block_type = get_block_at(position, block_index_0);

            if (Random::randf(rng_seed) < foliage_frequency && is_foliage_safe(chunk, position, local_position) && block_type == 0 && get_block_at(position - Vector3i(0, 1, 0), block_index_0) != 0) {
                block_type = block_index_1;
            }

            if (has_water && is_on_vertical_border(chunk, position, local_position)) {
                block_type = block_index_0;
            }

            if (exit_chunk && Random::randf(rng_seed) < 0.0001) {
                block_type = world->block_name_map["naraka block"];
            }

            chunk->blocks[Chunk::position_to_index(Vector3i(x, y, z))] = block_type;
            chunk->water[Chunk::position_to_index(Vector3i(x, y, z))] = has_water && position.y <= water_level ? 255 : 0;
        }
    }
    }
}

int DungeonBiome::get_block_at(Vector3i position, int block_index_0) {
    if (presence_noise->get_noise_3d(position.x, position.y, position.z) < thresholds[0]) {
        return 0;
    } else {
        int height = UtilityFunctions::floori(position.y / (float) level_height);
        int story_height = UtilityFunctions::posmod(position.y, level_height);
        int tunnel_height = UtilityFunctions::mini(story_height, level_height - story_height);

        Vector2i coord_0 = Vector2i(UtilityFunctions::posmod((int) (scales[0] * position.x + height * 64), width_0), UtilityFunctions::posmod((int) (scales[0] * position.z + height * 128), height_0));
        int index_0 = coord_0.x + coord_0.y * width_0;
        float sample_0 = data_0[index_0];
        
        Vector2i coord_1 = Vector2i(UtilityFunctions::posmod((int) (scales[1] * position.x + height * 32), width_1), UtilityFunctions::posmod((int) (scales[1] * position.z + height * 96), height_1));
        int index_1 = coord_1.x + coord_1.y * width_1;
        float sample_1 = data_1[index_1];

        bool mask_0 = sample_0 < thresholds[1] - tunnel_height * 0.2;
        bool mask_1 = sample_1 < thresholds[2] - tunnel_height * 0.2;


        int block_type = 0;
        if (mask_0) {
            block_type = block_index_0;
        }

        if (mask_1) {
            if (block_type == 0) {
                block_type = block_index_0;
            } else {
                block_type = 0;
            }
        }

        return block_type;
    }
}

void DungeonBiome::generate_decorations(Vector3i chunk_position, Vector3i biome_coordinate) { }

int64_t DungeonBiome::get_ground_level(Vector3i position, Vector3i biome_coordinate) {
    return GROUND_LEVEL_INVALID;
}