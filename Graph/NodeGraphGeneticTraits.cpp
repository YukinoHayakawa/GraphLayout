#include "NodeGraphGeneticTraits.hpp"

void usagi::node_graph::crossover(
    Population &pop,
    const Genotype &a,
    const Genotype &b,
    float p)
{
    assert(a.size() == b.size());

    Genotype c, d;
    c.reserve(a.size());
    d.reserve(a.size());

    Genotype::size_type split = (size_t) (p * a.size());

    c.insert(c.end(), a.begin(), a.begin() + split);
    c.insert(c.end(), b.begin() + split, b.end());
    assert(c.size() == a.size());

    d.insert(d.end(), b.begin(), b.begin() + split);
    d.insert(d.end(), a.begin() + split, a.end());
    assert(d.size() == a.size());

    pop.push_back(std::move(c));
    pop.push_back(std::move(d));
}
