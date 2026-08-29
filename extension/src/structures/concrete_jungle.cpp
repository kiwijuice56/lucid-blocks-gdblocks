#include "../../include/structures/concrete_jungle.h"
#include "../../include/world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

DEFINE_CONSTRUCTORS_AND_BINDINGS(ConcreteJungle);

int ConcreteJungle::update_rule(int x, int y, int z, int type) {
	const int CONCRETE = 0;
	const int REBAR = 1;
	const int CORE = 2;
	const int FLOOR = 3;

	uint16_t rng_seed = world->generator->seed;
	Random::scramble_rng_seed(&rng_seed, heart_position + Vector3i(x * 3, y * 5, z * 7));

	double height_ratio = (double) z / (double) BOUND_SIZE;

	bool base_band = (height_ratio < 0.35);
	bool mid_band = (height_ratio >= 0.35 && height_ratio < 0.65);
	bool top_band = (height_ratio >= 0.85);

	bool floor_layer = ((z % 7) == 0);

	int concrete_touch = touch_count(x, y, z, CONCRETE);
	int rebar_touch = touch_count(x, y, z, REBAR);
	int core_touch = touch_count(x, y, z, CORE);
	int floor_touch = touch_count(x, y, z, FLOOR);

	if (type == AIR) {
		if (base_band && concrete_touch + core_touch >= 3) {
			return CORE;
		}
		if (floor_layer && concrete_touch >= 4) {
			return FLOOR;
		}
	}

	if (type == CORE) {
		if (!floor_layer && Random::randf(rng_seed) < 0.9) {
			return CONCRETE;
		}

		if (top_band) {
			return AIR;
		}

		return CORE;
	}

	if (type == CONCRETE) {
		if (concrete_touch <= 1) {
			return AIR;
		} else if (floor_layer) {
			return FLOOR;
		} else if (top_band) {
			return AIR;
		}

		return CONCRETE;
	}

	if (type == REBAR) {
		if (concrete_touch < 5) {
			return AIR;
		}

		return REBAR;
	}

	if (type == FLOOR) {
		if (floor_touch + concrete_touch < 3) {
			return AIR;
		} else if (!floor_layer) {
			return AIR;
		}

		return FLOOR;
	}

	return type;
}