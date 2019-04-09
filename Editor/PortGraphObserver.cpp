#include "PortGraphObserver.hpp"

#include <fmt/format.h>

#include <Usagi/Extension/ImGui/ImGui.hpp>
#include <Usagi/Core/Logging.hpp>

usagi::PortGraphFitness::value_type usagi::PortGraphFitness::operator()(
	PortGraphIndividual &g) const
{
	float fit = 0;
	auto *base_graph = g.graph.base_graph;

	// calculate overlapped area
	const auto node_count = base_graph->nodes.size();
	for(std::size_t i = 0; i < node_count; ++i)
	{
		auto r0 = g.graph.mapNodeRegion(i);
		for(auto j = i + 1; j < node_count; ++j)
		{
			auto r1 = g.graph.mapNodeRegion(j);
			const auto overlapped = r0.intersection(r1);
			if(!overlapped.isEmpty())
				fit -= overlapped.volume();
		}
	}
	const auto link_count = base_graph->links.size();
	for(std::size_t i = 0; i < link_count; ++i)
	{
		auto [pos0, pos1] = g.graph.mapLinkEndPoints(i);
		if(pos0.x() < pos1.x())
			fit += 1.f / (0.1f + std::abs((pos0 - pos1).norm() - 50.f));
	}
	/*for(auto &&l : base_graph->links)
	{
		auto [n0, p0, n1, p1] = base_graph->mapLink(l);

		auto r0 =

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
			+ w_length * edge_length;
	}*/
	return fit;
}

usagi::PortGraphObserver::PortGraphObserver(Element *parent, std::string name)
	: Element(parent, std::move(name))
{
	addComponent(static_cast<ImGuiComponent*>(this));
	addComponent(static_cast<NuklearComponent*>(this));

	using namespace node_graph;
	static NodePrototype color { "Single Color", { 100, 100 }, 0, 1 };
	static NodePrototype color4 { "RGBA Color", { 100, 120 }, 0, 5 };
	static NodePrototype constant { "Constant", { 50, 50 }, 0, 1 };
	static NodePrototype add { "Add", { 60, 60 }, 2, 1 };
	static NodePrototype multiply { "Multiply", { 60, 60 }, 2, 1 };
	static NodePrototype divide { "Divide", { 60, 60 }, 2, 1 };
	static NodePrototype condition { "Divide", { 70, 110 }, 5, 1 };
	static NodePrototype shader { "Shader", { 110, 320 }, 17, 0 };

	auto &g = mOptimizer.generator.prototype;
	g.nodes.emplace_back(&constant); // 0
	g.nodes.emplace_back(&constant); // 1
	g.nodes.emplace_back(&constant); // 2
	g.nodes.emplace_back(&constant); // 3
	g.nodes.emplace_back(&constant); // 4
	g.nodes.emplace_back(&constant); // 5
	g.nodes.emplace_back(&constant); // 6
	g.nodes.emplace_back(&constant); // 7
	g.nodes.emplace_back(&constant); // 8
	g.nodes.emplace_back(&divide); // 9
	g.nodes.emplace_back(&add); // 10
	g.nodes.emplace_back(&condition); // 11
	g.nodes.emplace_back(&condition); // 12
	g.nodes.emplace_back(&condition); // 13
	g.nodes.emplace_back(&condition); // 14
	g.nodes.emplace_back(&color); // 15
	g.nodes.emplace_back(&color4); // 16
	g.nodes.emplace_back(&shader); // 17
	g.nodes.emplace_back(&multiply); // 18
	g.nodes.emplace_back(&color4); // 19

	g.links.emplace_back(0,0,9,0);
	g.links.emplace_back(9,0,10,1);
	g.links.emplace_back(1,0,10,0);
	g.links.emplace_back(10,0,11,2);
	g.links.emplace_back(1,0,11,0);
	g.links.emplace_back(2,0,11,1);
	g.links.emplace_back(2,0,11,3);
	g.links.emplace_back(2,0,11,4);
	g.links.emplace_back(1,0,13,0);
	g.links.emplace_back(1,0,14,0);
	g.links.emplace_back(16,0,12,2);
	g.links.emplace_back(15,0,12,3);
	g.links.emplace_back(15,0,12,4);
	g.links.emplace_back(11,0,12,0);
	g.links.emplace_back(19,1,12,1);
	g.links.emplace_back(19,1,14,1);
	g.links.emplace_back(3,0,14,2);
	g.links.emplace_back(4,0,14,3);
	g.links.emplace_back(4,0,14,4);
	g.links.emplace_back(8,0,17,0);
	g.links.emplace_back(12,0,18,0);
	g.links.emplace_back(7,0,18,1);
	g.links.emplace_back(18,0,17,4);
	g.links.emplace_back(14,0,13,1);
	g.links.emplace_back(6,0,13,2);
	g.links.emplace_back(5,0,13,3);
	g.links.emplace_back(5,0,13,4);
	g.links.emplace_back(13,0,17,5);

	const auto domain = std::uniform_real_distribution<float> {
		0.f, 1000.f
	};
	mOptimizer.generator.domain = domain;
	// proportional to canvas size of node graph
	mOptimizer.mutation.domain = domain;
	// todo prevent the graph from going off-center

	mOptimizer.initializePopulation(100);
}

void usagi::PortGraphObserver::draw(const Clock &clock, nk_context *ctx)
{
	const auto show = mDisplay ? mDisplay : mOptimizer.best;
	if(show == nullptr)
		return;

	if(nk_begin(ctx,
		"Graph",
		nk_rect(0, 0, 1200, 1200),
		NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_MINIMIZABLE |
		NK_WINDOW_SCALABLE | NK_WINDOW_CLOSABLE))
	{
		auto *canvas = nk_window_get_canvas(ctx);
		const auto total_space = nk_window_get_content_region(ctx);
		const auto bound = nk_layout_space_bounds(ctx);
		auto &g = show->graph;
		auto &b = *g.base_graph;
		nk_layout_space_begin(ctx, NK_STATIC, total_space.h,
			static_cast<int>(b.nodes.size()));

		for(std::size_t i = 0; i < b.nodes.size(); ++i)
		{
			auto &n = b.node(i);
			auto r = g.mapNodeRegion(i);
			nk_layout_space_push(ctx, nk_rect(
				r.min().x(), r.min().y(),
				r.sizes().x(), r.sizes().y()
			));
			nk_button_label(ctx, n.prototype->name.c_str());
		}
		for(std::size_t i = 0; i < b.links.size(); ++i)
		{
			auto [pos0, pos1] = g.mapLinkEndPoints(i);
			nk_stroke_curve(
				canvas,
				pos0.x() + bound.x, pos0.y() + bound.y,
				pos0.x() + bound.x + 50.f, pos0.y() + bound.y,
				pos1.x() + bound.x - 50.f, pos1.y() + bound.y,
				pos1.x() + bound.x, pos1.y() + bound.y,
				2.f, nk_rgb(255, 255, 0)
			);
			/*nk_stroke_line(
				canvas,
				pos0.x() + bound.x, pos0.y() + bound.y,
				pos1.x() + bound.x, pos1.y() + bound.y,
				2.f, nk_rgb(255, 255, 0)
			);*/
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
