#include <vector>
#include <cstdint>
#include <algorithm>
#include <cmath>
#include "draw.h"
#include "vec2.h"
#include "world.h"
#include "camera.h"
#include "prediction.h"

namespace draw
{
	namespace
	{
		inline Vec2 to_screen(Vec2 world_pos, const Camera& cam)
		{
			return (world_pos - Vec2(cam.x, cam.y)) * cam.zoom;
		}

		inline void plot_pixel(std::vector<uint32_t>* pixels, int width, int height, int x, int y, uint32_t color)
		{
			if (x >= 0 && x < width && y >= 0 && y < height)
			{
				(*pixels)[y * width + x] = color;
			}
		}

		inline void draw_circle(std::vector<uint32_t>* pixels, int width, int height, Vec2 center, int radius, uint32_t color)
		{
			int x0 = std::max(0, static_cast<int>(center.x) - radius);
			int x1 = std::min(width, static_cast<int>(center.x) + radius);
			int y0 = std::max(0, static_cast<int>(center.y) - radius);
			int y1 = std::min(height, static_cast<int>(center.y) + radius);

			int radius_squared = radius * radius;

			for (int y = y0; y < y1; ++y)
			{
				for (int x = x0; x < x1; ++x)
				{
					int dx = x - static_cast<int>(center.x);
					int dy = y - static_cast<int>(center.y);

					if (dx * dx + dy * dy <= radius_squared)
					{
						plot_pixel(pixels, width, height, x, y, color);
					}
				}
			}
		}

		constexpr int CLIP_INSIDE = 0;
		constexpr int CLIP_LEFT = 1;
		constexpr int CLIP_RIGHT = 2;
		constexpr int CLIP_TOP = 4;
		constexpr int CLIP_BOTTOM = 8;

		inline int clip_code(double x, double y, int width, int height)
		{
			int code = CLIP_INSIDE;
			if (x < 0) code |= CLIP_LEFT;
			else if (x > width - 1)  code |= CLIP_RIGHT;
			if (y < 0) code |= CLIP_TOP;
			else if (y > height - 1) code |= CLIP_BOTTOM;
			return code;
		}

		inline bool clip_line(double& x0, double& y0, double& x1, double& y1, int width, int height)
		{
			int c0 = clip_code(x0, y0, width, height);
			int c1 = clip_code(x1, y1, width, height);

			double xmax = width - 1;
			double ymax = height - 1;

			while (true)
			{
				if ((c0 | c1) == 0) return true;
				if ((c0 & c1) != 0) return false;

				int c = c0 ? c0 : c1;
				double x = 0.0, y = 0.0;

				if (c & CLIP_TOP) { x = x0 + (x1 - x0) * (0    - y0) / (y1 - y0); y = 0; }
				else if (c & CLIP_BOTTOM) { x = x0 + (x1 - x0) * (ymax - y0) / (y1 - y0); y = ymax; }
				else if (c & CLIP_RIGHT) { y = y0 + (y1 - y0) * (xmax - x0) / (x1 - x0); x = xmax; }
				else { y = y0 + (y1 - y0) * (0    - x0) / (x1 - x0); x = 0; }

				if (c == c0) { x0 = x; y0 = y; c0 = clip_code(x0, y0, width, height); }
				else { x1 = x; y1 = y; c1 = clip_code(x1, y1, width, height); }
			}
		}

		inline void draw_line(std::vector<uint32_t>* pixels, int width, int height, Vec2 a, Vec2 b, uint32_t color)
		{
			double ax = a.x, ay = a.y, bx = b.x, by = b.y;
			if (!clip_line(ax, ay, bx, by, width, height))
				return;

			int x0 = static_cast<int>(ax);
			int y0 = static_cast<int>(ay);
			int x1 = static_cast<int>(bx);
			int y1 = static_cast<int>(by);

			int dx =  std::abs(x1 - x0);
			int dy = -std::abs(y1 - y0);
			int sx = x0 < x1 ? 1 : -1;
			int sy = y0 < y1 ? 1 : -1;
			int err = dx + dy;

			while (true)
			{
				plot_pixel(pixels, width, height, x0, y0, color);
				if (x0 == x1 && y0 == y1) break;
				int e2 = 2 * err;
				if (e2 >= dy) { err += dy; x0 += sx; }
				if (e2 <= dx) { err += dx; y0 += sy; }
			}
		}

		inline uint32_t dim_color(uint32_t color)
		{
			uint32_t a = color & 0xFF000000;
			uint32_t r = ((color >> 16) & 0xFF) >> 1;
			uint32_t g = ((color >> 8) & 0xFF) >> 1;
			uint32_t b = ( color & 0xFF) >> 1;
			return a | (r << 16) | (g << 8) | b;
		}

		inline int scaled_radius(double radius, const Camera& cam)
		{
			return std::max(1, static_cast<int>(radius * cam.zoom));
		}
	}

	void draw_bodies(std::vector<uint32_t>* pixels, int width, int height, const World& world, const Camera& cam)
	{
		size_t body_count = world.get_body_count();

		for (size_t i = 0; i < body_count; ++i)
		{
			draw_circle(pixels, width, height, to_screen(world.positions[i], cam), scaled_radius(world.radii[i], cam), world.colors[i]);
		}
	}

	void draw_paths(std::vector<uint32_t>* pixels, int width, int height, const Prediction& prediction, const World& world, const Camera& cam)
	{
		size_t path_count = prediction.paths.size();

		for (size_t i = 0; i < path_count; ++i)
		{
			uint32_t color = dim_color(world.colors[i]);
			const std::vector<Vec2>& path = prediction.paths[i];
			size_t path_length = path.size();

			for (size_t k = 1; k < path_length; ++k)
			{
				draw_line(pixels, width, height, to_screen(path[k - 1], cam), to_screen(path[k], cam), color);
			}
		}

		for (const PredictedCollision& collision : prediction.collisions)
		{
			uint32_t color = dim_color(world.colors[collision.body]);
			draw_circle(pixels, width, height, to_screen(collision.position, cam), scaled_radius(world.radii[collision.body], cam), color);
		}
	}
}