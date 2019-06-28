#pragma once

#include <random>
#include <cassert>
#include <algorithm>

namespace usagi::genetic::crossover
{
// suppose the genotype is a linear container like vector/string
struct OnePointCrossover
{
    template <typename Genotype, typename RNG>
    void operator()(Genotype &a, Genotype &b, RNG &rng)
    {
        assert(a.size() == b.size());
        // since int distribution is closed range, it's possible that the
        // two genotypes are just swapped.
        std::uniform_int_distribution<typename Genotype::size_type> dist(
            0, a.size()
        );
        auto split_point = dist(rng);
        // aaaaaaaaaaaaaaaa --> bbbbbbaaaaaaaaaa
        // bbbbbbbbbbbbbbbb --> aaaaaabbbbbbbbbb
        std::swap_ranges(a.begin(), a.begin() + split_point, b.begin());
    }
};

struct WholeArithmeticRecombination
{
    template <typename Genotype, typename RNG>
    void operator()(Genotype &a, Genotype &b, RNG &)
    {
        assert(a.size() == b.size());
        for(auto i = a.begin(), j = b.begin(); i != a.end(); ++i, ++j)
        {
            *i = *j = 0.5f * (*i + *j);
        }
    }
};
}
