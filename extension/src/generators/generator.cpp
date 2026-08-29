#include "../../include/chunk.h"
#include "../../include/world.h"
#include "../../include/generators/generator.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void Generator::_bind_methods() {
    BIND_PROPERTY(Generator, int64_t, seed);
    BIND_REF_ARRAY_PROPERTY(Generator, Noise, noise);
    BIND_REF_ARRAY_PROPERTY(Generator, Biome, biomes);

    BIND_METHOD(Generator, get_biome_at_real, "position");
    BIND_METHOD(Generator, get_biome_at, "position", "layer");
    BIND_METHOD(Generator, get_true_biome_coordinate, "position");
}

DEFINE_CONSTRUCTORS(Generator);

void Generator::initialize() {
    for (int i = 0; i < biomes.size(); i++) {
        Ref<Biome> biome = biomes[i];
        if (biome->day_spawns.size() != biome->day_spawn_proportions.size() || biome->night_spawns.size() != biome->night_spawn_proportions.size()) {
            UtilityFunctions::printerr("Biome has incorrectly configured spawns: ", biome->internal_name);
        }
    }

    void_noise = noise[11];
    fusion_noise = noise[14];
    noise_x = noise[1];
    noise_z = noise[2];

    for (int i = 0; i < noise.size(); i++) {
        Ref<FastNoiseLite> layer = noise[i];
        layer->set_seed(seed);
    }

    Ref<RandomNumberGenerator> rng = memnew(RandomNumberGenerator);
    rng->set_seed(seed);

    // Scatter biomes into random spots of the biome map, more spots for higher proportions
    PackedFloat64Array proportion_per_height;

    proportion_per_height.resize(BIOME_MAP_SIZE_Y);
    proportion_per_height.fill(0);

    TypedDictionary<int, Biome> default_map;
    for (int32_t i = 0; i < biomes.size(); i++) {
        Ref<Biome> biome = biomes[i];
        // Wrap lower biomes to the top
        if (biome->height < 0) {
            biome->height = BIOME_MAP_SIZE_Y + biome->height;
        }

        // Insert the default biome for each layer
        if (!default_map.has(biome->height)) {
            default_map[biome->height] = biome;
        }

        proportion_per_height[biome->height] += biome->proportion;
    }

    biome_index.clear();
    biome_index.resize(BIOME_MAP_SIZE_X * BIOME_MAP_SIZE_Y * BIOME_MAP_SIZE_Z);

    biome_map.clear();
    biome_map.resize(BIOME_MAP_SIZE_X * BIOME_MAP_SIZE_Y * BIOME_MAP_SIZE_Z);

    uint8_t index = 0;

    for (int32_t i = 0; i < biomes.size(); i++) {
        Ref<Biome> biome = biomes[i];

        biome->sync_and_initialize(world, index++);

        int32_t proportion = (int32_t) (BIOME_MAP_SIZE_X * BIOME_MAP_SIZE_Z * (biome->proportion / proportion_per_height[biome->height]));
        while (proportion > 0) {
            int32_t x = rng->randi_range(0, BIOME_MAP_SIZE_X - 1);
            int32_t z = rng->randi_range(0, BIOME_MAP_SIZE_Z - 1);
            int32_t y = biome->height;

            Ref<Biome> other_biome = biome_map[x + z * BIOME_MAP_SIZE_X + y * BIOME_MAP_SIZE_X * BIOME_MAP_SIZE_Z];
            if (other_biome != nullptr) {
                continue;
            }

            biome_map[x + z * BIOME_MAP_SIZE_X + y * BIOME_MAP_SIZE_X * BIOME_MAP_SIZE_Z]   = biome;
            biome_index[x + z * BIOME_MAP_SIZE_X + y * BIOME_MAP_SIZE_X * BIOME_MAP_SIZE_Z] = biome->index;

            proportion--;
        }
    }

    for (int32_t i = 0; i < biomes.size(); i++) {
        Ref<Biome> biome = biomes[i];
        biome_height_map[biome->index] = biome->height;
    }

    // Set default biome to second in list if some spots are empty

    int default_height_matched_count = 0;
    int default_height_unmatched_count = 0;
    Ref<Biome> default_biome = biomes[0];  // This is the worst case, mostly for generators that don't fill the entire biome height
    for (int32_t i = 0; i < biome_map.size(); i++) {
        Ref<Biome> other_biome = biome_map[i];

        if (other_biome == nullptr) {
            int height = i / (BIOME_MAP_SIZE_X * BIOME_MAP_SIZE_Z);

            Ref<Biome> default_at_height = default_map.has(height) ? Ref<Biome>(default_map[height]) : default_biome;

            if (default_map.has(height)) {
                default_height_matched_count++;
            } else {
                default_height_unmatched_count++;
            }

            biome_map[i] = default_at_height;
            biome_index[i] = default_at_height->index;
        }
    }

    Dictionary unique_indices;
    for (int i = 0; i < biome_map.size(); i++) {
        unique_indices.set(biome_index[i], true);
    }

    UtilityFunctions::print("Default biomes: ", default_height_matched_count, " at correct height, ", default_height_unmatched_count, " at pure default");
    UtilityFunctions::print("Unique biomes: ", unique_indices.size());
    UtilityFunctions::print("Generator initialized.");
}

void Generator::generate(World* world, Chunk* chunk, Vector3i chunk_position, int layer) {
    // Add an offset so that the fusion isn't the same biome
    if (layer == LAYER_FUSION) {
        chunk_position = chunk_position + Vector3i(8192, 0, 8192);
    }

    if (layer == LAYER_FUSION_WEIRD) {
        chunk_position = chunk_position - Vector3i(8192, -256, 8192);
    }

    // Before doing the heavy fusion work, first check that this chunk has any fusion blocks at all (from the noise)
    bool is_fusion = false;
    bool consistent_fusion = true;
    bool first_sample = true;
    if (layer == LAYER_FUSION || layer == LAYER_FUSION_WEIRD) {
        for (int i = 0; i <= 4; i++) {
        for (int j = 0; j <= 4; j++) {
        for (int k = 0; k <= 4; k++) {
            Vector3i sample_position_local = Vector3i(i * Chunk::CHUNK_SIZE_X / 4, j * Chunk::CHUNK_SIZE_Y / 4, k * Chunk::CHUNK_SIZE_Z / 4);

            // Unlike biomes, these have to be in bounds since we check an actual block of this chunk
            sample_position_local.x = UtilityFunctions::mini(sample_position_local.x, Chunk::CHUNK_SIZE_X - 1);
            sample_position_local.y = UtilityFunctions::mini(sample_position_local.y, Chunk::CHUNK_SIZE_Y - 1);
            sample_position_local.z = UtilityFunctions::mini(sample_position_local.z, Chunk::CHUNK_SIZE_Z - 1);

            Vector3i sample_position = chunk_position + sample_position_local;
            uint64_t base_index = chunk->get_block_index_at(sample_position_local);
            bool sample_fusion = is_fusion_block(sample_position, base_index);

            // Check that the first block is consistent with the rest
            if (first_sample) {
                is_fusion = sample_fusion;
                first_sample = false;
            } else if (is_fusion != sample_fusion) {
                consistent_fusion = false;
                break;
            }
        }
        if (!consistent_fusion) break;
        }
        if (!consistent_fusion) break;
        }
    }

    // If this chunk has no fusion blocks, just stop generating data
    if ((layer == LAYER_FUSION || layer == LAYER_FUSION_WEIRD) && consistent_fusion && !is_fusion) {
        return;
    }

    bool past_was_void = chunk->consistent_void; // Junk data if not in layer fusion
    bool past_was_consistent = chunk->consistent_biome;
    PackedByteArray biome_array;

    if ((layer == LAYER_FUSION || layer == LAYER_FUSION_WEIRD)&& !past_was_void) {
        biome_array = chunk->biome.duplicate();
    }

    // Check all 8 corners to see if a chunk's biome is the same throughout (the norm)
    // Saves performance on having to check the biome for each block when not necessary
    Ref<Biome> consistent_biome = get_consistent_biome(chunk_position, layer);
    chunk->consistent_biome = consistent_biome != nullptr;
    chunk->consistent_void = consistent_biome != nullptr && consistent_biome->index == 0;

    Dictionary contained_biomes;
    if (!chunk->consistent_biome) {
        for (int64_t y = 0; y < Chunk::CHUNK_SIZE_Y; y++) {
        for (int64_t z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
        for (int64_t x = 0; x < Chunk::CHUNK_SIZE_X; x++) {
            Vector3i biome_coordinate = get_true_biome_coordinate(chunk_position + Vector3i(x, y, z));
            uint8_t biome_index = get_biome_index_at_biome_coordinate(biome_coordinate);

            if (!contained_biomes.has(biome_index)) {
                contained_biomes[biome_index] = biome_coordinate;
            }

            int block_index = Chunk::position_to_index(Vector3i(x, y, z));
            chunk->biome[block_index] = biome_index;
        }
        }
        }
    }

    PackedInt32Array base_blocks;
    PackedByteArray base_water;
    if (layer == LAYER_FUSION || layer == LAYER_FUSION_WEIRD) {
        base_blocks = chunk->blocks.duplicate();
        base_water = chunk->water.duplicate();
    }

    // Data generation
    if (chunk->consistent_biome) {
        Vector3i biome_coordinate = get_true_biome_coordinate(chunk_position);
        consistent_biome->generate_chunk_data(chunk, chunk_position, biome_coordinate);
    } else {
        Array biome_indices = contained_biomes.keys();
        for (uint8_t i = 0; i < biome_indices.size(); i++) {
            Ref<Biome> biome = biomes[biome_indices[i]];
            Vector3i biome_coordinate = contained_biomes[biome_indices[i]]; 
            biome->generate_chunk_data(chunk, chunk_position, biome_coordinate);
        }
    }

    if (layer == LAYER_FUSION || layer == LAYER_FUSION_WEIRD) {
        for (int64_t y = 0; y < Chunk::CHUNK_SIZE_Y; y++) {
        for (int64_t z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
        for (int64_t x = 0; x < Chunk::CHUNK_SIZE_X; x++) {
            Vector3i position = Vector3i(x, y, z) + chunk_position;

            int i = chunk->position_to_index(Vector3i(x, y, z));
            int base_index = base_blocks[i];
            int override_index = chunk->blocks[i];

            // By default, use the original terrain
            chunk->blocks[i] = base_index;
            chunk->water[i] = base_water[i];

            // Don't override simblocks or void blocks
            if (base_index == 1) {
                continue;
            }

            // Don't replace anything in void biomes!
            if (past_was_void || (!past_was_consistent && biome_array[i] == 0)) {
                continue;
            }

            int64_t fusion_index = base_index + override_index * world->fusion_table_width;

            if (fusion_index >= world->fusion_table.size()) {
                continue;
            }

            int fused_id = world->fusion_table[fusion_index];
            int fused_block_index = world->block_id_to_index_map[fused_id];

            // Filter out air and complex blocks for performance
            if (fused_block_index == 0 ||
                 chunk->is_block_foliage[base_index] && !chunk->is_block_foliage[fused_block_index] ||
                !chunk->is_block_foliage[base_index] && chunk->is_block_foliage[fused_block_index] || chunk->is_block_internal[fused_block_index] ||
                 chunk->is_block_living[fused_block_index] || chunk->is_block_living[base_index] ||
                !chunk->is_block_transparent[base_index] && chunk->is_block_transparent[fused_block_index] ||
                 base_index == 0 && chunk->is_block_transparent[fused_block_index]) {

                continue;
            }

            // Filter out by noise (but with less replacement in air higher up)
            if (!(consistent_fusion && is_fusion) && !is_fusion_block(position, base_index)) {
                continue;
            }

            chunk->blocks[i] = fused_block_index;
        }
        }
        }
    }
}

bool Generator::is_fusion_block(Vector3i position, int base_block) {
    if (base_block == 0) {
        if (fusion_noise->get_noise_3d(position.x, position.y, position.z) < 0.5 + UtilityFunctions::clampf(UtilityFunctions::absi(position.y) / 80.0, 0.0, 1.0)) {
            return false;
        }
    } else if (fusion_noise->get_noise_3d(position.x, position.y, position.z) < 0.45) {
        return false;
    }
    return true;
}

void Generator::generate_decorations(World* world, Vector3i chunk_position) {
    Ref<Biome> b = get_consistent_biome(chunk_position, LAYER_BASE);

    // Don't generate decorations on chunk borders ...
    // Better transitions and performance
    if (b == nullptr) {
        return;
    }

    Vector3i coordinate = get_true_biome_coordinate(chunk_position);
    b->generate_decorations(chunk_position, coordinate);
}

void Generator::generate_structure(World* world, Vector3i structure_position) {

    uint16_t rng_seed = world->generator->seed;
    Random::scramble_rng_seed(&rng_seed, structure_position);

    // Find the center of this chunk, with some added noise
    // Fudge the y value lower so that structures on the ground (most common) use ground biomes
    Vector3i structure_center = structure_position + Vector3i(World::STRUCTURE_SIZE / 2, World::STRUCTURE_SIZE / 2, World::STRUCTURE_SIZE / 2);
    Vector3i grid_center = structure_center;

    int size = STRUCTURE_SPAWN_RADIUS_CHUNKS * 2;
    int offset_index = Random::randi(rng_seed) % (size * size * size);
    Vector3i offset;
    offset.x = offset_index % size;
    offset.y = offset_index / (size * size);
    offset.z = (offset_index / size) % size;

    offset -= Vector3i(size, size, size) / 2;
    offset *= Vector3i(Chunk::CHUNK_SIZE_X, Chunk::CHUNK_SIZE_Y, Chunk::CHUNK_SIZE_Z);

    structure_center += offset;

    // Pick a random structure from the center biome
    Ref<Biome> biome = get_biome_at(structure_center, LAYER_BASE);
    Vector3i biome_coordinate = get_true_biome_coordinate(structure_center);

    double total_proportion = 0.0;
    PackedFloat32Array running_sum;
    for (int32_t i = 0; i < biome->structure_proportions.size(); i++) {
        double proportion = biome->structure_proportions[i];
        total_proportion += proportion;
        running_sum.push_back(total_proportion);
    }

    Random::scramble_rng_seed(&rng_seed, structure_position * structure_position + Vector3i(structure_position.z, structure_position.y * 124.0, structure_position.x * 12.0));
    double roll = Random::randf(rng_seed) * total_proportion;
    Ref<Structure> to_spawn = memnew(Structure);

    for (int32_t i = 0; i < biome->structures.size(); i++) {
        if (roll < running_sum[i]) {
            to_spawn = biome->structures[i];
            break;
        }
    }

    to_spawn = to_spawn->duplicate();

    // Initialize some basic variables
    to_spawn->world = world;
    to_spawn->root_position = structure_position;
    to_spawn->center_position = structure_center;

    // Place the heart at the chunk in the center column that touches the ground
    int height_level = biome->get_ground_level(structure_center, biome_coordinate);
    if (height_level == Biome::GROUND_LEVEL_INVALID) {
        height_level = structure_position.y;
    }

    Vector3i snapped_height = world->snap_to_chunk(Vector3i(0, height_level - Chunk::CHUNK_SIZE_Y + 1, 0));
    to_spawn->heart_position = Vector3i(structure_center.x, snapped_height.y, structure_center.z);

    // Also snap the grounded center position
    height_level = biome->get_ground_level(grid_center, biome_coordinate);
    if (height_level == Biome::GROUND_LEVEL_INVALID) {
        height_level = grid_center.y;
    }

    snapped_height = world->snap_to_chunk(Vector3i(0, height_level - Chunk::CHUNK_SIZE_Y + 1, 0));
    to_spawn->grid_center_grounded_position = Vector3i(grid_center.x, snapped_height.y, grid_center.z);

    to_spawn->initialize();

    world->structure_map[structure_position] = to_spawn;
}

void Generator::place_decoration_blocks(World* world, Chunk* chunk, Array decorations, Vector3i chunk_position) {
    int64_t decoration_count = world->decoration_count[chunk_position];
    for (int64_t i = 0; i < decoration_count; i++) {
        Ref<DecorationState> ds = decorations[i];
        Ref<Decoration> d = ds->decoration;
        fill_decoration(world, chunk, chunk_position, d, ds, false);
    }
}

Vector3i Generator::find_actual_block_position(Vector3i local_position, Vector3i decoration_position, Vector3i decoration_size, Vector3i decoration_center_offset, DecorationState::Direction decoration_direction) {
    int x = local_position.x;
    int y = local_position.y;
    int z = local_position.z;

    Vector3i center = decoration_center_offset;

    int rx = (decoration_direction == DecorationState::West) ? (decoration_size.x - x - 1) : x;
    int ry = y;
    int rz = (decoration_direction == DecorationState::East || decoration_direction == DecorationState::South) ? (decoration_size.z - z - 1) : z;
    if (decoration_direction == DecorationState::East || decoration_direction == DecorationState::West) {
        int t = rx;
        rx = rz;
        rz = t;
        center = Vector3i(center.z, center.y, center.x);
    }

    return Vector3i(rx, ry, rz) + decoration_position - center;
}

// Replace blocks that require custom logic, like randomly removing leaves on trees
int Generator::replace_decoration_block(World* world, Chunk* chunk, Ref<Decoration> d, Ref<DecorationState> ds, Vector3i local_position, int block_id) {
    int32_t index = 0;

    if (block_id >= 3 && block_id <= 12) {
        index = d->replace(ds, chunk, block_id - 2, local_position, ds->position);
    } else {
        index = world->block_id_to_index_map[block_id];
    }

    return index;
}


void Generator::fill_decoration(World* world, Chunk* chunk, Vector3i chunk_position, Ref<Decoration> d, Ref<DecorationState> ds, bool override_air) {
    int replace_water_index = world->block_name_map["replace_water"];
    Vector3i size = d->get_size();
    PackedInt32Array blocks = d->get_blocks();

    for (int64_t y = 0; y < size.y; y++) {
    for (int64_t z = 0; z < size.z; z++) {
    for (int64_t x = 0; x < size.x; x++) {
        Vector3i local_position = find_actual_block_position(Vector3i(x, y, z), ds->position, size, d->center_offset, ds->direction) - chunk_position;

        if (!Chunk::in_bounds(local_position)) continue;

        int block_id = blocks[x + z * size.x + y * size.x * size.z];
        int index = replace_decoration_block(world, chunk, d, ds, local_position, block_id);

        if (!override_air && index == 0) continue; // Don't override with air blocks
        if (index == 1) index = 0;  // But replace void blocks with air

        chunk->blocks[Chunk::position_to_index(local_position)] = index;

        if (index == replace_water_index) {
            chunk->water[Chunk::position_to_index(local_position)] = 255;
            chunk->blocks[Chunk::position_to_index(local_position)] = 0;
        }
    }
    }
    }
}

void Generator::fill_large_decoration(World* world, Chunk* chunk, Vector3i chunk_position, Ref<Decoration> d, Ref<DecorationState> ds, bool override_air) {
    if (d == nullptr) {
        UtilityFunctions::printerr("Null decoration: ", chunk_position);
        return;
    }
    int replace_water_index = world->block_name_map["replace_water"];
    PackedInt32Array blocks = d->get_blocks();
    Vector3i size = d->get_size();

    for (int y = 0; y < Chunk::CHUNK_SIZE_Y; y++) {
    for (int z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
    for (int x = 0; x < Chunk::CHUNK_SIZE_X; x++) {
        Vector3i local_position = Vector3i(x, y, z);
        Vector3i world_position = chunk_position + local_position;
        Vector3i c = d->get_center_offset();
        if (ds->direction == DecorationState::East || ds->direction == DecorationState::West) {
            c = Vector3i(c.z, c.y, c.x);
        }
        Vector3i center = ds->position - c;
        Vector3i decoration_unrotated = world_position - center;
        Vector3i decoration_local;

        switch (ds->direction) {
            case DecorationState::North:
                decoration_local = decoration_unrotated;
                break;
            case DecorationState::East:
                decoration_local = Vector3i(decoration_unrotated.z, decoration_unrotated.y, size.z - 1 - decoration_unrotated.x);
                break;
            case DecorationState::South:
                decoration_local = Vector3i(decoration_unrotated.x, decoration_unrotated.y, size.z - 1 - decoration_unrotated.z);
                break;
            case DecorationState::West:
                decoration_local = Vector3i(size.x - 1 - decoration_unrotated.z, decoration_unrotated.y, decoration_unrotated.x);
                break;
        }

        if (decoration_local.x < 0 || decoration_local.y < 0 || decoration_local.z < 0 ||
            decoration_local.x >= size.x || decoration_local.y >= size.y || decoration_local.z >= size.z) {
            continue;
        }

        int block_id = blocks[decoration_local.x + decoration_local.z * size.x + decoration_local.y * size.x * size.z];
        int index = replace_decoration_block(world, chunk, d, ds, decoration_local, block_id);

        if (!override_air && index == 0) continue; // Don't override with air blocks
        if (index == 1) index = 0;  // But replace void blocks with air

        chunk->blocks[Chunk::position_to_index(local_position)] = index;

        if (index == replace_water_index) {
            chunk->water[Chunk::position_to_index(local_position)] = 255;
            chunk->blocks[Chunk::position_to_index(local_position)] = 0;
        }
    }
    }
    }
}

void Generator::place_structure_blocks(World* world, Chunk* chunk, Vector3i chunk_position) {
    Ref<Structure> structure = world->get_nearest_structure(chunk_position);
    structure->generate_chunk_data(chunk, chunk_position);
}

Ref<Biome> Generator::get_biome_at(Vector3i position, int layer) {
    return biomes[get_biome_index_at(position, layer)];
}

Ref<Biome> Generator::get_biome_at_real(Vector3i position) {
    return get_biome_at(position, LAYER_BASE);
}

Vector3i Generator::get_true_biome_coordinate(Vector3i position) {
    float cell_scale_xz = 0.5; 
    float cell_scale_y = 0.06;
    float voronoi_value = noise_x->get_noise_2d(position.x * cell_scale_xz + position.y * cell_scale_y, position.z * cell_scale_xz - + position.y * cell_scale_y);
    uint16_t seed = UtilityFunctions::floori((voronoi_value + 1.0) * 10000.0);
    Random::scramble_rng_seed(&seed, Vector3(1, 1, 1));
    int biome_horizontal_index = Random::randi(seed) % (BIOME_MAP_SIZE_X * BIOME_MAP_SIZE_Z);
    float y_noise = noise_z->get_noise_2d(position.x, position.z);
    int y = 6 + UtilityFunctions::floori((double) (position.y + Y_BIOME_NOISE * y_noise + 19) / Y_PER_BIOME_Y);
    return Vector3i(biome_horizontal_index % BIOME_MAP_SIZE_X, y, biome_horizontal_index / BIOME_MAP_SIZE_X);
}

uint8_t Generator::get_biome_index_at(Vector3i position, int layer) {
    if (layer == LAYER_BASE) {
        Vector3i biome_coordinate = get_true_biome_coordinate(position);
        return get_biome_index_at_biome_coordinate(biome_coordinate);
    } else {
        return get_biome_index_at(position, LAYER_BASE);
    }
}

uint8_t Generator::get_biome_index_at_biome_coordinate(Vector3i biome_coordinate) {
    int x = UtilityFunctions::posmod(biome_coordinate.x + biome_coordinate.y, BIOME_MAP_SIZE_X);
    int y = UtilityFunctions::posmod(biome_coordinate.y, BIOME_MAP_SIZE_Y);
    int z = UtilityFunctions::posmod(biome_coordinate.z + biome_coordinate.y, BIOME_MAP_SIZE_Z);

    return biome_index[x + z * BIOME_MAP_SIZE_X + y * BIOME_MAP_SIZE_X * BIOME_MAP_SIZE_Z];
}

Ref<Biome> Generator::get_consistent_biome(Vector3i chunk_position, int layer) {
    uint8_t consistent_biome = get_biome_index_at(chunk_position, layer);
    Vector3i consistent_coordinate = get_true_biome_coordinate(chunk_position);
    for (int8_t i = 0; i <= 4; i++) {
        for (int8_t j = 0; j <= 2; j++) {
            for (int8_t k = 0; k <= 4; k++) {
                Vector3i sample_position = chunk_position + Vector3(Chunk::CHUNK_SIZE_X / 4 * i, Chunk::CHUNK_SIZE_Y / 2 * j, Chunk::CHUNK_SIZE_Z / 4 * k);
                Vector3i sample_biome_coordinate = get_true_biome_coordinate(sample_position);
                if (sample_biome_coordinate != consistent_coordinate) {
                    return nullptr;
                }
                uint8_t corner_biome = get_biome_index_at_biome_coordinate(sample_biome_coordinate);
                if (corner_biome != consistent_biome) {
                    return nullptr;
                }
            }
        }
    }
    return biomes[consistent_biome];
}

int64_t Generator::get_seed() const {
    return seed;
}

void Generator::set_seed(int64_t new_seed) {
    seed = new_seed;

    for (uint32_t i = 0; i < noise.size(); i++) {
        Ref<FastNoiseLite> noise_element = noise[i];
        noise_element->set_seed(seed);
    }
}

DEFINE_PROPERTY_GETTER_SETTER(Generator, TypedArray<Noise>, noise);
DEFINE_PROPERTY_GETTER_SETTER(Generator, TypedArray<Biome>, biomes);
