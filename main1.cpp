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
		auto [a,b,c,d,e,f] = std::tie(g[0], g[1], g[2], g[3], g[4], g[5]);
		constexpr auto func = [](auto a, auto b) {
			return std::sin(a + b) - a * a - b * b;
		};
		return func(a, b) + func(c, d) + func(e, f);
	}
};


struct Fitness3
{
	using value_type = double;

	value_type operator()(std::vector<value_type> &g)
	{
		auto[a, b] = std::tie(g[0], g[1]);
		constexpr auto func = [](auto a, auto b) {
			return std::sin(a + b) - a * a - b * b;
		};
		return func(a, b);
	}
};

// unbounded. cant converge
struct Fitness4
{
	using value_type = double;

	value_type operator()(std::vector<value_type> &g)
	{
		auto [a,b,c,d,e,f] = std::tie(g[0], g[1], g[2], g[3], g[4], g[5]);
		constexpr auto func = [](auto a, auto b) {
			return std::sin(a + b) - a * a - b * b;
		};
		return func(a, b) * func(c, d) * func(e, f);
	}
};

int main(int argc, char *argv[])
{
	GeneticOptimizer<
		double,
		Fitness2,
		parent::TournamentParentSelection<20>,
		crossover::OnePointCrossover,
		mutation::GaussianMutation<std::vector<double>>,
		// replacement::ReplaceOldest
		replacement::ReplaceWorst
	> optimizer;

	// optimizer.mutation.min = -0.01;
	// optimizer.mutation.max = 0.01;
	optimizer.mutation.std_dev = 0.01;
	// optimizer.mutation_rate = 0.1;

	// init populations
	std::uniform_real_distribution<> dist(
		-10, 10
	);
	optimizer.population.reserve(optimizer.population_size);
	for(int i = 0; i < optimizer.population_size; ++i)
	{
		auto &ind = optimizer.population.emplace_back();
		ind.birthday = 0;
		ind.genotype.insert(ind.genotype.begin(), 6, 0.0);
		// ind.genotype.insert(ind.genotype.begin(), 2, 0.0);
		std::generate(
			ind.genotype.begin(), ind.genotype.end(),
			std::bind(dist, optimizer.rng)
		);
		ind.fitness = optimizer.fitness(ind.genotype);
	}


	for(int i = 1; i < 1000000; ++i)
	{
		optimizer.mutation.std_dev = 1000.0 / i;
		auto best_fitness = std::max_element(
			optimizer.population.begin(),
			optimizer.population.end(),
			[](auto &p0, auto &p1) {
				return p0.fitness < p1.fitness;
			});

		fmt::print("{:>8}: {} ({})\n",
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
