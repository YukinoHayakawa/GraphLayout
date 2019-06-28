#pragma once

#include <random>
#include <algorithm>
#include <cassert>
#include <functional>
#include <array>

// https://www.tutorialspoint.com/genetic_algorithms/genetic_algorithms_parent_selection.htm
namespace usagi::genetic::parent
{
/**
 * \brief K-tournament parent selection.
 * If TournamentSize == 1 it degenerates to random selection.
 * todo currently only 2 candidates are used. other cases are not supported by GeneticOptimizer yet
 * Fig.5.3
 * \tparam TournamentSize How many members to choose from during each selection.
 * \tparam CandidateSize How many parents to be chosen.
 */
template <
    std::size_t TournamentSize = 5,
    std::size_t CandidateSize = 2
>
struct TournamentParentSelection
{
    static_assert(TournamentSize > 0);
    static_assert(CandidateSize > 0);

    template <typename Optimizer>
    auto operator()(Optimizer &o)
    {
        // ensure that we have many enough members to choose from.
        assert(o.population.size() >= TournamentSize);

        std::array<std::size_t, CandidateSize> candidates;
        std::size_t positions[TournamentSize];

        std::uniform_int_distribution<std::size_t> pos_dist(
            0, o.population.size() - 1
        );
        for(std::size_t i = 0; i < CandidateSize; ++i)
        {
            // randomly choose K individuals
            std::generate(
                positions, positions + TournamentSize,
                // remember to use ref to avoid copying!!!
                std::bind(pos_dist, std::ref(o.rng))
            );
            // find the one with max fitness
            // todo stochastic selection to allow choosing least fit members
            std::size_t *best = std::max_element(
                positions, positions + TournamentSize,
                [&](std::size_t p0, std::size_t p1) {
                    return o.population[p0].fitness < o.population[p1].fitness;
                });
            candidates[i] = *best;
        }

        return candidates;
    }

    // post offspring/fitness event handlers...
};

}
