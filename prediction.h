#pragma once

#include <vector>
#include "vec2.h"

struct PredictedCollision
{
	size_t body = 0;
	Vec2 position;
};

struct Prediction
{
	std::vector<std::vector<Vec2>> paths;
	std::vector<PredictedCollision> collisions;
};