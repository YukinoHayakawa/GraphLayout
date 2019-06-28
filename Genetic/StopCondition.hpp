#pragma once

#include <cstdint>

namespace usagi::genetic::stop
{
template <
    typename Fitness
>
struct SolutionConvergedStopCondition
{
    Fitness significant_improvement_threshold = 50;
    std::uint32_t significant_improvement_period = 20'000;

    template <typename Optimizer>
    bool operator()(Optimizer &o)
    {
        // don't stop if we even didn't start
        if(o.fitness_history.empty()) return false;
        auto i = o.fitness_history.rbegin();
        for(; i != o.fitness_history.rend(); ++i)
        {
            // search for the other end of evaluation interval
            if(o.year - i->year >= significant_improvement_period) break;
        }
        // if the search went through the whole history, take the first item.
        if(i == o.fitness_history.rend()) --i;
        // continue if we haven't reach iteration limit
        if(o.year - i->year < significant_improvement_period)
            return false;
        auto improvement = o.best.top()->fitness - i->fitness;
        // if no significant improvement within certain evaluation period, stop
        return improvement < significant_improvement_threshold;
    }
};
}
