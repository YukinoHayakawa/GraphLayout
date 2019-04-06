#pragma once

#include <Usagi/Core/Element.hpp>
#include <Usagi/Extension/ImGui/ImGuiComponent.hpp>
#include <GraphLayout/Graph/NodeGraph.hpp>
#include <Usagi/Extension/Nuklear/NuklearComponent.hpp>
#include <GraphLayout/Genetic/GeneticOptimizer.hpp>
#include <GraphLayout/Genetic/ParentSelection.hpp>
#include <GraphLayout/Genetic/Crossover.hpp>
#include <GraphLayout/Genetic/Mutation.hpp>
#include <GraphLayout/Genetic/Replacement.hpp>

namespace usagi
{
using PortNode = Node<std::function<Vector2f()>>;
using PortGraph = NodeGraph<PortNode>;

struct PortGraphIndividual : genetic::Individual<std::vector<float>, float>
{
	PortGraph *graph = nullptr;

	Vector2f & position(std::size_t node_index)
	{
		return reinterpret_cast<Vector2f*>(genotype.data())[node_index];
	}
};

struct PortGraphFitness
{
	using value_type = float;
	float w_dir = 1;
	float w_length = 1;

	value_type operator()(PortGraphIndividual &g);
};

struct PortGraphPopulationGenerator
{
	PortGraph *linked_graph = nullptr;
	std::size_t size = 0;
	std::uniform_real_distribution<float> dist { 0, 1000 };

	template <typename Optimizer>
	PortGraphIndividual operator()(Optimizer &o)
	{
		PortGraphIndividual individual;
		individual.genotype.resize(size);
		std::generate(
			individual.genotype.begin(), individual.genotype.end(),
			// use ref for rng to prevent being copied
			std::bind(dist, std::ref(o.rng))
		);
		individual.graph = linked_graph;
		return std::move(individual);
	}
};

class PortGraphObserver
	: public Element
	, public NuklearComponent
	, public ImGuiComponent
{
	PortGraph mGraph;

	using Gene = float;
	using Genotype = std::vector<float>;

	genetic::GeneticOptimizer<
		Gene,
		PortGraphFitness,
		genetic::parent::TournamentParentSelection<3>,
		genetic::crossover::OnePointCrossover,
		genetic::mutation::GaussianMutation<Genotype>,
		genetic::replacement::ReplaceWorst,
		PortGraphPopulationGenerator,
		Genotype,
		PortGraphIndividual
	> mOptimizer;

	bool mProgress = false;
	int mStep = 100;
	PortGraphIndividual *mDisplay = nullptr;

public:
	PortGraphObserver(Element *parent, std::string name);

	void draw(const Clock &clock, nk_context *ctx) override;
	void draw(const Clock &clock) override;
};
}
