#pragma once

#include <vector>
#include <cstdint>

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;

struct Input
{
	bool quit = false;

	bool mouse_held = false;
	int mouse_x = 0;
	int mouse_y = 0;
	int mouse_wheel = 0;

	bool space_key = false;
	bool left_key = false;
	bool right_key = false;
	bool up_key = false;
	bool down_key = false;

	bool r_key = false;
	bool p_key = false;
};

struct Window
{
	SDL_Window* handle = nullptr;
	SDL_Renderer* renderer = nullptr;
	SDL_Texture* texture = nullptr;

	int width = 0;
	int height = 0;

	std::vector<uint32_t> pixels;
};

namespace window
{
	bool create(Window& win, const char* title, bool fullscreen, int width, int height);
	Input poll_events(const Window& win);
	void present(Window& win);
	void destroy(Window& win);
}