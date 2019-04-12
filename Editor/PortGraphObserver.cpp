#include "PortGraphObserver.hpp"

#include <fmt/format.h>

#include <Usagi/Extension/ImGui/ImGui.hpp>
#include <Usagi/Core/Logging.hpp>

namespace
{
// https://stackoverflow.com/questions/99353/how-to-test-if-a-line-segment-intersects-an-axis-aligned-rectange-in-2d
bool SegmentIntersectRectangle(
	float a_rectangleMinX,
	float a_rectangleMinY,
	float a_rectangleMaxX,
	float a_rectangleMaxY,
	float a_p1x,
	float a_p1y,
	float a_p2x,
	float a_p2y)
{
	// Find min and max X for the segment

	float minX = a_p1x;
	float maxX = a_p2x;

	if(a_p1x > a_p2x)
	{
		minX = a_p2x;
		maxX = a_p1x;
	}

	// Find the intersection of the segment's and rectangle's x-projections

	if(maxX > a_rectangleMaxX)
	{
		maxX = a_rectangleMaxX;
	}

	if(minX < a_rectangleMinX)
	{
		minX = a_rectangleMinX;
	}

	if(minX > maxX) // If their projections do not intersect return false
	{
		return false;
	}

	// Find corresponding min and max Y for min and max X we found before

	float minY = a_p1y;
	float maxY = a_p2y;

	float dx = a_p2x - a_p1x;

	if(abs(dx) > 0.00001)
	{
		float a = (a_p2y - a_p1y) / dx;
		float b = a_p1y - a * a_p1x;
		minY = a * minX + b;
		maxY = a * maxX + b;
	}

	if(minY > maxY)
	{
		float tmp = maxY;
		maxY = minY;
		minY = tmp;
	}

	// Find the intersection of the segment's and rectangle's y-projections

	if(maxY > a_rectangleMaxY)
	{
		maxY = a_rectangleMaxY;
	}

	if(minY < a_rectangleMinY)
	{
		minY = a_rectangleMinY;
	}

	if(minY > maxY) // If Y-projections do not intersect return false
	{
		return false;
	}

	return true;
}

// https://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect
bool get_line_intersection(
	float p0_x,
	float p0_y,
	float p1_x,
	float p1_y,
	float p2_x,
	float p2_y,
	float p3_x,
	float p3_y,
	float *i_x,
	float *i_y)
{
	float s1_x, s1_y, s2_x, s2_y;
	s1_x = p1_x - p0_x;
	s1_y = p1_y - p0_y;
	s2_x = p3_x - p2_x;
	s2_y = p3_y - p2_y;

	float s, t;
	s = (-s1_y * (p0_x - p2_x) + s1_x * (p0_y - p2_y)) / (-s2_x * s1_y + s1_x *
		s2_y);
	t = (s2_x * (p0_y - p2_y) - s2_y * (p0_x - p2_x)) / (-s2_x * s1_y + s1_x *
		s2_y);

	if(s >= 0 && s <= 1 && t >= 0 && t <= 1)
	{
		// Collision detected
		if(i_x != NULL)
			*i_x = p0_x + (t * s1_x);
		if(i_y != NULL)
			*i_y = p0_y + (t * s1_y);
		return 1;
	}

	return 0; // No collision
}
}

usagi::PortGraphFitness::value_type usagi::PortGraphFitness::operator()(
	PortGraphIndividual &g) const
{
	float fit = 0;
	auto *base_graph = g.graph.base_graph;

	g.f_overlap = 0;
	g.f_link_pos = 0;
	g.f_link_angle = 0;
	g.f_link_crossing = 0;
	g.f_link_node_crossing = 0;
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
				g.f_overlap -= 1000;// overlapped.volume();
		}
	}
	// calculate link position
	const auto link_count = base_graph->links.size();
	// for(std::size_t i = 0; i < link_count; ++i)
	// {
	// 	auto [pos0, pos1] = g.graph.mapLinkEndPoints(i);
	// 	g.f_link_pos -= (pos0 - pos1).norm();
	// }
	// calculate link angle
	for(std::size_t i = 0; i < link_count; ++i)
	{
		auto [p0, p1] = g.graph.mapLinkEndPoints(i);
		Vector2f edge_diff = p1 - p0;
		Vector2f normalized_edge = edge_diff.normalized();
		// normalized edge direction using dot product. prefer edge towards
		// right.
		const auto angle = std::acos(normalized_edge.dot(Vector2f::UnitX()));
		const auto delta_x = p1.x() - p0.x();
		// output port is to the left of input port
		if(delta_x < 0)
			g.f_link_pos += delta_x;
		// prefer smaller angle
		if(angle > degreesToRadians(45.f))
			g.f_link_angle -= 100;
		// estimate bezier intersections
		for(std::size_t j = i + 1; j < link_count; ++j)
		{
			auto [pp0, pp1] = g.graph.mapLinkEndPoints(j);
			if(get_line_intersection(
				p0.x(), p0.y(), p1.x(), p1.y(),
				pp0.x(), pp0.y(), pp1.x(), pp1.y(), nullptr, nullptr
			))
				g.f_link_crossing -= 100;
		}
		// estimate bezier and node intersections
		for(std::size_t j = 0; j < node_count; ++j)
		{
			auto r = g.graph.mapNodeRegion(j);
			if(SegmentIntersectRectangle(
				r.min().x(), r.min().y(),
				r.max().x(), r.max().y(),
				p0.x(), p0.y(), p1.x(), p1.y()
			))
				g.f_link_node_crossing -= 100;
		}
	}
	fit = g.f_overlap
		+ g.f_link_pos
		+ g.f_link_angle
		+ g.f_link_crossing
		+ g.f_link_node_crossing;
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

	using namespace node_graph;
	static NodePrototype color { "Single Color", { 100, 100 }, 0, 1 };
	static NodePrototype color4 { "RGBA Color", { 100, 120 }, 0, 5 };
	static NodePrototype constant { "Constant", { 50, 50 }, 0, 1 };
	static NodePrototype add { "Add", { 60, 60 }, 2, 1 };
	static NodePrototype multiply { "Multiply", { 60, 60 }, 2, 1 };
	static NodePrototype divide { "Divide", { 60, 60 }, 2, 1 };
	static NodePrototype condition { "If", { 70, 110 }, 5, 1 };
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
		0.f, 1200.f
	};
	mOptimizer.generator.domain = domain;
	// proportional to canvas size of node graph
	mOptimizer.mutation.domain = domain;
	// mOptimizer.mutation.std_dev = 100;
	// todo prevent the graph from going off-center


	mOptimizer.fitness_history_max = 10000;


	initPopulation();
}

void usagi::PortGraphObserver::initPopulation()
{
	mDisplay = nullptr;
	mOptimizer.initializePopulation(200);
}

void usagi::PortGraphObserver::draw(const Clock &clock)
{
	using namespace ImGui;

	const auto show = mDisplay ? mDisplay : mOptimizer.best.top();
	if(Begin("Graph Inspection",
		nullptr,
		ImGuiWindowFlags_HorizontalScrollbar) && show)
	{
		auto &g = show->graph;
		auto &b = *g.base_graph;
		auto draw_list = GetWindowDrawList();

		for(std::size_t i = 0; i < b.nodes.size(); ++i)
		{
			auto &n = b.node(i);
			auto r = g.mapNodeRegion(i);
			SetCursorPos({ r.min().x(), r.min().y() });
			Button(fmt::format("{}##{}",
				n.prototype->name.c_str(), i).c_str(),
				{ r.sizes().x(), r.sizes().y() }
			);
			if(IsItemActive() && IsMouseDragging())
			{
				g.node_positions[i] += Vector2f {
					GetIO().MouseDelta.x, GetIO().MouseDelta.y
				};
				mOptimizer.reevaluateIndividual(*show);
			}
		}
		SetCursorPos({ 0, 0 });
		const ImVec2 p = GetCursorScreenPos();
		for(std::size_t i = 0; i < b.links.size(); ++i)
		{
			auto [p0, p1] = g.mapLinkEndPoints(i);
			const Vector2f size = (p1 - p0).cwiseAbs();
			draw_list->AddBezierCurve(
				{ p0.x() + p.x, p0.y() + p.y },
				{
					p0.x() + p.x + size.x() * 0.8f,
					p0.y() + p.y
				},
				{
					p1.x() + p.x - size.x() * 0.8f,
					p1.y() + p.y
				},
				{ p1.x() + p.x, p1.y() + p.y },
				IM_COL32(47, 79, 79, 200),
				2
			);
		}
	}
	End();

	if(Begin("Genetic Algorithm Control",
		nullptr,
		ImGuiWindowFlags_HorizontalScrollbar))
	{
		SliderInt("Generations Per Step", &mStep, 1, 2000);
		Checkbox("Progress", &mProgress);
		if(Button("Step"))
			mOptimizer.step();
		if(Button("Init"))
		{
			initPopulation();
		}
		if(CollapsingHeader("Fitness", ImGuiTreeNodeFlags_DefaultOpen))
		{
			PlotLines(
				"Best Fitness History",
				mOptimizer.fitness_history.data(),
				static_cast<int>(mOptimizer.fitness_history.size()),
				0, nullptr, FLT_MAX, FLT_MAX,
				{ 0, 300 }
			);
		}
		if(CollapsingHeader("Population", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if(Button("Inspect Realtime Best"))
				mDisplay = nullptr;
			if(Button("Select Current Best"))
				mDisplay = mOptimizer.best.top();
			for(std::size_t i = 0; i < mOptimizer.population.size(); ++i)
			{
				auto &ind = mOptimizer.population[i];
				if(Selectable(fmt::format(
					"#{} Birth: {}, Family: {}, Gen: {}, Fit: {}[overlap={},link={},angle={},e_cross={},en_cross={}], Chromo: {}##{}",
					i,
					ind.birthday,
					ind.family,
					ind.generation,
					ind.fitness,
					ind.f_overlap,
					ind.f_link_pos,
					ind.f_link_angle,
					ind.f_link_crossing,
					ind.f_link_node_crossing,
					fmt::join(ind.genotype.begin(), ind.genotype.end(), " "),
					i
				).c_str(), mDisplay == &ind))
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
