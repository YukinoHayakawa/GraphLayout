#pragma once

#include <vector>
#include <random>

namespace usagi::genetic
{
template <typename Genotype, typename Fitness>
struct Individual
{
	// generator provides genotype
	Genotype genotype;

	// this algorithm will set these values
	Fitness fitness = 0;
	std::size_t birthday = 0;
	std::size_t family = 0;
	std::size_t generation = 0;

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
	PopulationGenerator generator;

	std::size_t population_size = 100;
	std::size_t year = 0;
	double crossover_rate = 0.85;
	double mutation_rate = 0.2;

	Individual *best = nullptr;
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
		auto &dead0 = replacement(*this, static_cast<IndividualT*>(nullptr));
		auto &dead1 = replacement(*this, &dead0);
		return std::forward_as_tuple(dead0, dead1);
	}

	void newIndividual(Individual &individual)
	{
		individual.birthday = year;
		individual.fitness = fitness(individual);
		// track best individual (elite)
		// todo: ordered fitness using priority queue
		if(best == nullptr || individual.fitness > best->fitness)
			best = &individual;
	}

	void initializePopulation(std::size_t size)
	{
		year = 0;
		best = nullptr;
		population.clear();
		population.reserve(size);
		fitness_history.clear();
		for(std::size_t i = 0; i < size; ++i)
		{
			population.push_back(generator(*this));
			auto &back = population.back();
			back.family = i;
			newIndividual(back);
		}
	}

	auto step()
	{
		// track best fitness history
		if(fitness_history.size() < fitness_history_max && best)
		{
			if(year % fitness_history_interval == 0)
				fitness_history.push_back(best->fitness);
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
		std::uniform_real_distribution<> dm(0, mutation_rate);
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
