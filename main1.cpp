#include <GraphLayout/Genetic/GeneticOptimizer.hpp>
#include <GraphLayout/Genetic/ParentSelection.hpp>
#include <GraphLayout/Genetic/Crossover.hpp>
#include <GraphLayout/Genetic/Mutation.hpp>
#include <GraphLayout/Genetic/Replacement.hpp>
#include <fmt/printf.h>

using namespace usagi;
using namespace genetic;

struct Fitness
{
	using value_type = double;

	value_type operator()(std::vector<value_type> &g)
	{
		constexpr value_type coeff[] = { 4.0, -2.0, 7.0, 5.0, 11.0, 1.0 };
		value_type fitness = 0;
		for(std::size_t i = 0; i < 6; ++i)
		{
			fitness += g[i] * coeff[i];
		}
		return fitness;
	}
};

struct Fitness2
{
	using value_type = double;

	value_type operator()(std::vector<value_type> &g)
	{
		auto [x, y] = std::tie(g[0], g[1]);
		return std::sin(x + y) - x * x - y * y;
	}
};

int main(int argc, char *argv[])
{
	GeneticOptimizer<
		double,
		Fitness2,
		parent::TournamentParentSelection<5>,
		crossover::OnePointCrossover,
		mutation::GaussianMutation<std::vector<double>>,
		replacement::RemoveOldestReplacementPolicy
	> optimizer;

	// init populations
	std::uniform_real_distribution<> dist(
		-10, 10
	);
	optimizer.population.reserve(optimizer.population_size);
	for(int i = 0; i < optimizer.population_size; ++i)
	{
		auto &ind = optimizer.population.emplace_back();
		ind.birthday = 0;
		// ind.genotype.insert(ind.genotype.begin(), 6, 0.0);
		ind.genotype.insert(ind.genotype.begin(), 2, 0.0);
		std::generate(
			ind.genotype.begin(), ind.genotype.end(),
			std::bind(dist, optimizer.rng)
		);
		ind.fitness = optimizer.fitness(ind.genotype);
	}


	for(int i = 0; i < 1000; ++i)
	{
		auto best_fitness = std::max_element(
			optimizer.population.begin(),
			optimizer.population.end(),
			[](auto &p0, auto &p1) {
				return p0.fitness < p1.fitness;
			});

		fmt::print("{:>4}: {} ({})\n",
			optimizer.year,
			best_fitness->fitness,
			fmt::join(
				best_fitness->genotype.begin(),
				best_fitness->genotype.end(),
				", "
			));

		optimizer.step();
	}

	return 0;
}
