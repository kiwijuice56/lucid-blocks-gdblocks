
#pragma once

#include "macros.h"
#include "block.h"
#include "generators/generator.h"
#include "structures/structure.h"
#include "biomes/biome.h"
#include "chunk.h"
#include "decoration.h"
#include "item_state.h"

#include <godot_cpp/variant/typed_dictionary.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/shader_material.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/texture2d_array.hpp>
#include <godot_cpp/classes/worker_thread_pool.hpp>
#include <godot_cpp/classes/thread.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/window.hpp>

namespace godot {

class World : public Node3D {
	DECLARE_CLASS(World, Node3D);

	static const int64_t REGION_SIZE = 16;
	static const int64_t MAX_DECORATIONS = 64;
	static const int64_t MAX_WATER_SUBCHUNKS_PER_FRAME = 48;
	static const int64_t MAX_WATER_RERENDERED_CHUNKS_PER_FRAME = 2;
	static const int64_t FIRE_VISUAL_POOL_SIZE = 64;
	static const int64_t STRUCTURE_SIZE = 256; // (In blocks)
	static const int64_t WATER_SKIP = 5;

	// (In blocks)
	int64_t instance_radius = 128;
	int64_t water_simulate_radius = 64;

	// Frame allocation 
	DECLARE_PROPERTY(uint64_t, usec_budget_per_frame, 3000);
	DECLARE_PROPERTY(int, chunk_batch_size, 8);
	DECLARE_PROPERTY(int, decoration_batch_size, 16);
	DECLARE_PROPERTY(int, chunk_render_limit, 8);

	uint64_t elapsed_time = 0; // Microseconds spent on the main thread
	uint64_t frame_start_time = 0;

	// Resources
	DECLARE_PROPERTY_NO_DEFAULT(TypedArray<Block>, block_types);
	PackedByteArray is_block_foliage;
	PackedByteArray is_block_floating_foliage;
    PackedByteArray is_block_transparent;
	PackedByteArray is_block_living;
	PackedByteArray is_block_internal;
	PackedByteArray is_block_top_face_randomized;
	PackedByteArray is_block_non_collision;
	PackedInt32Array fusion_table;
	int fusion_table_width;
	DECLARE_PROPERTY_NO_DEFAULT(TypedArray<Decoration>, decorations);
	TypedArray<Biome> biomes;
	DECLARE_PROPERTY_NO_DEFAULT(Ref<Generator>, generator);

	DECLARE_PROPERTY_NO_DEFAULT(Ref<ShaderMaterial>, block_material);
	DECLARE_PROPERTY_NO_DEFAULT(Ref<ShaderMaterial>, foliage_material);
	DECLARE_PROPERTY_NO_DEFAULT(Ref<ShaderMaterial>, transparent_block_material);
	DECLARE_PROPERTY_NO_DEFAULT(Ref<ShaderMaterial>, water_material);
	DECLARE_PROPERTY_NO_DEFAULT(Ref<ShaderMaterial>, water_surface_material);
	Ref<PackedScene> break_effect_scene;
	Ref<PackedScene> place_effect_scene;
	Ref<PackedScene> fire_visual_scene;
	DECLARE_PROPERTY_NO_DEFAULT(TypedArray<ShaderMaterial>, requires_texture_atlas);

	// The center chunk's position
	Vector3i center_chunk = Vector3i(0, 0, 0);

	bool first_load = false;
	bool all_loaded = false;
	DECLARE_PROPERTY(bool, debug_stall, false);
	int render_frame = 0;

	// Stores references to all chunk node instances
	std::vector<Chunk*> all_chunks;

	// Used to save up all chunks that need to be remeshed for gameplay reasons;
	// Currently only implemented for block placement, and pushes + call to remesh_pending_chunks
	// must be atomic
	std::vector<Chunk*> to_remesh_later;

	// Stores coordinates (Vector3i) of loaded chunks
	TypedDictionary<Vector3i, bool> is_chunk_loaded;

	// Stores (Vector3i : Chunk) mapping for easy access
	TypedDictionary<Vector3i, Chunk> chunk_map;

	// Stores (Vector3i : bool) for which chunks have decorations generated
	TypedDictionary<Vector3i, bool> decoration_generated;

	// Stores (Vector3i : Structure) mapping
	TypedDictionary<Vector3i, Ref<Structure>> structure_map;

	// Stores data of modified chunks (Vector3i : PackedInt32Array)
	TypedDictionary<Vector3i, PackedInt32Array> chunk_data;
	TypedDictionary<Vector3i, PackedByteArray>  chunk_water_data;
	TypedDictionary<Vector3i, PackedByteArray>  chunk_water_awake_data;
	TypedDictionary<Vector3i, PackedByteArray>  chunk_fire_data; 

	// Stores the region coordinates that had a dirty chunk unloaded within it (i.e. needs saving)
	TypedDictionary<Vector3i, bool> dirty_regions; 

	// Used to access chunks that need to be initialized
	std::vector<Chunk*> init_queue;
	std::vector<Vector3i> init_queue_positions;
	std::vector<Chunk*> needs_meshing_queue;
	std::mutex mesh_queue_mutex;

	// Multithreading state
	uint64_t task_id = 0;
	bool finalizing_chunks = false;
	int chunk_finalization_index = 0;
	bool has_task = false;
	std::mutex decoration_lock;

	// Water state
	uint8_t water_frame = 0;
	uint8_t water_direction = 0;
	uint16_t simulated_water_subchunks = 0;
	uint16_t rendered_water_chunks = 0;

	// Living block state

	// Stores (Vector3i : LivingBlock) mappings
	// of currently loaded living blocks
	TypedDictionary<Vector3i, Ref<Node3D>> living_block_map;

	// Decoration state

	// Stores (Vector3i : Array[DecorationState])
	TypedDictionary<Vector3i, Array> decoration_map;

	// Stores (Vector3i : int) [size of above arrays]
	TypedDictionary<Vector3i, int> decoration_count;

	// Helpful resource mappings

	// Stores (int : int)
	TypedDictionary<int, int> block_id_to_index_map;

	// Stores (int : int) ... we could just use block_types, but this is faster
	PackedInt32Array block_index_to_id_map;

	// Stores (String : Resource) name to Decoration pairs
	TypedDictionary<String, Ref<Decoration>> decoration_name_map;

	// Stores (String : int) name to index pairs
	TypedDictionary<String, int> block_name_map;

	void set_block_indices();
	void start_up();
	void initialize();
	void create_texture_atlas();
	void instantiate_chunks();
	void refresh_instance_radius();
	Chunk* instantiate_chunk();
	void unload_chunk(Chunk* chunk);
	void clear();
	bool is_all_loaded();
	void force_reload();
	void measure_elapsed_time(); 

	// Used to set the loaded region of the world, new_center is usually the player's position
	void set_loaded_region_center(Vector3 new_center);
	void update_loaded_region();
	void initialize_chunk(uint64_t index); // MT
	void initialize_chunk_decorations(uint64_t index); // MT
	void initialize_structure(uint64_t index); // MT

	// Helpful interfacing methods
	void simulate_dynamic();
	bool is_position_loaded(Vector3 position) const;
	bool is_position_loading(Vector3 position) const;
	bool is_chunk_in_radius(Vector3i coordinate, int64_t radius) const;
	Chunk* get_chunk_at(Vector3 position); // Gets the chunk nearest to the given position
	Vector3i get_region_coordinate(Vector3 position) const;
	Vector3i snap_to_chunk(Vector3 position) const; // Snaps a position to the nearest chunk's position
	Vector3i snap_to_nearest_structure(Vector3 position) const;
	Ref<Block> get_block_type_at(Vector3 position);
	Ref<Structure> get_nearest_structure(Vector3 position);
	bool is_within_structure(Vector3 position);
	Vector3i find_closest_cutscene_block(Vector3 starting_position, TypedDictionary<Vector3i, bool> collected_blocks);
	bool is_block_solid_at(Vector3 position);
	bool is_block_index_solid(int32_t index);
	void break_block_at(Vector3 position, bool play_effect, bool override_restrictions);
	void place_block_at(Vector3 position, Ref<Block> block_type, bool play_effect, bool immediate_remesh);
	void explode_at(Vector3 position, int radius, bool firey);
	void explode_placement_at(Vector3 position, Ref<Block> block_type, int radius, float drop_chance); // Assumes never immediate remesh
	void flood_at(Vector3 position, int radius);
	bool is_chunk_modified(Vector3 position);
	void modify_chunk(Vector3 position);
	void liven_chunk(Chunk* chunk, Vector3i coordinate);
	void remesh_pending_chunks();
	TypedDictionary<Vector3i, int> initialize_challenge(Ref<Decoration> challenge_decoration, TypedDictionary<int, int> block_replace_map);

	// After a living block is placed/created, this method is always called to create
	// a reference to the living block scene and place it in the world
	void liven_block(Vector3i position, Ref<Block> block_type);
	void kill_block(Vector3i position);
	void register_living_block(Vector3i position, Node3D* living_block);
	void unregister_living_block(Vector3i position);
	Node3D* get_living_block_at(Vector3 position);

	void place_decoration(Ref<DecorationState> decoration_state);
	void place_water_at(Vector3 position, uint8_t amount);
	uint8_t get_water_level_at(Vector3 position);
	bool is_under_water(Vector3 position);
	bool fire_eligible(Vector3 position);
	void place_fire_at(Vector3 position, uint8_t amount);
	uint8_t get_fire_at(Vector3 position);

	// Save data (old functions)
	void save_data(Dictionary data, String prefix); // deprecated
	void load_data(Dictionary data, String prefix); // for old files only

	// (new functions)
	void register_loaded_chunks(); // registers any loaded chunk data into the permanent storage map
	void clear_save_data(); // completely clears all data
	void load_region(Dictionary region); // loads a region of chunks without resetting data
	TypedDictionary<Vector3i, bool> get_regions_to_save() const; // returns a list of regions that should be polled in order to save all data (future: only dirty)
	Dictionary get_region_data(Vector3i coordinate) const; // returns the region file to be saved
	void make_all_regions_dirty(); // used to transfer old files

	// Boilerplate setters and getters
	void set_instance_radius(int64_t new_radius);
	int64_t get_instance_radius() const;

    void set_fusion_table(const PackedInt32Array new_array, int width);

};

}