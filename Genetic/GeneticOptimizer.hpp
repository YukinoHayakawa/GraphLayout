#pragma once

#include <vector>
#include <random>

#include <Usagi/Core/Math.hpp>

namespace usagi
{
template <typename Genotype, typename Fitness>
struct Individual
{
	Genotype genotype;
	Fitness fitness;
	std::size_t birthday;
};

// https://www.tutorialspoint.com/genetic_algorithms/index.htm
template <
	typename Gene,
	typename FitnessFunction,
	typename ParentSelection,
	typename CrossoverOperator,
	typename MutationOperator,
	typename ReplacementStrategy,
	typename Rng = std::mt19937,
	typename Genotype = std::vector<Gene>,
	typename Individual = Individual<
		Genotype,
		typename FitnessFunction::value_type
	>,
	typename Population = std::vector<Individual>
>
struct GeneticOptimizer
{
	using GeneT = Gene;
	using FitnessFunctionT = FitnessFunction;
	using ParentSelectionT = ParentSelection;
	using CrossoverOperatorT = CrossoverOperator;
	using MutationOperatorT = MutationOperator;
	using ReplacementStrategyT = ReplacementStrategy;
	using RngT = Rng;
	using GenotypeT = Genotype;
	using IndividualT = Individual;
	using PopulationT = Population;

	RngT rng;
	FitnessFunctionT fitness;
	ParentSelectionT parent_selection;
	CrossoverOperatorT crossover;
	MutationOperatorT mutation;
	ReplacementStrategyT replacement;
	PopulationT population;

	std::size_t population_size = 100;
	std::size_t year = 0;
	double crossover_rate = 0.7;
	double mutation_rate = 0.1;

	auto chooseParents()
	{
		return std::forward_as_tuple(
			parent_selection(*this),
			parent_selection(*this)
		);
	}

	auto chooseReplacedIndividuals()
	{
		auto &dead0 = replacement(*this, static_cast<IndividualT*>(nullptr));
		auto &dead1 = replacement(*this, &dead0);
		return std::forward_as_tuple(dead0, dead1);
	}

	auto step()
	{
		// increment time
		++year;

		// choose parents whose gene will be used to produce offspring
		auto [p0, p1] = chooseParents();
		// choose dead individuals and replace them with offspring
		auto [o0, o1] = chooseReplacedIndividuals();

		// copy genes
		o0.genotype = p0.genotype;
		o1.genotype = p1.genotype;
		// reset age
		o0.birthday = o1.birthday = year;
		std::uniform_real_distribution<> dc(0, crossover_rate);
		std::uniform_real_distribution<> dm(0, mutation_rate);

		// crossover
		if(dc(rng) < crossover_rate)
			crossover(o0.genotype, o1.genotype, rng);
		// mutation
		if(dm(rng) < mutation_rate)
			mutation(o0.genotype, rng);
		if(dm(rng) < mutation_rate)
			mutation(o1.genotype, rng);

		// evaluate fitness of offspring
		o0.fitness = fitness(o0.genotype);
		o1.fitness = fitness(o1.genotype);
	}

	// void initializePopulation()
	// {
	// 	for(auto i = 0; i < population_size; ++i)
	// 	{
	// 	}
	// }
	//
	// void optimize()
	// {
	// 	initializePopulation();
	// 	do
	// 	{
	// 		step();
	// 	}
	// 	while(!terminationCondition());
	// }

	/*struct Individual
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
	node_graph::Population pop, pop2;
	int init_pop_size = 100;
	NodeGraph graph;
	float pc = 0.2f;
	float pm = 0.05f;

	GeneticOptimizer(Element *parent, std::string name)
	    : Element(parent, std::move(name))
	{
	}

	void iteration()
	{
	    auto [i0, i1] = selectParents();
	    [d0, d1] = selectDead();
	    reproduce(i0, i1);

	}

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
	    while(!terminateCondition());#1#
	}*/
};
}
