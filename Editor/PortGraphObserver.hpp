﻿#pragma once

#include <future>

#include <Usagi/Core/Element.hpp>
#include <Usagi/Extension/ImGui/ImGuiComponent.hpp>
#include <GraphLayout/Graph/NodeGraph.hpp>
#include <GraphLayout/Genetic/GeneticOptimizer.hpp>
#include <GraphLayout/Genetic/ParentSelection.hpp>
#include <GraphLayout/Genetic/Crossover.hpp>
#include <GraphLayout/Genetic/Mutation.hpp>
#include <GraphLayout/Genetic/Replacement.hpp>
#include <GraphLayout/Genetic/StopCondition.hpp>

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
	int c_angle = 0;
	int c_invert_pos = 0;

	std::vector<Vector2f> crosses;

	static constexpr std::size_t BEZIER_SEGMENT_COUNT = 6;
	static constexpr std::size_t BEZIER_POINT_COUNT = BEZIER_SEGMENT_COUNT + 1;
	struct BezierInfo
	{
		std::array<Vector2f, BEZIER_POINT_COUNT> points;
		AlignedBox2f bbox;
	};
	std::vector<BezierInfo> bezier_curves;
};

struct PortGraphFitness
{
	using FitnessT = float;

	bool center_graph = false;
	int grid = 1;
	float p_max_angle = 60;
	float p_min_pos_x = 50;

	float node_overlap_penalty = -1000;
	float edge_crossing_penalty = -100;
	float edge_node_crossing_penalty = -100;

	FitnessT operator()(PortGraphIndividual &g);
};

struct RandomTestConfig
{
	int start_node_amount = 4;
	int finish_node_amount = 100;
	// # of randomly generated graph
	int generation = 5;
	// # of repeated optimization on each generated graph
	int repeat = 5;
	int population = 100;
	float canvas_size_per_node = 250;

	int pin_amount = 5;
	// # of edges / # of nodes
	float pin_connection_rate = 2;
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

	using OptimizerT = genetic::GeneticOptimizer<
		Gene,
		PortGraphFitness,
		genetic::parent::TournamentParentSelection<5, 2>,
		genetic::crossover::WholeArithmeticRecombination,
		genetic::mutation::UniformRealMutation<Genotype>,
		genetic::replacement::RoundRobinTournamentReplacement<10, 2>,
		genetic::stop::SolutionConvergedStopCondition<float>,
		PortGraphPopulationGenerator,
		Genotype,
		PortGraphIndividual
	>;

	OptimizerT mOptimizer;

	bool mProgress = false;
	int mStep = 100;
	PortGraphIndividual *mDisplay = nullptr;
	bool mShowDebugBezierCurves = false;
	bool mShowPorts = true;
	bool mShowCrossings = false;
	float mCanvasSize = 1200;
	bool mStopWhenReachedTerminationCondition = true;
	std::filesystem::path mGraphPath = "Data/graphs";
	std::filesystem::path mCurrentGraph = "Data/graphs";
	std::filesystem::path mTestFolder;

	RandomTestConfig mTest;
	bool mContinueTests = true;
	std::future<void> mTestThread;

	void loadGraph(const std::filesystem::path &filename);
	void initPopulation();
	void performRandomizedTest(int node_amount);
	void performRandomizedTests();

public:
	PortGraphObserver(Element *parent, std::string name);

	void draw(const Clock &clock) override;
};
}
