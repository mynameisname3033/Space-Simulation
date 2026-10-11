#include <algorithm>
#include <chrono>
#include <cmath>
#include <vector>
#include <optional>
#include "window.h"
#include "world.h"
#include "engine.h"
#include "draw.h"
#include "camera.h"
#include "prediction.h"
#include "vec2.h"
#include "worldgen.h"

static constexpr bool FULLSCREEN = true;
static constexpr int WIDTH = 1080;
static constexpr int HEIGHT = 720;

static constexpr double SIM_DT = 1.0 / 120.0;
static constexpr int MAX_SAVED_STEPS = 300000;
static constexpr double PLAY_RATE = 1.0;
static constexpr double PLAY_RATE_INCREMENT = 3.0;

static constexpr int PREDICT_STEPS = 30000;

static constexpr double ZOOM_STEP = 1.05;
static constexpr double ZOOM_MIN = 0.05;
static constexpr double ZOOM_MAX = 120.0;

static constexpr double ADDITIONAL_GENERATION_WINDOW_MULTIPLIER = 1.0;

int main(int argc, char* argv[])
{
	Window win;
	if (!window::create(win, "Space Simulation", FULLSCREEN, WIDTH, HEIGHT))
	{
		return 1;
	}

	worldgen::init();
	std::optional<World> optional_world = worldgen::generate_interesting_stable_world(4, Vec2(win.width / 2.0, win.height / 2.0), Vec2(win.width / 2.0 * ADDITIONAL_GENERATION_WINDOW_MULTIPLIER, win.height / 2.0 * ADDITIONAL_GENERATION_WINDOW_MULTIPLIER), Vec2(-150, 150), 1.0e5, 1.0e7, 1, 10, PREDICT_STEPS * 3, SIM_DT, 1000000);
	if (!optional_world.has_value())
		return 1;

	World world = optional_world.value();

	//world.add_body({ win.width / 2.0 - 413, win.height / 2.0 + 43 }, { -15.0, -8.0 }, 1.2e6, 5, 0xFFFFF082);
	//world.add_body({ win.width / 2.0 + 309, win.height / 2.0 }, { 6.0, 27.0 }, 1.0e6, 5, 0x0090EE90);
	//world.add_body({ win.width / 2.0 + 50, win.height / 2.0 - 38 }, { 19.0, -10.0 }, 1.6e6, 5, 0xFFADD8E6);

	//world.add_body({ win.width / 2.0 - 43.5, win.height / 2.0 + 41.6 }, { -15.0, 93.0 }, 1.2e6, 5, 0xFFFFF082);
	//world.add_body({ win.width / 2.0 + 300.1, win.height / 2.0 }, { 36.0, 38.0 }, 1.0e6, 5, 0x0090EE90);
	//world.add_body({ win.width / 2.0 + 50.3, win.height / 2.0 - 49 }, { 18.0, -42.0 }, 1.6e6, 5, 0xFFADD8E6);
	
	//world.add_body({ win.width / 2.0 - 731, win.height / 2.0 + 43 }, { -16.5, -20.0 }, 1.2e6, 5, 0xFFFFF082);
	//world.add_body({ win.width / 2.0 + 309, win.height / 2.0 }, { 6.0, 27.0 }, 1.0e6, 5, 0x0090EE90);
	//world.add_body({ win.width / 2.0 + 50, win.height / 2.0 - 38 }, { 19.0, -10.0 }, 1.6e6, 5, 0xFFADD8E6);

	//world.add_body({ win.width / 2.0 + 608, win.height / 2.0 }, { -10.0, -27.0 }, 1.0e6, 4, 0x0090EE90);
	//world.add_body({ win.width / 2.0 + 50, win.height / 2.0 - 38 }, { 19.0, -10.0 }, 1.6e6, 7, 0xFFADD8E6);
	//world.add_body({ win.width / 2.0 - 130, win.height / 2.0 - 38 }, { 30.0, 80.0 }, 3.6e6, 10, 0x00FFB6C1);
	//world.add_body({ win.width / 2.0 - 331, win.height / 2.0 + 43 }, { 10.5, -60.0 }, 1.2e6, 5, 0xFFFFF082);

	//world.add_body({ win.width / 2.0, win.height / 2.0 }, { -15.0, -20.0 }, 20.0e6, 30, 0xFFFFF082);
	//world.add_body({ win.width / 2.0 + 800, win.height / 2.0 }, { 6.0, -130.0 }, 1.6e5, 2, 0x0090EE90);
	//world.add_body({ win.width / 2.0 + 810, win.height / 2.0 - 30.0 }, { -35.0, -150.0 }, 1.0e4, 1, 0x0090EE90);

	std::vector<World> history;
	auto record = [&](const World& w)
	{
		history.push_back(w);
		if (history.size() > MAX_SAVED_STEPS)
			history.erase(history.begin());
	};

	Camera cam;

	int last_mouse_x = 0, last_mouse_y = 0;
	bool dragging = false;

	double play_rate = PLAY_RATE;
	double dt = 1.0 / 60.0;
	double accumulator = 0.0;

	bool sim = true;
	bool predict = true;
	bool running = true;

	while (running)
	{
		auto start = std::chrono::steady_clock::now();
		
		Input input = window::poll_events(win);

		if (input.quit)
			running = false;

		if (input.mouse_held)
		{
			if (dragging)
			{
				cam.x -= (input.mouse_x - last_mouse_x) / cam.zoom;
				cam.y -= (input.mouse_y - last_mouse_y) / cam.zoom;
			}
			last_mouse_x = input.mouse_x;
			last_mouse_y = input.mouse_y;
			dragging = true;
		}
		else
		{
			dragging = false;
		}

		if (input.mouse_wheel != 0)
		{
			Vec2 mouse(input.mouse_x, input.mouse_y);
			Vec2 world_before = Vec2(cam.x, cam.y) + mouse / cam.zoom;

			cam.zoom *= std::pow(ZOOM_STEP, input.mouse_wheel);
			cam.zoom = std::clamp(cam.zoom, ZOOM_MIN, ZOOM_MAX);

			Vec2 offset = world_before - mouse / cam.zoom;
			cam.x = offset.x;
			cam.y = offset.y;
		}

		if (input.r_key)
			play_rate = PLAY_RATE;

		if (input.up_key)
			play_rate += PLAY_RATE_INCREMENT * dt;
		if (input.down_key)
			play_rate -= PLAY_RATE_INCREMENT * dt;
		play_rate = std::max(0.0, play_rate);

		if (input.space_key)
			sim = !sim;

		if (input.p_key)
			predict = !predict;

		if (sim)
		{
			accumulator += dt * play_rate;
			accumulator = std::min(accumulator, 0.25);

			while (accumulator >= SIM_DT)
			{
				record(world);
				engine::update(world, SIM_DT);
				accumulator -= SIM_DT;
			}
		}
		else if (input.right_key)
		{
			int steps = std::max(1, static_cast<int>(play_rate + 0.5));
			for (int i = 0; i < steps; ++i)
			{
				record(world);
				engine::update(world, SIM_DT);
			}
		}
		else if (input.left_key)
		{
			int steps = std::max(1, static_cast<int>(play_rate + 0.5));
			for (int i = 0; i < steps; ++i)
			{
				if (history.empty())
					break;
				world = history.back();
				history.pop_back();
			}
		}
		else
		{
			accumulator = 0.0;
		}

		Prediction prediction;
		if (predict)
			prediction = engine::predict(world, PREDICT_STEPS, SIM_DT);

		std::fill(win.pixels.begin(), win.pixels.end(), 0xFF000000);
		draw::draw_paths(&win.pixels, win.width, win.height, prediction, world, cam);
		draw::draw_bodies(&win.pixels, win.width, win.height, world, cam);

		window::present(win);

		auto end = std::chrono::steady_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
		dt = duration.count() / 1000000000.0;
	}

	window::destroy(win);
	return 0;
}