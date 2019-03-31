#pragma once

#include <vector>
#include <random>

#include <GraphLayout/Graph/NodeGraphGeneticTraits.hpp>
#include <Usagi/Core/Math.hpp>
#include <GraphLayout/Graph/NodeGraph.hpp>
#include <Usagi/Core/Logging.hpp>

namespace usagi
{
// template <
//     typename Genetype,
//     typename GenetypeTraits,
//     typename T
// >
// https://www.tutorialspoint.com/genetic_algorithms/index.htm
struct GeneticOptimizer
{
    std::random_device rd;
    std::mt19937 gen { rd() };

    struct Individual
    {
        node_graph::Genotype genotype;
        float fitness = NAN;
        NodeGraph phenotype;

        void mapPhenotype(const NodeGraph &prototype)
        {
            phenotype = prototype;
            for(int i = 0; i < phenotype.nodes.size(); ++i)
            {
                phenotype.nodes[i].center = {
                    genotype[i * 2], genotype[i * 2 + 1]
                };
            }
        }

        void calcFitness()
        {
            float f = 0;
            for(auto &&l : phenotype.links)
            {
                auto &n0 = phenotype.nodes[l.node0];
                auto &n1 = phenotype.nodes[l.node1];
                f += (n1.center - n0.center).dot(Vector2f::UnitX());
            }
            fitness = f;
        }
    };

    AlignedBox2i bound;
    std::vector<Individual> population;
    node_graph::Population pop, pop2;
    int init_pop_size = 100;
    NodeGraph graph;
    float pc = 0.2f;
    float pm = 0.05f;


    void initPopulation()
    {
        for(int i = 0; i < init_pop_size; ++i)
        {
            node_graph::Genotype g;
            auto len = graph.nodes.size() * 2;
            g.reserve(len);
            for(int j = 0; j < len; ++j)
            {
                std::uniform_real_distribution<float> dis(10, 1000);
                g.push_back(dis(gen));
            }
            Individual ind;
            ind.genotype = std::move(g);
            ind.mapPhenotype(graph);
            ind.calcFitness();

            LOG(info, "Fitness: {}", ind.fitness);

            population.push_back(std::move(ind));
        }
    }

    void copy()
    {
        // select (1-pc) from old population
    }

    void optimize()
    {
        initPopulation();
        /*do
        {
            for(auto &&p : population)
            {
                copy();
                chooseParents();
                crossover();
                mutate();
            }
        }
        while(!terminateCondition());*/
    }
};
}
