﻿#pragma once

#include <cassert>
#include <queue>
#include <random>
#include <algorithm>
#include <atomic>

namespace usagi::genetic::replacement
{
template <
	std::size_t TournamentSize = 10,
	std::size_t ReplacementSize = 2
>
struct RoundRobinTournamentReplacement
{
	struct Tournament
	{
		std::uint16_t index = 0;
		std::atomic<std::uint16_t> wins = 0;

		friend bool operator<(const Tournament &a, const Tournament &b)
		{
			return a.wins < b.wins;
		}
	};

	std::vector<Tournament> results;

	template <typename Optimizer>
	auto operator()(Optimizer &o)
	{
		assert(o.population.size() >= ReplacementSize);
		assert(o.population.size() < 0xFFFF);

		results.resize(o.population.size());

		std::size_t opponents[TournamentSize];
		std::uniform_int_distribution<std::size_t> pos_dist(
			0, o.population.size() - 1
		);

		// for each individual, find some random competitors
		for(std::size_t i = 0; i < o.population.size(); ++i)
		{
			results[i].index = static_cast<std::uint16_t>(i);
			results[i].wins = 0;
			// randomly choose opponents
			std::generate(
				opponents, opponents + TournamentSize,
				std::bind(pos_dist, std::ref(o.rng))
			);
			for(std::size_t j = 0; j < TournamentSize; ++j)
			{
				if(o.population[i].fitness > o.population[j].fitness)
					++results[i].wins;
			}
		}

		// find individuals with low wins and let them be replaced
		std::partial_sort(
			results.begin(), results.begin() + ReplacementSize,
			results.end()
		);
		std::array<std::size_t, ReplacementSize> replacement;
		for(std::size_t i = 0; i < ReplacementSize; ++i)
		{
			replacement[i] = results[i].index;
		}
		return replacement;
	}
};

struct ReplaceOldest
{
	template <
		typename Optimizer,
		typename Individual = typename Optimizer::IndividualT
	>
	auto operator()(Optimizer &o, Individual *other) -> Individual &
	{
		assert(o.population.size() > 1);
		Individual *oldest = nullptr;
		// todo priority queue
		for(auto &&individual : o.population)
		{
			if((oldest == nullptr || individual.birthday < oldest->birthday)
				&& &individual != other)
				oldest = &individual;
		}
		return *oldest;
	}
};

struct ReplaceWorst
{
	template <
		typename Optimizer,
		typename Individual = typename Optimizer::IndividualT
	>
	auto operator()(Optimizer &o, Individual *other) -> Individual &
	{
		assert(o.population.size() > 1);
		Individual *worst = nullptr;
		// todo priority queue
		for(auto &&individual : o.population)
		{
			if((worst == nullptr || individual.fitness < worst->fitness)
				&& &individual != other)
				worst = &individual;
		}
		return *worst;
	}
};
}
