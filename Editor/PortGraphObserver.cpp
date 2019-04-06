#include "PortGraphObserver.hpp"

#include <fmt/format.h>

#include <Usagi/Extension/ImGui/ImGui.hpp>
#include <Usagi/Core/Logging.hpp>

usagi::PortGraphFitness::value_type usagi::PortGraphFitness::operator()(
	PortGraphIndividual &g)
{
	float fit = 0;
	for(auto &&l : g.graph->links)
	{
		// Vector2f &p0 = g.position(l.node0);
		// Vector2f &p1 = g.position(l.node1);
		auto &node0 = g.graph->nodes[l.node0];
		auto &node1 = g.graph->nodes[l.node1];
		auto *proto0 = node0.prototype;
		auto *proto1 = node1.prototype;
		auto &port0 = proto0->out_ports[l.port0];
		auto &port1 = proto1->in_ports[l.port1];
		auto pos0 = node0.portPosition(
			port0, g.position(l.node0));
		auto pos1 = node1.portPosition(
			port1, g.position(l.node1));
		// f -= std::abs((pos0 - pos1).norm() - 100.f);
		// // f -= (p1 - p0).norm();
		// f -= std::abs((pos1 - pos0).dot(Vector2f::UnitY()));
		// prefer

		Vector2f edge_diff = pos1 - pos0;
		Vector2f normalized_edge = edge_diff.normalized();
		// normalized edge direction using dot product. prefer edge towards
		// right.
		const auto edge_direction = normalized_edge.dot(Vector2f::UnitX());
		// prefer given edge length
		const auto edge_length = -std::pow(edge_diff.norm() - 300.f, 2.f) + 1;
		fit += w_dir * edge_direction
			* w_length * edge_length;
	}
	return fit;
}

usagi::PortGraphObserver::PortGraphObserver(Element *parent, std::string name)
	: Element(parent, std::move(name))
{
	addComponent(static_cast<ImGuiComponent*>(this));
	addComponent(static_cast<NuklearComponent*>(this));

	static NodePrototype p0 { "Node", { 150, 100 } };
	p0.in_ports.emplace_back("in0", Port::Edge::WEST, 0.25f);
	p0.in_ports.emplace_back("in1", Port::Edge::WEST, 0.5f);
	p0.in_ports.emplace_back("in2", Port::Edge::WEST, 0.75f);
	p0.out_ports.emplace_back("out0", Port::Edge::EAST, 0.25f);
	p0.out_ports.emplace_back("out1", Port::Edge::EAST, 0.5f);
	p0.out_ports.emplace_back("out2", Port::Edge::EAST, 0.75f);

	mGraph.nodes.emplace_back(&p0); // , Vector2f{ 70, 200 });
	mGraph.nodes.emplace_back(&p0); // , Vector2f{ 350, 60 });
	mGraph.nodes.emplace_back(&p0); // , Vector2f{ 350, 200 });
	mGraph.nodes.emplace_back(&p0); // , Vector2f{ 350, 400 });
	mGraph.nodes.emplace_back(&p0); // , Vector2f{ 700, 200 });

	mGraph.links.emplace_back(0, 0, 1, 1);
	mGraph.links.emplace_back(0, 1, 2, 1);
	mGraph.links.emplace_back(0, 2, 3, 2);
	mGraph.links.emplace_back(1, 1, 2, 0);
	mGraph.links.emplace_back(2, 1, 4, 1);
	mGraph.links.emplace_back(3, 1, 2, 2);
	mGraph.links.emplace_back(3, 2, 4, 2);

	// proportional to canvas size of node graph
	mOptimizer.mutation.std_dev = 10;
	// todo prevent the graph from going off-center

	mOptimizer.generator.size = mGraph.nodes.size() * 2;
	mOptimizer.generator.linked_graph = &mGraph;

	mOptimizer.initializePopulation(100);
}

void usagi::PortGraphObserver::draw(const Clock &clock, nk_context *ctx)
{
	const auto disp = mDisplay ? mDisplay : mOptimizer.best;
	if(disp == nullptr)
		return;

	if(nk_begin(ctx, fmt::format("Graph {}", static_cast<void*>(this)).c_str(),
		nk_rect(0, 0, 1200, 1200),
		NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_MINIMIZABLE |
		NK_WINDOW_SCALABLE | NK_WINDOW_CLOSABLE))
	{
		auto *canvas = nk_window_get_canvas(ctx);
		const auto total_space = nk_window_get_content_region(ctx);
		const auto bound = nk_layout_space_bounds(ctx);
		nk_layout_space_begin(ctx, NK_STATIC, total_space.h,
			(int)mGraph.nodes.size());

		for(std::size_t i = 0; i < mGraph.nodes.size(); ++i)
		{
			auto &n = mGraph.nodes[i];
			auto *proto = n.prototype;
			auto &p = disp->position(i);
			nk_layout_space_push(ctx, nk_rect(
				p.x(), p.y(),
				proto->size.x(), proto->size.y()
			));
			nk_button_label(ctx, proto->name.c_str());
		}
		for(auto &&l : mGraph.links)
		{
			auto &node0 = mGraph.nodes[l.node0];
			auto &node1 = mGraph.nodes[l.node1];
			auto *proto0 = node0.prototype;
			auto *proto1 = node1.prototype;
			auto &port0 = proto0->out_ports[l.port0];
			auto &port1 = proto1->in_ports[l.port1];
			auto pos0 = node0.portPosition(
				port0, disp->position(l.node0));
			auto pos1 = node1.portPosition(
				port1, disp->position(l.node1));
			nk_stroke_line(
				canvas,
				pos0.x() + bound.x, pos0.y() + bound.y,
				pos1.x() + bound.x, pos1.y() + bound.y,
				2.f, nk_rgb(255, 255, 0)
			);
		}
	}
	nk_end(ctx);
}

void usagi::PortGraphObserver::draw(const Clock &clock)
{
	using namespace ImGui;

	if(Begin("Genetic Algorithm Control"))
	{
		SliderInt("Generations Per Step", &mStep, 1, 1000);
		Checkbox("Progress", &mProgress);
		if(Button("Step"))
			mOptimizer.step();
		if(Button("Init"))
		{
			mDisplay = nullptr;
			mOptimizer.initializePopulation(100);
		}
		if(CollapsingHeader("Fitness"))
		{
			DragFloat("Direction", &mOptimizer.fitness.w_dir, 0.01f);
			DragFloat("Length", &mOptimizer.fitness.w_length, 0.01f);
		}
		if(Button("Inspect Best"))
			mDisplay = mOptimizer.best;
		if(CollapsingHeader("Population"))
		{
			for(std::size_t i = 0; i < mOptimizer.population.size(); ++i)
			{
				auto &ind = mOptimizer.population[i];
				if(Selectable(fmt::format(
					"#{} Birth: {}, Family: {}, Gen: {}, Fit: {}, Chromo: {}",
					i,
					ind.birthday,
					ind.family,
					ind.generation,
					ind.fitness,
					fmt::join(ind.genotype.begin(), ind.genotype.end(), " ")
				).c_str()))
				{
					mDisplay = &ind;
				}
			}
		}

	}
	End();

	if(mProgress)
	{
		for(int i = 0; i < mStep; ++i)
			mOptimizer.step();
		// LOG(info, "Best: {}", mOptimizer.best->fitness);
	}
}
