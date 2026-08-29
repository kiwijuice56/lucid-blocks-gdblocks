#include "../include/register_types.h"

#include "../include/chunk.h"
#include "../include/item_loader.h"
#include "../include/world.h"
#include "../include/block.h"
#include "../include/item.h"
#include "../include/item_state.h"
#include "../include/loot.h"
#include "../include/essence.h"
#include "../include/tesseract.h"
#include "../include/ik_leg_solver.h"

#include "../include/decorations/decoration.h"
#include "../include/decorations/tree_decoration.h"
#include "../include/decorations/ambiguous_decoration.h"
#include "../include/decorations/rock_decoration.h"
#include "../include/decorations/house_decoration.h"
#include "../include/decorations/clay_tower_decoration.h"
#include "../include/decorations/simple_decoration.h"

#include "../include/decorations/wfc_tile.h"

#include "../include/decoration_state.h"
#include "../include/generators/generator.h"

#include "../include/biomes/biome.h"
#include "../include/biomes/debug_biome.h"
#include "../include/biomes/plain_biome.h"
#include "../include/biomes/city_biome.h"
#include "../include/biomes/tube_biome.h"
#include "../include/biomes/babel_biome.h"
#include "../include/biomes/ambiguous_biome.h"
#include "../include/biomes/flats_biome.h"
#include "../include/biomes/algae_biome.h"
#include "../include/biomes/ball_biome.h"
#include "../include/biomes/flesh_biome.h"
#include "../include/biomes/farlands_biome.h"
#include "../include/biomes/snow_biome.h"
#include "../include/biomes/flood_biome.h"
#include "../include/biomes/desert_biome.h"
#include "../include/biomes/forest_biome.h"
#include "../include/biomes/gallery_biome.h"
#include "../include/biomes/prison_biome.h"
#include "../include/biomes/anger_biome.h"
#include "../include/biomes/pool_biome.h"
#include "../include/biomes/void_biome.h"
#include "../include/biomes/super_flat_biome.h"
#include "../include/biomes/fractal_biome.h"
#include "../include/biomes/abble_biome.h"
#include "../include/biomes/checker_biome.h"
#include "../include/biomes/pillar_biome.h"
#include "../include/biomes/farm_biome.h"
#include "../include/biomes/window_biome.h"
#include "../include/biomes/brutal_biome.h"
#include "../include/biomes/clown_biome.h"
#include "../include/biomes/lain_biome.h"
#include "../include/biomes/water_cube_biome.h"
#include "../include/biomes/night_biome.h"
#include "../include/biomes/lotus_biome.h"
#include "../include/biomes/julia_biome.h"
#include "../include/biomes/warped_biome.h"
#include "../include/biomes/highway_biome.h"
#include "../include/biomes/hell_biome.h"
#include "../include/biomes/moon_biome.h"
#include "../include/biomes/naraka_wasteland_biome.h"
#include "../include/biomes/shower_biome.h"
#include "../include/biomes/pocket_biome.h"
#include "../include/biomes/frutiger_biome.h"
#include "../include/biomes/menger_biome.h"
#include "../include/biomes/light_biome.h"
#include "../include/biomes/ground_rust_biome.h"
#include "../include/biomes/wiggler_biome.h"
#include "../include/biomes/tree_mirror_biome.h"
#include "../include/biomes/flower_biome.h"
#include "../include/biomes/fudge_biome.h"
#include "../include/biomes/temple_town_biome.h"
#include "../include/biomes/aether_biome.h"
#include "../include/biomes/memory_biome.h"
#include "../include/biomes/challenge_biome.h"
#include "../include/biomes/firmament_biome.h"
#include "../include/biomes/dungeon_biome.h"
#include "../include/biomes/yhvh_biome.h"
#include "../include/biomes/slender_biome.h"
#include "../include/biomes/abyss_biome.h"
#include "../include/biomes/swan_biome.h"
#include "../include/biomes/random_biome.h"
#include "../include/biomes/alien_biome.h"
#include "../include/biomes/cosmic_island_biome.h"
#include "../include/biomes/null_biome.h"
#include "../include/biomes/mars_biome.h"
#include "../include/biomes/digital_space_biome.h"
#include "../include/biomes/lucy_biome.h"

#include "../include/structures/structure.h"
#include "../include/structures/pyramid.h"
#include "../include/structures/sappy_town.h"
#include "../include/structures/clay_tower.h"
#include "../include/structures/cube_temple.h"
#include "../include/structures/sponge.h"
#include "../include/structures/virus.h"
#include "../include/structures/decoration_structure.h"
#include "../include/structures/wfc_structure.h"
#include "../include/structures/fridge_structure.h"
#include "../include/structures/cellular_structure.h"
#include "../include/structures/wood_temple.h"
#include "../include/structures/concrete_jungle.h"
#include "../include/structures/crystal_thing.h"
#include "../include/structures/sky_thing.h"

#include <gdextension_interface.h>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

using namespace godot;

void initialize_gdblocks_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	GDREGISTER_RUNTIME_CLASS(Item);
	GDREGISTER_RUNTIME_CLASS(ItemLoader);
	GDREGISTER_RUNTIME_CLASS(ItemState);
	GDREGISTER_RUNTIME_CLASS(Essence);
	GDREGISTER_RUNTIME_CLASS(Block);
	GDREGISTER_RUNTIME_CLASS(Tesseract);
	GDREGISTER_RUNTIME_CLASS(IKLegSolver);

	GDREGISTER_RUNTIME_CLASS(Decoration);
	GDREGISTER_RUNTIME_CLASS(TreeDecoration);
	GDREGISTER_RUNTIME_CLASS(RockDecoration);
	GDREGISTER_RUNTIME_CLASS(AmbiguousDecoration);
	GDREGISTER_RUNTIME_CLASS(HouseDecoration);
	GDREGISTER_RUNTIME_CLASS(ClayTowerDecoration);
	GDREGISTER_RUNTIME_CLASS(SimpleDecoration);

	GDREGISTER_RUNTIME_CLASS(WfcTile);
	GDREGISTER_RUNTIME_CLASS(DecorationState);
	GDREGISTER_RUNTIME_CLASS(Chunk);

	GDREGISTER_RUNTIME_CLASS(Generator);

	GDREGISTER_RUNTIME_CLASS(Biome);
	GDREGISTER_RUNTIME_CLASS(DebugBiome);
	GDREGISTER_RUNTIME_CLASS(PlainBiome);
	GDREGISTER_RUNTIME_CLASS(CityBiome);
	GDREGISTER_RUNTIME_CLASS(TubeBiome);
	GDREGISTER_RUNTIME_CLASS(BabelBiome);
	GDREGISTER_RUNTIME_CLASS(FlatsBiome);
	GDREGISTER_RUNTIME_CLASS(AmbiguousBiome);
	GDREGISTER_RUNTIME_CLASS(AlgaeBiome);
	GDREGISTER_RUNTIME_CLASS(BallBiome);
	GDREGISTER_RUNTIME_CLASS(FleshBiome);
	GDREGISTER_RUNTIME_CLASS(FarlandsBiome);
	GDREGISTER_RUNTIME_CLASS(SnowBiome);
	GDREGISTER_RUNTIME_CLASS(FloodBiome);
	GDREGISTER_RUNTIME_CLASS(DesertBiome);
	GDREGISTER_RUNTIME_CLASS(ForestBiome);
	GDREGISTER_RUNTIME_CLASS(GalleryBiome);
	GDREGISTER_RUNTIME_CLASS(PrisonBiome);
	GDREGISTER_RUNTIME_CLASS(AngerBiome);
	GDREGISTER_RUNTIME_CLASS(PoolBiome);
	GDREGISTER_RUNTIME_CLASS(VoidBiome);
	GDREGISTER_RUNTIME_CLASS(SuperFlatBiome);
	GDREGISTER_RUNTIME_CLASS(FractalBiome);
	GDREGISTER_RUNTIME_CLASS(AbbleBiome);
	GDREGISTER_RUNTIME_CLASS(CheckerBiome);
	GDREGISTER_RUNTIME_CLASS(PillarBiome);
	GDREGISTER_RUNTIME_CLASS(FarmBiome);
	GDREGISTER_RUNTIME_CLASS(WindowBiome);
	GDREGISTER_RUNTIME_CLASS(BrutalBiome);
	GDREGISTER_RUNTIME_CLASS(ClownBiome);
	GDREGISTER_RUNTIME_CLASS(LainBiome);
	GDREGISTER_RUNTIME_CLASS(WaterCubeBiome);
	GDREGISTER_RUNTIME_CLASS(NightBiome);
	GDREGISTER_RUNTIME_CLASS(LotusBiome);
	GDREGISTER_RUNTIME_CLASS(JuliaBiome);
	GDREGISTER_RUNTIME_CLASS(WarpedBiome);
	GDREGISTER_RUNTIME_CLASS(HighwayBiome);
	GDREGISTER_RUNTIME_CLASS(HellBiome);
	GDREGISTER_RUNTIME_CLASS(MoonBiome);
	GDREGISTER_RUNTIME_CLASS(NarakaWastelandBiome);
	GDREGISTER_RUNTIME_CLASS(ShowerBiome);
	GDREGISTER_RUNTIME_CLASS(PocketBiome);
	GDREGISTER_RUNTIME_CLASS(FrutigerBiome);
	GDREGISTER_RUNTIME_CLASS(MengerBiome);
	GDREGISTER_RUNTIME_CLASS(LightBiome);
	GDREGISTER_RUNTIME_CLASS(GroundRustBiome);
	GDREGISTER_RUNTIME_CLASS(WigglerBiome);
	GDREGISTER_RUNTIME_CLASS(TreeMirrorBiome);
	GDREGISTER_RUNTIME_CLASS(FlowerBiome);
	GDREGISTER_RUNTIME_CLASS(FudgeBiome);
	GDREGISTER_RUNTIME_CLASS(TempleTownBiome);
	GDREGISTER_RUNTIME_CLASS(AetherBiome);
	GDREGISTER_RUNTIME_CLASS(MemoryBiome);
	GDREGISTER_RUNTIME_CLASS(ChallengeBiome);
	GDREGISTER_RUNTIME_CLASS(FirmamentBiome);
	GDREGISTER_RUNTIME_CLASS(DungeonBiome);
	GDREGISTER_RUNTIME_CLASS(YhvhBiome);
	GDREGISTER_RUNTIME_CLASS(SlenderBiome);
	GDREGISTER_RUNTIME_CLASS(AbyssBiome);
	GDREGISTER_RUNTIME_CLASS(SwanBiome);
	GDREGISTER_RUNTIME_CLASS(RandomBiome);
	GDREGISTER_RUNTIME_CLASS(AlienBiome);
	GDREGISTER_RUNTIME_CLASS(CosmicIslandBiome);
	GDREGISTER_RUNTIME_CLASS(NullBiome);
	GDREGISTER_RUNTIME_CLASS(MarsBiome);
	GDREGISTER_RUNTIME_CLASS(DigitalSpaceBiome);
	GDREGISTER_RUNTIME_CLASS(LucyBiome);

	GDREGISTER_RUNTIME_CLASS(Structure);
	GDREGISTER_RUNTIME_CLASS(Pyramid);
	GDREGISTER_RUNTIME_CLASS(SappyTown);
	GDREGISTER_RUNTIME_CLASS(ClayTower);
	GDREGISTER_RUNTIME_CLASS(CubeTemple);
	GDREGISTER_RUNTIME_CLASS(Sponge);
	GDREGISTER_RUNTIME_CLASS(Virus);
	GDREGISTER_RUNTIME_CLASS(DecorationStructure);
	GDREGISTER_RUNTIME_CLASS(WfcStructure);
	GDREGISTER_RUNTIME_CLASS(FridgeStructure);
	GDREGISTER_RUNTIME_CLASS(CellularStructure);
	GDREGISTER_RUNTIME_CLASS(WoodTemple);
	GDREGISTER_RUNTIME_CLASS(ConcreteJungle);
	GDREGISTER_RUNTIME_CLASS(CrystalThing);
	GDREGISTER_RUNTIME_CLASS(SkyThing);

	GDREGISTER_RUNTIME_CLASS(Loot);
	GDREGISTER_RUNTIME_CLASS(World);
}

void uninitialize_gdblocks_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
}

extern "C" {
GDExtensionBool GDE_EXPORT gdblocks_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization) {
	godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

	init_obj.register_initializer(initialize_gdblocks_module);
	init_obj.register_terminator(uninitialize_gdblocks_module);
	init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

	return init_obj.init();
}
}