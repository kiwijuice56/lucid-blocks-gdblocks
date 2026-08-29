#include "../../include/structures/crystal_thing.h"
#include "../../include/world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

DEFINE_CONSTRUCTORS_AND_BINDINGS(CrystalThing);

int CrystalThing::update_rule(int x, int y, int z, int type) {
	const int CRYSTAL = 0;
	const int VEIN = 1;
	const int SEED = 2;

	uint16_t rng_seed = world->generator->seed;
	Random::scramble_rng_seed(&rng_seed, heart_position + Vector3i(x * 9, y * 13, z * 17));

	int crystal_touch = touch_count(x, y, z, CRYSTAL);
	int vein_touch = touch_count(x, y, z, VEIN);
	int seed_touch = touch_count(x, y, z, SEED);

	double height_ratio = (double) z / (double) BOUND_SIZE;

	if (type == AIR) {
		if (seed_touch >= 1 && Random::randf(rng_seed) < 0.12) {
			return CRYSTAL;
		} else if (crystal_touch >= 3 && Random::randf(rng_seed) < 0.03) {
			return VEIN;
		}
	}

	if (type == SEED) {
		if (Random::randf(rng_seed) < 0.6 - height_ratio * 0.4) {
			return CRYSTAL;
		}
		return SEED;
	}

	if (type == CRYSTAL) {
		if (crystal_touch <= 1 && Random::randf(rng_seed) < 0.3) {
			return AIR;
		} else if (Random::randf(rng_seed) < 0.01 + height_ratio * 0.05) {
			return AIR;
		}
		return CRYSTAL;
	}

	if (type == VEIN) {
		if (crystal_touch == 0) {
			return AIR;
		}	
		return VEIN;
	}

	return type;
}