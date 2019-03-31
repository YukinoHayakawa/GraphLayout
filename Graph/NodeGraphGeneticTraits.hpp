#pragma once

#include <vector>
#include <cassert>

namespace usagi::node_graph
{
// fixed ports
// [node0x, node0y, node1x, node1y, ...]
using Genotype = std::vector<float>;
using Population = std::vector<Genotype>;

// genetic operators

// https://en.wikipedia.org/wiki/Crossover_(genetic_algorithm)
void crossover(
    Population &pop,
    const Genotype &a,
    const Genotype &b,
    float p);

// namespace mutation
// {
// void gaussian(
//     Genotype &a,
//
//     )
// }
//
//
// float fitness(const NodeGraph &a)
// {
//
// }

struct GeneticTraits
{
};
}
