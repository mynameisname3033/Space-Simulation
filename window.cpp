#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_mouse.h>
#include <cstdint>
#include "window.h"

bool window::create(Window& win, const char* title, bool fullscreen, int width, int height)
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		return false;
	}

	if (fullscreen)
	{
		SDL_Rect display_bounds;
		if (SDL_GetDisplayBounds(0, &display_bounds) != 0)
		{
			return false;
		}
		width = display_bounds.w;
		height = display_bounds.h;
	}

	win.width = width;
	win.height = height;

	win.handle = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN | (fullscreen ? SDL_WINDOW_FULLSCREEN : 0));
	win.renderer = SDL_CreateRenderer(win.handle, -1, SDL_RENDERER_ACCELERATED);
	win.texture = SDL_CreateTexture(win.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);

	win.pixels.assign(static_cast<size_t>(width) * height, 0x00000000);

	return true;
}

Input window::poll_events(const Window& win)
{
	Input input;
	SDL_Event event;

	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_QUIT)
			input.quit = true;

		if (event.type == SDL_KEYDOWN)
		{
			if (event.key.repeat == 0)
			{
				if (event.key.keysym.sym == SDLK_ESCAPE) input.quit = true;
				if (event.key.keysym.sym == SDLK_SPACE) input.space_key = true;
				if (event.key.keysym.sym == SDLK_r) input.r_key = true;
				if (event.key.keysym.sym == SDLK_p) input.p_key = true;
			}
			
			if (event.key.keysym.sym == SDLK_LEFT) input.left_key = true;
			if (event.key.keysym.sym == SDLK_RIGHT) input.right_key = true;
		}

		if (event.type == SDL_MOUSEWHEEL)
			input.mouse_wheel += event.wheel.y;
	}

	const uint8_t* keys = SDL_GetKeyboardState(nullptr);
	input.up_key = keys[SDL_SCANCODE_UP];
	input.down_key = keys[SDL_SCANCODE_DOWN];

	int mx, my;
	uint32_t buttons = SDL_GetMouseState(&mx, &my);
	input.mouse_x = mx;
	input.mouse_y = my;
	input.mouse_held = (buttons & SDL_BUTTON_LMASK) != 0;

	return input;
}

void window::present(Window& win)
{
	SDL_UpdateTexture(win.texture, nullptr, win.pixels.data(), win.width * sizeof(uint32_t));

	SDL_RenderClear(win.renderer);
	SDL_RenderCopy(win.renderer, win.texture, nullptr, nullptr);
	SDL_RenderPresent(win.renderer);
}

void window::destroy(Window& win)
{
	SDL_DestroyTexture(win.texture);
	SDL_DestroyRenderer(win.renderer);
	SDL_DestroyWindow(win.handle);
	SDL_Quit();
}