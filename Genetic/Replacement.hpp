#pragma once

#include <cassert>

namespace usagi::genetic::replacement
{
struct RemoveOldestReplacementPolicy
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
}
