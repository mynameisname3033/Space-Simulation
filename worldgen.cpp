#include "worldgen.h"

#include <optional>
#include <cstdint>
#include <random>
#include "world.h"
#include "engine.h"
#include "vec2.h"

namespace worldgen
{
	namespace
	{
		uint64_t s[4] = { 0x123456789, 0xABCDEF12, 0x3456789A, 0xBCDEF123 };

		inline uint64_t next_uint64()
		{
			const uint64_t result = ((s[1] * 5) << 7 | (s[1] * 5) >> 57) * 9;
			const uint64_t t = s[1] << 17;
			s[2] ^= s[0]; s[3] ^= s[1]; s[1] ^= s[2]; s[0] ^= s[3];
			s[2] ^= t;
			s[3] = (s[3] << 45) | (s[3] >> 19);
			return result;
		}

		inline double next_double(double min, double max)
		{
			double random = (next_uint64() >> 11) * 0x1.0p-53;
			return min + random * (max - min);
		}

		inline World generate_random_world(size_t body_count, Vec2 center, Vec2 position_bounds, Vec2 velocity_bounds, double min_mass, double max_mass, double min_radius, double max_radius)
		{
			World world;

			for (size_t i = 0; i < body_count; ++i)
			{
				Vec2 position = center + Vec2(next_double(-position_bounds.x, position_bounds.x), next_double(-position_bounds.y, position_bounds.y));
				Vec2 velocity = Vec2(next_double(-velocity_bounds.x, velocity_bounds.x), next_double(-velocity_bounds.y, velocity_bounds.y));
				double mass = next_double(min_mass, max_mass);
				double radius = next_double(min_radius, max_radius);
				world.add_body(position, velocity, mass, radius, 0xFFFFFFFF);
			}

			return world;
		}

		bool is_stable(World world, int steps, double dt)
		{
			size_t body_count = world.get_body_count();

			for (int s = 0; s < steps; ++s)
			{
				engine::update(world, dt);
				for (size_t i = 0; i < body_count; ++i)
				{
					if (world.stopped[i])
						return false;
				}
			}

			return engine::is_bound(world);
		}
	}

	void init()
	{
		std::random_device rd;

		s[0] = (static_cast<uint64_t>(rd()) << 32) | rd();
		s[1] = (static_cast<uint64_t>(rd()) << 32) | rd();
		s[2] = (static_cast<uint64_t>(rd()) << 32) | rd();
		s[3] = (static_cast<uint64_t>(rd()) << 32) | rd();

		if ((s[0] | s[1] | s[2] | s[3]) == 0)
		{
			s[0] = 0x123456789ULL;
			s[1] = 0xABCDEF12ULL;
			s[2] = 0x3456789AULL;
			s[3] = 0xBCDEF123ULL;
		}
	}

	std::optional<World> generate_interesting_stable_world(size_t body_count, Vec2 center, Vec2 position_bounds, Vec2 velocity_bounds, double min_mass, double max_mass, double min_radius, double max_radius, int steps, double dt, int max_attempts)
	{
		for (int i = 0; i < max_attempts; ++i)
		{
			World candidate = generate_random_world(body_count, center, position_bounds, velocity_bounds, min_mass, max_mass, min_radius, max_radius);
			if (is_stable(candidate, steps, dt))
				return candidate;
		}

		return std::nullopt;
	}
}