#pragma once

#include <vector>
#include <random>

#include "BinaryHeap.hpp"

namespace usagi::genetic
{
template <typename Genotype, typename Fitness>
struct Individual
{
	std::uint32_t birthday = 0;
	std::uint32_t generation = 0;

	std::uint32_t family = 0;
	std::uint32_t index = -1;
	std::size_t queue_index = -1;

	// generator provides genotype
	Genotype genotype;

	// this algorithm will set these values
	Fitness fitness = 0;

	// todo use trait functions -> genotype() -> auto &
};

// https://www.tutorialspoint.com/genetic_algorithms/index.htm
template <
	typename Gene,
	typename FitnessFunction,
	typename ParentSelection,
	typename CrossoverOperator,
	typename MutationOperator,
	typename ReplacementStrategy,
	typename PopulationGenerator,
	typename Genotype = std::vector<Gene>,
	typename Individual = Individual<
		Genotype,
		typename FitnessFunction::value_type
	>,
	typename Population = std::vector<Individual>,
	typename Rng = std::mt19937
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
	using PopulationGeneratorT = PopulationGenerator;
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
	PopulationGeneratorT generator;

	std::size_t population_size = 100;
	std::uint32_t year = 0;
	double crossover_rate = 0.85;

	// elite tracking

	/**
	 * \brief Uses > to turn the heap into a max heap.
	 */
	struct FitnessComparator
	{
		bool operator()(Individual *a, Individual *b) const
		{
			return a->fitness > b->fitness;
		}
	};

	BinaryHeap<Individual*, FitnessComparator> best;

	// fitness history

	std::vector<float> fitness_history;
	std::size_t fitness_history_interval = 500;
	std::size_t fitness_history_max = 1000;

	auto chooseParents()
	{
		auto [i0, i1] = parent_selection(*this);
		return std::forward_as_tuple(population[i0], population[i1]);
	}

	auto chooseReplacedIndividuals()
	{
		auto [i0, i1] = replacement(*this);
		return std::forward_as_tuple(population[i0], population[i1]);
	}

	void newIndividual(Individual &individual)
	{
		individual.birthday = year;
		reevaluateIndividual(individual);
	}

	void initializePopulation(const std::size_t size)
	{
		assert(size < std::numeric_limits<std::uint32_t>::max());
		year = 0;
		best.clear();
		best.reserve(size);
		population.clear();
		population.reserve(size);
		fitness_history.clear();
		for(std::size_t i = 0; i < size; ++i)
		{
			population.push_back(generator(*this));
			auto &back = population.back();
			back.family = static_cast<std::uint32_t>(i);
			back.index = static_cast<std::uint32_t>(i);
			newIndividual(back);
		}
	}

	void reevaluateIndividual(Individual &individual)
	{
		individual.fitness = fitness(individual);

		// track best individual (elite)

		// first iteration
		if(individual.queue_index == -1)
			best.insert(&individual);
		// later iterations
		else
			best.modifyKey(individual.queue_index);
	}

	auto step()
	{
		// track best fitness history
		if(fitness_history.size() < fitness_history_max)
		{
			assert(!best.empty());
			if(year % fitness_history_interval == 0)
				fitness_history.push_back(best.top()->fitness);
		}

		// increment time
		++year;

		// choose parents whose gene will be used to produce offspring
		auto [p0, p1] = chooseParents();
		// choose dead individuals and replace them with offspring
		auto [o0, o1] = chooseReplacedIndividuals();

		// copy genes
		o0.genotype = p0.genotype;
		o1.genotype = p1.genotype;
		// set family
		o0.family = p0.family;
		o0.generation = p0.generation + 1;
		o1.family = p1.family;
		o1.generation = p1.generation + 1;

		std::uniform_real_distribution<> dc(0, crossover_rate);
		// crossover
		if(dc(rng) < crossover_rate)
			crossover(o0.genotype, o1.genotype, rng);
		// mutation
		mutation(o0.genotype, rng);
		mutation(o1.genotype, rng);

		// evaluate fitness of offspring
		newIndividual(o0);
		newIndividual(o1);
	}
};
}
