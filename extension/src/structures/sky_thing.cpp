#include "../../include/structures/sky_thing.h"
#include "../../include/world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

DEFINE_CONSTRUCTORS_AND_BINDINGS(SkyThing);

int SkyThing::update_rule(int x, int y, int z, int type) {
	const int STONE = 0;
	const int CORE = 1;
	const int RIB = 2;

	uint16_t rng_seed = world->generator->seed;
	Random::scramble_rng_seed(&rng_seed, heart_position + Vector3i(x * 13, y * 17, z * 19));

	double phase = (z % 12) / 12.0;
	bool twist_layer = (phase > 0.25 && phase < 0.75);

	int stone_touch = touch_count(x, y, z, STONE);
	int core_touch = touch_count(x, y, z, CORE);
	int rib_touch = touch_count(x, y, z, RIB);

	if (type == AIR) {
		if (core_touch >= 1 && Random::randf(rng_seed) < 0.05) {
			return STONE;
		} else if (twist_layer && stone_touch >= 2 && Random::randf(rng_seed) < 0.03) {
			return RIB;
		}
	}

	if (type == CORE) {
		if (Random::randf(rng_seed) < 0.6) {
			return STONE;
		}
		return CORE;
	}

	if (type == STONE) {
		if (stone_touch <= 1 && Random::randf(rng_seed) < 0.2) {
			return AIR;
		}
		return STONE;
	}

	if (type == RIB) {
		if (rib_touch < 1 && Random::randf(rng_seed) < 0.2) {
			return AIR;
		}
		return RIB;
	}

	return type;
}