#include <vector>
#include "engine.h"
#include "world.h"
#include "vec2.h"
#include "prediction.h"

namespace engine
{
	namespace
	{
		constexpr double g = 0.5;

		inline Vec2 calculate_acceleration(World& world, size_t i)
		{
			size_t body_count = world.get_body_count();

			Vec2 position1 = world.positions[i];
			double mass1 = world.masses[i];
			double radius1 = world.radii[i];

			Vec2 total_force;

			for (size_t j = 0; j < body_count; ++j)
			{
				if (i == j || world.stopped[j])
					continue;

				Vec2 position2 = world.positions[j];
				double mass2 = world.masses[j];
				double radius2 = world.radii[j];

				Vec2 vector = position2 - position1;
				double length = vector.length();

				if (length <= (radius1 + radius2))
				{
					world.stopped[i] = true;
					world.stopped[j] = true;
					continue;
				}

				Vec2 dir = vector / length;
				total_force += dir * g * mass1 * mass2 / (length * length);
			}

			return total_force / mass1;
		}
	}

	void update(World& world, double dt)
	{
		size_t body_count = world.get_body_count();

		for (size_t i = 0; i < body_count; ++i)
		{
			if (world.stopped[i])
				continue;

			world.accelerations[i] = calculate_acceleration(world, i);
		}

		for (size_t i = 0; i < body_count; ++i)
		{
			if (world.stopped[i])
				continue;

			world.positions[i] += world.velocities[i] * dt + world.accelerations[i] * 0.5 * dt * dt;
		}

		for (size_t i = 0; i < body_count; ++i)
		{
			if (world.stopped[i])
				continue;

			Vec2 new_acceleration = calculate_acceleration(world, i);
			world.velocities[i] += (world.accelerations[i] + new_acceleration) * 0.5 * dt;
		}
	}

	Prediction predict(World world, int steps, double dt)
	{
		size_t body_count = world.get_body_count();

		Prediction result;
		result.paths.resize(body_count);

		std::vector<bool> was_stopped(body_count);
		for (size_t i = 0; i < body_count; ++i)
			was_stopped[i] = world.stopped[i];

		for (int s = 0; s < steps; ++s)
		{
			update(world, dt);

			for (size_t i = 0; i < body_count; ++i)
			{
				if (world.stopped[i])
				{
					if (!was_stopped[i])
					{
						result.collisions.push_back({ i, world.positions[i] });
						was_stopped[i] = true;
					}
					continue; 
				}

				result.paths[i].push_back(world.positions[i]);
			}
		}

		return result;
	}

	bool is_bound(const World& world)
	{
		size_t body_count = world.get_body_count();

		if (body_count < 2)
			return true;

		Vec2 com_pos, com_vel;
		double total_mass = 0.0;
		for (size_t i = 0; i < body_count; ++i)
		{
			com_pos += world.positions[i] * world.masses[i];
			com_vel += world.velocities[i] * world.masses[i];
			total_mass += world.masses[i];
		}
		com_pos = com_pos / total_mass;
		com_vel = com_vel / total_mass;

		for (size_t i = 0; i < body_count; ++i)
		{
			Vec2 rel_pos = world.positions[i] - com_pos;
			Vec2 rel_vel = world.velocities[i] - com_vel;

			double r = rel_pos.length();
			if (r == 0.0)
				continue;

			double M = total_mass - world.masses[i];
			double kinetic = 0.5 * rel_vel.length_squared();
			double potential = g * M / r;
			double energy = kinetic - potential;

			if (energy >= 0.0)
				return false;
		}

		return true;
	}
}