#pragma once

#include <random>

// https://en.wikipedia.org/wiki/Mutation_(genetic_algorithm)
// Mutation occurs during evolution according to a user-definable mutation probability. This probability should be set low. If it is set too high, the search will turn into a primitive random search.
namespace usagi::genetic::mutation
{
// add a Gaussian distributed number to one
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

template <
    typename Genotype
>
struct UniformIntMutation
{
    using ValueT = typename Genotype::value_type;
    ValueT min = -1;
    ValueT max = 1;

    template <typename RNG>
    void operator()(Genotype &a, RNG &rng)
    {
        assert(a.size() > 0);
        std::uniform_int_distribution<typename Genotype::size_type> gene_dist(
            0, a.size() - 1
        );
        auto gene_pos = gene_dist(rng);

        std::uniform_real_distribution<ValueT> num_dist(min, max);
        a[gene_pos] += num_dist(rng);
    }
};
template <
    typename Genotype
>
struct UniformRealMutation
{
    using ValueT = typename Genotype::value_type;
    std::uniform_real_distribution<ValueT> domain { 0, 1 };
    float mutation_rate = 0.1f;

    template <typename RNG>
    void operator()(Genotype &a, RNG &rng)
    {
        std::uniform_real_distribution<float> mutation_dist { 0, 1 };
        for(auto &&g : a)
        {
            if(mutation_dist(rng) < mutation_rate)
                g = domain(rng);
        }
    }
};
}
