﻿#pragma once

#include <random>

// https://en.wikipedia.org/wiki/Mutation_(genetic_algorithm)
// Mutation occurs during evolution according to a user-definable mutation probability. This probability should be set low. If it is set too high, the search will turn into a primitive random search.
namespace usagi::genetic::mutation
{
// add a Gaussian distributed number to one gene

template <
    typename Genotype
>
struct GaussianMutation
{
    using ValueT = typename Genotype::value_type;
    ValueT mean = 0;
    ValueT std_dev = 1;

    template <typename RNG>
    void operator()(Genotype &a, RNG &rng)
    {
        assert(a.size() > 0);
        std::uniform_int_distribution<typename Genotype::size_type> gene_dist(
            0, a.size() - 1
        );
        auto gene_pos = gene_dist(rng);

        std::normal_distribution<ValueT> num_dist(mean, std_dev);
        a[gene_pos] += num_dist(rng);
    }
};
}