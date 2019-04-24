#include "PortGraphObserver.hpp"

#include <fmt/format.h>

#include <Usagi/Extension/ImGui/ImGui.hpp>

namespace
{
using namespace usagi;

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
	const Vector2f &p0,
	const Vector2f &p1,
	const Vector2f &p2,
	const Vector2f &p3,
	std::vector<Vector2f> &crosses
)
{
	const Vector2f s1 = p1 - p0;
	const Vector2f s2 = p3 - p2;

	float s, t;
	s = (-s1.y() * (p0.x() - p2.x()) + s1.x() * (p0.y() - p2.y())) / (-s2.x() *
		s1.y() + s1.x() *
		s2.y());
	t = (s2.x() * (p0.y() - p2.y()) - s2.y() * (p0.x() - p2.x())) / (-s2.x() *
		s1.y() + s1.x() *
		s2.y());

	if(s >= 0 && s <= 1 && t >= 0 && t <= 1)
	{
		const Vector2f x = p0 + t * s1;
		// sometimes we have crossing exactly at segment crossing,
		// so cannot do this.
		// if(x == p0 || x == p1 || x == p2 || x == p3) return false;
		crosses.push_back(x);
		return true;
	}

	return false;
}

// imgui
// http://www.malinc.se/m/DeCasteljauAndBezier.php
void PathBezierToCasteljau(
	std::vector<Vector2f> &points,
	const Vector2f &p1,
	const Vector2f &p2,
	const Vector2f &p3,
	const Vector2f &p4,
	float tess_tol = 0.99f,
	int level = 0)
{
	if(level == 0) points.emplace_back(p1.x(), p1.y());
	const auto dx = p4.x() - p1.x();
	const auto dy = p4.y() - p1.y();
	const auto d2 = std::abs((p2.x() - p4.x()) * dy - (p2.y() - p4.y()) * dx);
	const auto d3 = std::abs((p3.x() - p4.x()) * dy - (p3.y() - p4.y()) * dx);
	if((d2 + d3) * (d2 + d3) < tess_tol * (dx * dx + dy * dy))
	{
		points.emplace_back(p4.x(), p4.y());
	}
	else if(level < 5)
	{
		const Vector2f p12 = 0.5f * (p1 + p2);
		const Vector2f p23 = 0.5f * (p2 + p3);
		const Vector2f p34 = 0.5f * (p4 + p4);
		const Vector2f p123 = 0.5f * (p12 + p23);
		const Vector2f p234 = 0.5f * (p23 + p34);
		const Vector2f p1234 = 0.5f * (p123 + p234);
		PathBezierToCasteljau(points, p1, p12, p123, p1234,
			tess_tol, level + 1);
		PathBezierToCasteljau(points, p1234, p234, p34, p4,
			tess_tol, level + 1);
	}
}

template <std::size_t I>
void PathBezierCurveTo(
	std::array<Vector2f, I> &points,
	const Vector2f &p1,
	const Vector2f &p2,
	const Vector2f &p3,
	const Vector2f &p4)
{
	points[0] = p1;
	float t_step = 1.0f / (float)(I - 1);
	for(int i_step = 1; i_step <= I - 1; i_step++)
	{
		float t = t_step * i_step;
		float u = 1.0f - t;
		float w1 = u * u*u;
		float w2 = 3 * u*u*t;
		float w3 = 3 * u*t*t;
		float w4 = t * t*t;
		points[i_step] = Vector2f(
			w1*p1.x() + w2 * p2.x() + w3 * p3.x() + w4 * p4.x(),
			w1*p1.y() + w2 * p2.y() + w3 * p3.y() + w4 * p4.y()
		);
	}
}


auto getBezierControlPoints(
	const Vector2f &p0,
	const Vector2f &p1,
	const Vector2f &offset
)
{
	const Vector2f size = (p1 - p0).cwiseAbs();
	const auto control_x = std::min(size.x(), 250.f);

	return std::make_tuple(
		Vector2f(p0.x() + offset.x(), p0.y() + offset.y()),
		Vector2f(
			p0.x() + offset.x() + control_x * 0.8f,
			p0.y() + offset.y()
		),
		Vector2f(
			p1.x() + offset.x() - control_x * 0.8f,
			p1.y() + offset.y()),
		Vector2f(p1.x() + offset.x(), p1.y() + offset.y())
	);
}
}

PortGraphFitness::FitnessT PortGraphFitness::operator()(
	PortGraphIndividual &g)
{
	float fit = 0;
	auto *base_graph = g.graph.base_graph;

	// centers graph
	if(center_graph)
	{
		const auto center = g.graph.base_graph->size.x() * 0.5f;
		const auto sum = std::accumulate(
			g.genotype.begin(), g.genotype.end(), 0.f);
		const auto mean = sum / g.genotype.size();
		std::transform(
			g.genotype.begin(), g.genotype.end(),
			g.genotype.begin(),
			[=](float v) { return v - mean + center; });
	}

	if(grid != 1)
	{
		std::transform(
			g.genotype.begin(), g.genotype.end(),
			g.genotype.begin(),
			[this](float v) { return std::floor(v / grid) * grid; });
	}

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
	g.bezier_curves.resize(link_count);
	g.crosses.clear();
	for(std::size_t i = 0; i < link_count; ++i)
	{
		auto [p0, p1] = g.graph.mapLinkEndPoints(i);
		// bezier_points[i].clear();
		auto [a, b, c, d] = getBezierControlPoints(p0, p1, Vector2f::Zero());
		// PathBezierToCasteljau(bezier_points[i], a, b, c, d);
		PathBezierCurveTo(g.bezier_curves[i].points, a, b, c, d);
		g.bezier_curves[i].bbox = AlignedBox2f();
		for(auto &&p : g.bezier_curves[i].points)
		{
			g.bezier_curves[i].bbox.extend(p);
		}
	}
	for(std::size_t i = 0; i < link_count; ++i)
	{
		auto [p0, p1] = g.graph.mapLinkEndPoints(i);

		AlignedBox2f box0 { p0, p1 };
		// estimate bezier intersections
		for(std::size_t j = i + 1; j < link_count; ++j)
		{
			auto [pp0, pp1] = g.graph.mapLinkEndPoints(j);
			for(std::size_t ii = 0;
				ii < g.bezier_curves[i].points.size() - 1; ++ii)
			{
				for(std::size_t jj = 0;
					jj < g.bezier_curves[j].points.size() - 1; ++jj)
				{
					if(!g.bezier_curves[i].bbox.intersects(g.bezier_curves[j].bbox))
						continue;
					if(get_line_intersection(
						g.bezier_curves[i].points[ii],
						g.bezier_curves[i].points[ii + 1],
						g.bezier_curves[j].points[jj],
						g.bezier_curves[j].points[jj + 1],
						g.crosses
					))
						g.f_link_crossing -= 100;
				}
			}
		}

		Vector2f edge_diff = p1 - p0;
		Vector2f normalized_edge = edge_diff.normalized();
		// normalized edge direction using dot product. prefer edge towards
		// right.
		const auto angle = std::acos(normalized_edge.dot(Vector2f::UnitX()));
		// output port is to the left of input port
		g.f_link_pos += std::min(edge_diff.x(), p_min_pos_x);
		// prefer smaller angle
		const auto deg_angle = radiansToDegrees(angle);
		g.f_link_angle -= std::max(p_max_angle, deg_angle);

		// estimate bezier and node intersections
		for(std::size_t j = 0; j < node_count; ++j)
		{
			auto r = g.graph.mapNodeRegion(j);
			// the curve cannot intersect with this node
			if(!g.bezier_curves[i].bbox.intersects(r))
				continue;
			for(std::size_t ii = 0;
				ii < g.bezier_curves[i].points.size() - 1; ++ii)
			{
				const std::pair<
					AlignedBox2f::CornerType, AlignedBox2f::CornerType
				> box_edges[] = {
					{ AlignedBox2f::TopLeft, AlignedBox2f::TopRight },
					{ AlignedBox2f::BottomLeft, AlignedBox2f::BottomRight },
					{ AlignedBox2f::TopLeft, AlignedBox2f::BottomLeft },
					{ AlignedBox2f::TopRight, AlignedBox2f::BottomRight },
				};
				for(auto &&e : box_edges)
				{
					if(get_line_intersection(
						g.bezier_curves[i].points[ii],
						g.bezier_curves[i].points[ii + 1],
						r.corner(e.first),
						r.corner(e.second),
						g.crosses
					))
						g.f_link_node_crossing -= 100;
				}
			}
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

void PortGraphObserver::loadGraph(const std::filesystem::path &filename)
{
	using namespace node_graph;

	auto &graph = mOptimizer.generator.prototype =
		NodeGraph::readFromFile(mGraphPath / filename);
	mCurrentGraph = filename;

	const auto domain = std::uniform_real_distribution<float> {
		0.f, (mCanvasSize = graph.size.x())
	};
	mOptimizer.generator.domain = domain;
	// proportional to canvas size of node graph
	mOptimizer.mutation.domain = domain;
	// mOptimizer.mutation.std_dev = 100;
	// todo prevent the graph from going off-center

	initPopulation();
}

PortGraphObserver::PortGraphObserver(Element *parent, std::string name)
	: Element(parent, std::move(name))
{
	addComponent(static_cast<ImGuiComponent*>(this));

	loadGraph("default.ng");
}

void PortGraphObserver::initPopulation()
{
	mDisplay = nullptr;
	mOptimizer.initializePopulation(200);
}

void PortGraphObserver::draw(const Clock &clock)
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

		SetCursorPos({ 0, 0 });
		const ImVec2 p = GetCursorScreenPos();
		const auto scr = [p](const Vector2f &v) {
			return ImVec2 { v.x() + p.x, v.y() + p.y };
		};
		draw_list->AddRect(
			scr({ 0, 0 }), scr({ mCanvasSize, mCanvasSize }),
			IM_COL32(205, 92, 92, 255));
		for(std::size_t i = 0; i < b.nodes.size(); ++i)
		{
			auto &n = b.node(i);
			auto r = g.mapNodeRegion(i);
			SetCursorPos({ r.min().x(), r.min().y() });
			Button(fmt::format("{}##{}",
				n.name.empty() ? n.prototype->name.c_str() : n.name.c_str(), i).c_str(),
				{ r.sizes().x(), r.sizes().y() }
			);
			if(mShowPorts)
			{
				for(auto &&port : n.prototype->out_ports)
				{
					draw_list->AddCircleFilled(
						scr(n.prototype->portPosition(port, g.mapNodePosition(i))),
						5, IM_COL32(72, 61, 139, 200)
					);
				}
				for(auto &&port : n.prototype->in_ports)
				{
					draw_list->AddCircleFilled(
						scr(n.prototype->portPosition(port, g.mapNodePosition(i))),
						5, IM_COL32(72, 61, 139, 200)
					);
				}
			}
			if(IsItemActive() && IsMouseDragging())
			{
				g.node_positions[i] += Vector2f {
					GetIO().MouseDelta.x, GetIO().MouseDelta.y
				};
				mOptimizer.reevaluateIndividual(*show);
			}
			// todo draw ports
		}

		for(std::size_t i = 0; i < b.links.size(); ++i)
		{
			auto [p0, p1] = g.mapLinkEndPoints(i);
			auto [a, b, c, d] = getBezierControlPoints(p0, p1, (Vector2f&)p);

			if(mShowDebugBezierCurves)
			{
				auto &curve = show->bezier_curves[i];
				auto &points = curve.points;
				draw_list->AddRect(
					scr(curve.bbox.corner(AlignedBox2f::TopLeft)),
					scr(curve.bbox.corner(AlignedBox2f::BottomRight)),
					IM_COL32(0, 255, 255, 128)
				);
				for(std::size_t j = 0; j < curve.points.size() - 1; ++j)
				{
					draw_list->AddLine(
						scr(points[j]), scr(points[j + 1]),
						IM_COL32(255, 0, 255, 128)
					);
				}
			}
			else
			{
				draw_list->AddBezierCurve(
					(ImVec2&)a,
					(ImVec2&)b,
					(ImVec2&)c,
					(ImVec2&)d,
					IM_COL32(47, 79, 79, 200),
					2//, PortGraphFitness::BEZIER_SEGMENT_COUNT
				);
			}
		}
		// draw edge crosses
		if(mShowCrossings)
		{
			for(auto &&c : show->crosses)
			{
				Vector2f center = c + (Vector2f&)p;
				draw_list->AddCircle(
					(const ImVec2&)center,
					2, IM_COL32(255, 0, 0, 255), 4
				);
			}
		}
	}
	End();

	if(Begin("Genetic Algorithm Control",
		nullptr,
		ImGuiWindowFlags_HorizontalScrollbar))
	{
		if(CollapsingHeader("Graphs", ImGuiTreeNodeFlags_DefaultOpen))
		{
			SliderFloat("Canvas Size", &mCanvasSize, 500, 3000);
			for(auto &&p : std::filesystem::directory_iterator(mGraphPath))
			{
				auto name = p.path().filename();
				if(Selectable(
					name.u8string().c_str(), name == mCurrentGraph,
					ImGuiSelectableFlags_AllowDoubleClick) && IsMouseDoubleClicked(0))
				{
					loadGraph(name);
				}
			}
		}
		if(CollapsingHeader("Debug", ImGuiTreeNodeFlags_DefaultOpen))
		{
			Checkbox("Show Debug Bezier Curves", &mShowDebugBezierCurves);
			Checkbox("Show Crossings", &mShowCrossings);
			Checkbox("Show Ports", &mShowPorts);
		}
		if(CollapsingHeader("Parameters", ImGuiTreeNodeFlags_DefaultOpen))
		{
			SliderInt("Grid Size", &mOptimizer.fitness.grid, 1, 100);
			Checkbox("Centers Graph", &mOptimizer.fitness.center_graph);
			SliderFloat("Max Angle",
				&mOptimizer.fitness.p_max_angle,
				0, 180);
			SliderFloat("Min Positive Edge Length X",
				&mOptimizer.fitness.p_min_pos_x,
				0, 200);
			Checkbox("Stop When Reached Termination Condition",
				&mStopWhenReachedTerminationCondition);
			SliderFloat("Significant Improvement Threshold",
				&mOptimizer.significant_improvement_threshold,
				1, 1000);
			int period = mOptimizer.significant_improvement_period;
			SliderInt("Significant Improvement Period",
				&period,
				1'000, 100'000);
			mOptimizer.significant_improvement_period = period;
		}
		SliderInt("Generations Per Step", &mStep, 1, 500);
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
				&mOptimizer.fitness_history.front().fitness,
				static_cast<int>(mOptimizer.fitness_history.size()),
				0, nullptr, FLT_MAX, FLT_MAX,
				{ 0, 300 },
				sizeof(decltype(mOptimizer.fitness_history.front()))
			);
			Text("Should Stop=%d", mOptimizer.stopCondition());
		}
		if(CollapsingHeader("Population", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if(Button("Inspect Realtime Best"))
				mDisplay = nullptr;
			if(Button("Select Current Best"))
				mDisplay = mOptimizer.best.top();
			// show best individual
			{
				auto &best = *mOptimizer.best.top();
				Text(fmt::format("Best: #{} Birth: {}, Family: {}, Gen: {}, Fit: {}[overlap={},link={},angle={},e_cross={},en_cross={}]",
					best.index,
					best.birthday,
					best.family,
					best.generation,
					best.fitness,
					best.f_overlap,
					best.f_link_pos,
					best.f_link_angle,
					best.f_link_crossing,
					best.f_link_node_crossing
				).c_str());
			}
			for(std::size_t i = 0; i < mOptimizer.population.size(); ++i)
			{
				auto &ind = mOptimizer.population[i];
				if(Selectable(format(
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
		if(!(mStopWhenReachedTerminationCondition
			&& mOptimizer.stopCondition()))
		{
			for(int i = 0; i < mStep; ++i)
				mOptimizer.step();
		}
	}
}
