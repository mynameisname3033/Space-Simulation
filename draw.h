#pragma once

#include <vector>
#include <cstdint>
#include "world.h"
#include "camera.h"
#include "prediction.h"

namespace draw
{
	void draw_bodies(std::vector<uint32_t>* pixels, int width, int height, const World& world, const Camera& cam);
	void draw_paths(std::vector<uint32_t>* pixels, int width, int height, const Prediction& prediction, const World& world, const Camera& cam);
}