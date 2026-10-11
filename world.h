#pragma once

#include <vector>
#include <cstdint>
#include "vec2.h"

struct World
{
	std::vector<Vec2> positions;
	std::vector<Vec2> velocities;
	std::vector<double> masses;

	std::vector<Vec2> accelerations;
	std::vector<bool> stopped;

	std::vector<double> radii;
	std::vector<uint32_t> colors;

	inline void add_body(Vec2 position, Vec2 velocity, double mass, double radius, uint32_t color)
	{
		positions.push_back(position);
		velocities.push_back(velocity);
		masses.push_back(mass);

		accelerations.push_back(Vec2());
		stopped.push_back(false);

		radii.push_back(radius);
		colors.push_back(color);
	}

	inline size_t get_body_count() const { return positions.size(); }
};