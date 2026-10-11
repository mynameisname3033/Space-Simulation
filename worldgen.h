#pragma once

#include <optional>
#include "world.h"
#include "vec2.h"

namespace worldgen
{
	void init();
	std::optional<World> generate_interesting_stable_world(size_t body_count, Vec2 center, Vec2 position_bounds, Vec2 velocity_bounds, double min_mass, double max_mass, double min_radius, double max_radius, int steps, double dt, int max_attempts = 1000);
}