#pragma once

#include "world.h"
#include "prediction.h"

namespace engine
{
	void update(World& world, double dt);
	Prediction predict(World world, int steps, double dt);
	bool is_bound(const World& world);
}