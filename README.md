# lucid-blocks-gdblocks
The voxel engine gdextension used in Lucid Blocks. This is NOT the repository for the entire game, only the C++ code needed for specific features such as chunk loading, biome generation, and the water simulation. Feel free to contact me on the Lucid Blocks discord server or at lucykiwijuice(at)gmail.com if you have any modding questions!

## Mod Outline
The game loads all `.pck` files in the `mods` directory placed next to the `.exe` ([see Godot documentation here](https://docs.godotengine.org/en/stable/tutorials/export/exporting_pcks.html)). When exporting your mod, please make sure you include only the modified/new files (excluding dependencies as well)! This will greatly increase the compatability of your mod.

## Compiling
Note: Lucid Blocks uses *double precision* Godot, which has to be manually compiled using the `precision=double` flag. The GDExtension compilation as described below accounts for this. However, GodotSteam has to be compiled using the `precision=double` and `custom_api_file="extension_api.json` flags. I decided to do this manually rather than make GodotSteam a submodule, but the process is relatively simple. The GodotSteam `.gdextension` file should be updated to point to the new binaries. Due to a Godot bug, `CAMERA_POSITION_WORLD` is inverted in any shader when double precision is enabled. All shaders account for this already, but this will need to change if the project is ever upgraded to a later version.
### Addons
1) [SoFluffy 1.1.0](https://github.com/maxvolumedev/sofluffy). Minor note: must change `camera.transform` instances in code to `camera.global_transform`.
2) [GodotSteam 4.17.1](https://godotsteam.com/). Minor note: must recompile and change `.gdextension` file to point to double precision binaries.
### Debug
1) Nagivate to the root folder and run `scons debug_symbols=yes precision=double custom_api_file="extension_api.json"`.
3) Enable `debug` on the root node in `game/main/main.tscn` to unlock developer features.
4) Run the project from the editor.
### Release
1) Nagivate to the root folder and run `scons precision=double target=template_release custom_api_file="extension_api.json"`.
3) Ensure that `debug` is disabled on the root node in `game/main/main.tscn`.
4) Export the project (without debug) using the main Windows configuration. Files will be placed in `build/`.
### Release checklist
1) `debug` toggle disabled
2) Version tag updated


## Code Structure
### Entities
Entities are the base class for the player and other creatures (important: not all interactive objects are entities, such as dropped items). `Entity` scripts act as the "glue" for the various components that make up a specific entity, such as its stats, mesh instances, or collision shapes. The majority of logic for an entity is handled via composition using `Behavior` scripts, each which implement specific behaviors like `PickUpItems` or `DropItems`.

#### Entity Boilerplate
First, inherit the base entity scene `res://main/entity/entity.tscn` and inherit from the base entity script. You can use the script below as a template (pay attention to the lines marked as important, and don't remove them unless you know what you're doing):
```
class_name MyEntity extends Entity

# example of state
enum { EXAMPLE_STATE_STUFF, IDLE, SAD }
var state: int = IDLE

func _ready() -> void:
	super._ready() # ! important

	modulate_changed.connect(_on_modulate_changed) # ! important
	alpha_changed.connect(_on_alpha_changed)       # ! important

	# other set up here, such as connecting signals...


func _physics_process(delta: float) -> void:
	# ! important: don't let your entity act while its disabled or on unloaded terrain
	if disabled or not Ref.world.is_position_loaded(global_position):
		return
	# ! important: if you want water/fire checks + default velocity handling
	super._physics_process(delta) 
	# ! imporant: check is_future_position() to not go into unloaded terrain, 
	#   and call move_and_slide() to move based on velocity and collisions
	if is_future_position_loaded(delta):
		move_and_slide()

	if dead:
		pass # maybe your entity does some weird animation after death?
	else:
		pass # normal behavior

# ! important: the "modulate" is used to tint your entity red after its hit!
#              make sure any body parts of your entity react accordingly
func _on_modulate_changed(new_modulate: Color) -> void:
	pass
	# example: 
	# %BodyMesh.set("instance_shader_parameters/tint_color", new_modulate)


# ! important: same as above, but used to make your entity fade out after death             
func _on_alpha_changed(new_alpha: float) -> void:
	pass
	# example:
	# %BodyMesh.set("instance_shader_parameters/fade", new_alpha)
	#
	# ... then in your fragment shader:
	# const vec3 magic = vec3(0.06711056, 0.00583715, 52.9829189);
	# if (fade < fract(magic.z * fract(dot(FRAGCOORD.xy, magic.xy)))) {
	# 	discard;
	# }

 
func preserve_save(file: SaveFile, uuid: String) -> void:
	super.preserve_save(file, uuid) # ! important

	# example of saving state
	file.set_data("node/%s/state" % uuid, state) 


func preserve_load(file: SaveFile, uuid: String) -> void:
	super.preserve_load(file, uuid)  # ! important

	# example of loading state
	state = file.get_data("node/%s/state" % uuid, IDLE)
```

When setting up your scene, make sure you set the `head` and `hand` marker variables. These should point to `Marker3D` nodes placed under their respective body parts. This is not too important, but the base code uses it to place held items correctly, for example. Lastly, add some behavior children under the `Behaviors` node. If a behavior has a packed scene, you *must* use it since the script relies on it. Otherwise, you can attach the behavior script to a new `Node3D`. You must point the `entity` property to the root entity node. 

### Save Data

#### Save File Format
Save files are split into two parts: a `SaveFile` resource and a `SaveFileRegister` resource. `SaveFileRegister` extends from `SaveFile` and as such is mostly equivalent, but the register file is intended to store only critical information about a save file -- the name, save version and date, and other data shown on the save file screen. This allows the save file screen to be responsive even if save files become too large.

A UUID (stored in the register) is used to identify a save file. Both files are written to a folder named after this UUID as `data.txt` and `register.txt`. This format makes it seamless to share over Steam Workshop.

Data in both types of files is stored in a nested dictionary. The `set_data` and `get_data` methods can be used to traverse this dictionary using file-like paths that terminate in a serializable value (e.g. `random_game_data/bobbles/health = 10` can be set using `file.set_data('random_game_data/bobbles/health', 10)`). When the game is saved, this data dictionary is converted to JSON and stored in a `.txt` file (to prevent arbitrary resource loading).

#### Saving Node Data
The actual saving of data is handled by organizing nodes into the following groups:
- `save`
- `preserve`
- `preserve_but_delete_on_unload`
- `save_register`
- `delete_on_quit`

`save` is the most standard group. When a game is saved, all nodes in this group will have their `save_file(file: File) -> void` method called. The node should save any necessary information into file using `file.set_data(property_path: String, data: Variant) -> void`, which stores `data` in a nested dictionary in the path `property_path`. All data in the dictionary should be serializable, since it is saved in the `.json` format. This method creates any path if it does not exist. `load_file(file: File) -> void` is similar, allowing the node to call `file.load_data(property_path: String, default: Variant) -> Variant`. Use this to restore the state of the node or set the default state.

For example, see the methods on `game/main/world/sun.gd`:

```
func save_file(file: SaveFile) -> void:
	file.set_data("sun/time", time)

func load_file(file: SaveFile) -> void:
	time = file.get_data("sun/time", 0.8)
```

*Only nodes that always exist should be in this group*, i.e., not nodes that are instanced or deleted during gameplay. Instead, temporary nodes should be in either group `preserve` or `preserve_but_delete_on_unload`. Placing a node in this group will not only save its data, but also force it to load and unload along with chunks, thus requiring the node to be a descendent of `Node3D`. Nodes in the `preserve` group must also be placed under the tree root (`get_tree().get_root().add_child(...)`) for that node to be automatically detected and saved.

Nodes in `preserve` persist both when the game closes and when the nearest chunk is unloaded, while nodes in `preserve_but_delete_on_unload` only persist when the game is closed. The former is intended for nodes such as dropped items, while the latter is intended for nodes such as entities. `preserve_on_modified_chunks_only` is preserved only when the chunk it is contained within is modified, both when a chunk is unloaded or the game is closed. This group is primarily intended for living blocks.

The syntax for saving/loading data is similar to that of nodes in `save`, consisting of two methods: `preserve_save(file: SaveFile, uuid: String) -> void` and `preserve_load(file: SaveFile, uuid: String) -> void`. The instructions are the same, but the `uuid` parameter should be used to give this node a unique property path.

For example, see the implementation on `game/main/entity/sheep/sheep.gd`:

```
func preserve_save(file: SaveFile, uuid: String) -> void:
	super.preserve_save(file, uuid)
	file.set_data("node/%s/desired_angle" % uuid, desired_angle)

func preserve_load(file: SaveFile, uuid: String) -> void:
	super.preserve_load(file, uuid)
	desired_angle = file.get_data("node/%s/desired_angle" % uuid, 0)
	%RotationPivot.rotation.y = desired_angle
```

No extra logic is needed for nodes that are deleted during gameplay (such as entities dying) -- saving and loading of data is completely handled by the engine as long as both `preserve_load` and `preserve_save` are implemented. However, `preserve_load` is not called when the node is first added to the tree, so initialization logic in `_ready()` is still necessary. Usually, it is best to create some sort of `initialize()` method that reads variables stored on the node (which are set by `_ready()` for new nodes and then overriden by `preserve_load` for saved nodes) to set up initial state.

In addition, only root nodes in a scene should be part of `preserve` or `preserve_but_delete_on_unload`. Nodes that preserve the state of their modular children should do so by calling their `preserve_save` and `preserve_load` methods explicitly (note that the base Entity script does this by default, so only a `super.preserve_save(file, uuid)` and `super.preserve_load(file, uuid)`call is needed):

```
for child in find_children("*"):
    if "preserve_save" in child:
        child.preserve_save(file, uuid)
```

`save_register` is the same as `save`, but handles the register file instead. It is extremely unlikely that you will need to use this group.

Finally, the `delete_on_quit` does not save any data, but instead ensures that a node will be deleted when a game is closed. Nodes for one-off visual or sound effects should be placed in this group to prevent them from persisting across save files.

Note: due to the chunk-based save system, nodes are only removed from the tree and saved when the chunk corresponding to their current position is unloaded. *This means that nodes on unloaded chunks will not be saved or freed.* It is important that any preserve node prevents itself from moving into an unloaded chunk.


### Items
Static information about items (such as the display name and icon) is stored using `Item` resources. An `Item` resource exists for every type of item in the game, such as blocks and tools. Dynamic information is stored in `ItemState` resources, which stores the `id` of a particular `Item` along with variables such as the number of blocks in a stack or the durability of a tool. The `Item` resource belonging to an `id` can be quickly accessed using the `ItemMap` autoload script. Finally, the behavior of an item when it is held by an entity is coded using `HeldItem` scenes. `HeldItem` scenes are not necessarily unique to a specific `Item` type.

For a quick tutorial of how items work in-game, consider the example of the player switching their held item to a sword in the hotbar: The player stores their items in an array of `ItemState` resources associated with an `Inventory` node. The icon, name, and `HeldItem` scene of the sword are acquired by finding the `Item` resource whose `id` matches the `ItemState` of the currently selected item. Next, the `HeldItem` scene is instantiated and placed at the location of the player's hand, where it is initialized according to the information in the `ItemState`. During this initialization, `HeldItem` increases the attack stat of the player (and decreases it when it is unheld).

### Hand Sprites
Hand sprites are separate from the actual items. At runtime, the game loads all `PlayerHandVariant` subclasses from `res://main/entity/player/hand/hand_variants/`. Each of these scripts (associated with a `Node2D` scene that contains some `Sprite` and `AnimationPlayer` nodes) checks if any given item matches with that hand by overriding `matches_with_item(item: Item) -> bool`. Each hand variant scene must have 2-3 sprites: `Idle`, `Attack`, and `Interact` (each with the corresponding group flag set). If a hand has the same sprite for interacting and attacking, keep only the `Attack` node and give it both the `interact` and `attack` groups. Under each sprite should be an `AnimationPlayer` with only one animation named `trigger`. You may also need to reposition the root node (Godot will warn you about this, but it's expected in this case) in order for it to fit on screen closely with the other hands. 

### Blocks, Water and Fire
Entities, behaviors, and held items can interact with the environment via the `World` singleton (accessed using `Ref.world` in GDscript). The world class contains the state of all blocks and water. The majority of complex logic is handled by `World`, so scripts can rely on simple methods such as `break_block_at` and `place_water_at` as long as any queried positions are loaded (*must* be verified manually using `is_position_loaded`). Water and fire simulations are updated approximately every frame, but some chunks often pause simulations to save on performance.

#### Block Indices
While blocks have their own `id` like any other item, they also have an internal `index` property set by the game at initialization. Many parts of the game store all blocks sequentially in an array, so the `index` property keeps track of where in the array that block is. Most of the Godot code does not concern itself with this, but occasionally it is needed to interface with shaders.

### Internal Items
Some items in the game are given the `internal` flag, which means the player should never have access to that item. Most of these items are used for debug purposes, but this flag is also given to any automatically generated directional blocks. Any mechanic that generates random items should filter out internal items.

In addition, the first 20 block indices are reserved for debug blocks. The `id` of each of these blocks is the same as their index for clarity. These are all flagged as `internal`:
- 0 air block
- 1 void block
- 2 debug
- 3 replace_1
- 4 replace_2
- 5 replace_3
- 6 replace_4
- 7 replace_5
- 8 replace_6
- 9 replace_7
- 10 replace_8
- 11 replace_9
- 12 replace_10
- 13 direction block
- 14 direction block y+
- 15 direction block y-
- 16 direction block z+
- 17 direction block z-
- 18 direction block x+
- 19 direction block x-
- 20 replace_water

Note: any biome or mechanic that makes use of random block selection should filter out respawn blocks and heaven blocks (these aren't internal, but *respawn blocks cannot be placed by world generation without corrupting respawn state* and *the divine dimension has no portal back to the naraka dimension, so players become stuck if they access this block from a survival qualia*).

#### Block Textures
Block textures (90 x 15 pixels, ordered top, bottom, z-, z+, x-, x+ from left to right) are automatically stored in a texture array generated by `World`. Should any shader need to access this array, a reference to `ShaderMaterial` should be added to the `requires_texture_atlas` array of `World`.
The shader uniform should be named `textures`. Textures of blocks are stored according to their `index` property.

The `textureless` flag is an optimization that allows the texture array to exclude a block (important because the size is limited on some platforms). Textureless blocks are given indices larger than textured blocks, so their index goes out of bounds of the texture array by default. The texture array includes a transparent texture at the end. Shaders accessing the block texture array should clamp any out of bounds index to the final item so that textureless blocks are not rendered. Note that a textureless block must also be flagged `transparent`, or else a black cube will render in its place.

#### Block Icons
Block icons are automatically generated when the game starts. To add a custom icon, enable `override_icon` -- this will stop the generator from replacing the icon within the resource file.

#### Block Drops
The `can_drop` flag is unfortunately named -- a better name is `will_drop_self`. It indicates whether a block will drop itself when broken, regardless of the other settings. `drop_item` and `drop_loot` will be dropped separately.

#### Directional Blocks
Blocks with the `directional` flag generate six new blocks when the game is loaded, one for each direction. Directional blocks need six open ID slots after the main ID for these blocks. Directional block variants themselves are not accessible in gameplay; The `HeldBlock` accepts the primary block and places the correct directional variant according to orientation.

#### Living Blocks
Blocks that have extra state attached to them (like cache cubes and cutscene blocks) are called living blocks. Living blocks have two counterparts: the actual block ID stored in chunks like any regular block, and an instance of the `LivingBlock` scene associated with the block at that position. `LivingBlock` nodes are much like entities, allowing the storage of properties like inventories and interactions with entities.

When a block is placed during gameplay (by a player or other entity), the `place_block` function creates a `LivingBlock` using `liven_block` and registers it into the `World` map, allowing it to be tracked and accessed by other interfacing methods of `World`. `kill_block` is used to destroy instances of `LivingBlock` nodes when the corresponding block is broken. In general, the `World` and its interfacing methods maintain the 1 to 1 relationship between `LivingBlock` nodes and actual living blocks in any loaded chunks. `liven_block` should only need to be manually called by world generation methods when block data is modified in bulk. `LivingBlock` nodes are always in the `preserve_on_modified_chunks_only` group, so they are saved/loaded without extra work from `World`. Living blocks can also be added to chunk data during world generation; they are initialized before the chunk is loaded.

### World Generation
Generation is handled by `Generator` resources, which contain a random seed and a list of `Biome` resources. A generator first uses the `height` and `proportion` properties of its biomes to determine the random distribution of biomes per seed. `proportion` refers to how common a biome is (relative to the proportion of all other biomes in this generator), while `height` specifies at what height this biome should spawn. Each height encapsulates ~256 blocks. Biomes that appear near the surface (around `y=40`) should be at height 6, while biomes below should be at height 5 and so on.

Biomes themselves are coded in C++ for best performance. The block at any position within a biome is deterministic using the generator's noise layers combined with a save file's seed value.

#### Decorations
Decorations are prefabs for world generation that can cross chunk borders. To create a decoration, ensure that `debug` is enabled on the root scene, then create a new divine world. You can build decorations in the cueb with corners at (0, 0, 0) and (32, 32, 32). Once complete, press `C` on your keyboard to capture the decoration and save it to the path specified by the world's `decoration_to_edit_path` variable. Give the new decoration resource an appropriate `center_offset` and `internal_name` to access it from the world's `decoration_name_map` and generate it in any biome's `void generate_decorations(Vector3i chunk_position, Vector3i biome_coordinate)` method.

Note that decorations placed during regular world generation can only span up to 32x32x32 blocks if centered.

Decorations can be given custom generation logic by creating a new class in C++. When designing a decoration in-game, you can place special blocks with the internal name `replace_%d` (these should be the first blocks in the creative menu that resembled colored dice). When a decoration is transformed into blocks, `int replace(int replace_number, Vector3i position)` is called for each replacement block found. This function should use some random (but deterministic) logic to return a block index based on what `replace_number` the block originally was. For example, trees randomly replace `replace_1` blocks with either air or leaves to give them some variation. Note that you must change a captured decoration's type to match your custom decoration class, which can be done by editing the generated resource file manually.

#### Structures
Structures are dynamic prefabs for world generation, designed to fill slots of size 256x256x256 spanning the entire world. There can only exist one structure per slot. Structures are similar to decorations, but with more logic upon initialization to give each structure detail that spans across many chunks. Structures are initialized with a `root_position` (the position of the aformentioned structure slot), `heart_position` (a randomized position within the bounds of the structure), and `center_position` (`heart_position` but with `y` set to the ground position in the xz column). Note that structures can only span a size of 9x9x9 chunks in order to not be clipped at structure borders.

#### Biome Fusion
After the terrain, decorations, and structures are voxelized, a fusion pass blends together biomes to increase variety. Fusion happens at a block-by-block basis. A table is generated at start-up storing the fusion result for every pair of blocks. Since this takes ~10-20 seconds, the results are cached in a text file. If many new blocks are added, this cache can be updated by enabling `update_fusion_cache` on the player fuser node.

#### Dimensions
Each dimension in the game corresponds to its own dictionary in the save file. If a save file is marked as `is_dimensional`, then properties are automatically prefixed with a string denoting the dimension (unless `multidimensional=true` is passed as an argument to the save and load functions). This means that by default all saved/preserved properties correspond to a specific dimension, preventing interference across dimensions.

As of now, the only entity with multidimensional properties is the player: the inventory and behavior scripts pass `multidimensional=true` if and only if "player" is passed as the entity UUID. This is a small hack, but no other multidimensional entities are planned.

Each dimension has a generator resource corresponding to it.

#### Noise Layers in the Main Generator
For ease of use, noise layers are placed into an array stored in the generator resource. Any biome/structure/decoration can access any noise layer, but the following key shows what each layer is (mostly) used for. Be careful -- many biomes reuse noise of others:
- `0`: base terrain noise
- `1`: biome voronoi noise
- `2`: biome y variation noise
- `3`: random biome voronoi noise
- `4`: extra terrain noise
- `5`: extra terrain noise without fractal
- `6`: algae biome noise
- `7`: flesh biome noise
- `8`: flood biome noise
- `9`: pool biome noise
- `10`: cave noise
- `11`: (not used) -- originally intended for void biome placement
- `12`: fractal noise
- `13`: tendril noise
- `14`: fusion noise
- `15`: sponge noise
- `16`: zany noise
- `17`: night noise
- `18`: lotus noise
- `19`: river noise 1
- `20`: river noise 2
- `21`: shore sand noise
- `22`: julia noise
- `23`: flesh noise 2
- `24`: flesh noise 3
- `25`: highway noise
- `26`: fridge noise
- `27`: hell noise
- `28`: rust noise
- `29`: decoration noise
- `30`: aether cloud noise
- `31`: dungeon noise
- `32`: mars noise
