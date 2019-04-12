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
struct PortGraphIndividual : genetic::Individual<std::vector<float>, float>
{
	node_graph::PortGraph graph;

	float f_overlap = 0;
	float f_link_pos = 0;
	float f_link_angle = 0;
};

struct PortGraphFitness
{
	using value_type = float;

	value_type operator()(PortGraphIndividual &g) const;
};

struct PortGraphPopulationGenerator
{
	node_graph::NodeGraph prototype;
	std::uniform_real_distribution<float> domain { 0, 1 };

	template <typename Optimizer>
	PortGraphIndividual operator()(Optimizer &o)
	{
		PortGraphIndividual individual;
		individual.genotype.resize(prototype.nodes.size() * 2);
		std::generate(
			individual.genotype.begin(), individual.genotype.end(),
			// use ref for rng to prevent being copied
			std::bind(domain, std::ref(o.rng))
		);
		individual.graph.base_graph = &prototype;
		individual.graph.node_positions = reinterpret_cast<Vector2f*>(
			individual.genotype.data());
		return std::move(individual);
	}
};

class PortGraphObserver
	: public Element
	, public NuklearComponent
	, public ImGuiComponent
{
	using Gene = float;
	using Genotype = std::vector<float>;

	genetic::GeneticOptimizer<
		Gene,
		PortGraphFitness,
		genetic::parent::TournamentParentSelection<5, 2>,
		genetic::crossover::WholeArithmeticRecombination,
		genetic::mutation::UniformRealMutation<Genotype>,
		genetic::replacement::RoundRobinTournamentReplacement<10, 2>,
		PortGraphPopulationGenerator,
		Genotype,
		PortGraphIndividual
	> mOptimizer;

	bool mProgress = false;
	int mStep = 1000;
	PortGraphIndividual *mDisplay = nullptr;

	void initPopulation();

public:
	PortGraphObserver(Element *parent, std::string name);

	void draw(const Clock &clock, nk_context *ctx) override;
	void draw(const Clock &clock) override;
};
}
