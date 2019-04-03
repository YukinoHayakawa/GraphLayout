#pragma once

#include <random>
#include <algorithm>
#include <cassert>
#include <functional>

// https://www.tutorialspoint.com/genetic_algorithms/genetic_algorithms_parent_selection.htm
namespace usagi::genetic::parent
{
// K-tournament parent selection. if k == 1 it becomes random selection.
template <std::size_t K>
struct TournamentParentSelection
{
    static_assert(K > 0);

    template <typename Optimizer>
    auto & operator()(Optimizer &o)
    {
        assert(o.population.size() >= K);
        std::uniform_int_distribution<std::size_t> pos_dist(
            0, o.population.size() - 1
        );

		// randomly choose K individuals
        std::size_t positions[K];
        std::generate(positions, positions + K, std::bind(pos_dist, o.rng));

        // find the one with max fitness
        std::size_t *best = std::max_element(positions, positions + K,
			[&](std::size_t p0, std::size_t p1) {
				return o.population[p0].fitness < o.population[p1].fitness;
			});

        return o.population[*best];
    }

	// post offspring/fitness event handlers...
};

}
