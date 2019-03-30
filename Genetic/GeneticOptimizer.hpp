#pragma once
#include <vector>

namespace usagi
{
template <
    typename Genetype,
    typename GenetypeTraits,
    typename
>
struct GeneticOptimizer
{
    std::vector<Genetype> population;

    void step()
    {
    }

    void optimize()
    {
        initPopulation();
        do
        {
            for(auto &&p : population)
            {
                chooseParents();
                crossover();
                mutate();

            }
        }
        while(!terminateCondition());
    }
};
}
