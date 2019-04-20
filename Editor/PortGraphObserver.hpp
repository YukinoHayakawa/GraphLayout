﻿#pragma once

#include <Usagi/Core/Element.hpp>
#include <Usagi/Extension/ImGui/ImGuiComponent.hpp>
#include <GraphLayout/Graph/NodeGraph.hpp>
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
	float f_link_crossing = 0;
	float f_link_node_crossing = 0;

	std::vector<Vector2f> crosses;
};

struct PortGraphFitness
{
	static constexpr std::size_t BEZIER_SEGMENT_COUNT = 6;
	using value_type = float;
	// + 1 for beginning point
	std::vector<std::array<Vector2f, BEZIER_SEGMENT_COUNT + 1>> bezier_points;

	value_type operator()(PortGraphIndividual &g);
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
	int mStep = 100;
	PortGraphIndividual *mDisplay = nullptr;

	void initPopulation();

public:
	PortGraphObserver(Element *parent, std::string name);

	void draw(const Clock &clock) override;
};
}
