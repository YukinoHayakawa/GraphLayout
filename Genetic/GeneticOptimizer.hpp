#pragma once

#include <vector>
#include <random>

#include "BinaryHeap.hpp"
#include <Usagi/Core/Logging.hpp>

namespace usagi::genetic
{
template <typename Genotype>
struct Individual
{
	std::uint32_t birthday = 0;
	std::uint32_t generation = 0;

	std::uint32_t family = 0;
	std::uint32_t index = -1;
	std::size_t queue_index = -1;

	// generator provides genotype
	Genotype genotype;

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
	typename Individual = Individual<Genotype>,
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
	using FitnessT = typename FitnessFunctionT::FitnessT;

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
			return b->fitness < a->fitness;
		}
	};

	BinaryHeap<Individual*, FitnessComparator> best;

	// fitness history

	struct FitnessHistory
	{
		FitnessT fitness;
		std::uint32_t year;
	};
	std::vector<FitnessHistory> fitness_history;
	FitnessT last_best_fitness = -10e10f;
	FitnessT significant_improvement_threshold = 50;
	std::uint32_t significant_improvement_period = 20'000;

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
		last_best_fitness = -10e10f;
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

	bool stopCondition()
	{
		// don't stop if we even didn't start
		if(fitness_history.empty()) return false;
		auto i = fitness_history.rbegin();
		for(; i != fitness_history.rend(); ++i)
		{
			// search for the other end of evaluation interval
			if(year - i->year >= significant_improvement_period) break;
		}
		// if the search went through the whole history, take the first item.
		if(i == fitness_history.rend()) --i;
		// continue if we haven't reach iteration limit
		if(year - i->year < significant_improvement_period)
			return false;
		auto improvement = best.top()->fitness - i->fitness;
		// if no significant improvement within certain evaluation period, stop
		return improvement < significant_improvement_threshold;
	}

	auto step()
	{
		// track best fitness history
		assert(!best.empty());
		if(last_best_fitness < best.top()->fitness)
		{
			fitness_history.push_back({ best.top()->fitness, year });
			last_best_fitness = best.top()->fitness;
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
