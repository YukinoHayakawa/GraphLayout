﻿#include "PortGraphObserver.hpp"

#include <fstream>
#include <optional>
#include <array>

#include <Usagi/Core/Format.hpp>
#include <Usagi/Extensions/SysImGui/ImGui.hpp>
#include <Usagi/Math/Matrix.hpp>
#include <Usagi/Math/Angle.hpp>
#include <Usagi/Math/Bound.hpp>

#ifdef _WIN32
#    define WIN32_LEAN_AND_MEAN
#    include <Windows.h>
#    undef min
#    undef max
#endif

namespace
{
using namespace usagi;

// https://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect
std::optional<Vector2f> get_line_intersection(
    const Vector2f &p0,
    const Vector2f &p1,
    const Vector2f &p2,
    const Vector2f &p3,
    const Vector2f &ignore0,
    const Vector2f &ignore1
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
        if(x == ignore0) return { };
        if(x == ignore1) return { };
        // sometimes we have crossing exactly at segment crossing,
        // so cannot do this.
        // if(x == p0 || x == p1 || x == p2 || x == p3) return false;
        return { x };
    }

    return { };
}

// from imgui
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
    const Vector2f &offset,
    const float control_factor_a,
    const float control_factor_b
)
{
    const Vector2f size = (p1 - p0).cwiseAbs();
    // const auto control_x = std::min(size.x(), 250.f);
    const auto control_x = size.x();

    return std::make_tuple(
        Vector2f(p0.x() + offset.x(), p0.y() + offset.y()),
        Vector2f(
            p0.x() + offset.x() + control_x * control_factor_a,
            p0.y() + offset.y()
        ),
        Vector2f(
            p1.x() + offset.x() - control_x * control_factor_b,
            p1.y() + offset.y()),
        Vector2f(p1.x() + offset.x(), p1.y() + offset.y())
    );
}
}

std::size_t PortGraphFitness::countEdgeCrossings(
    PortGraphIndividual &g,
    std::size_t i)
{
    std::size_t cross = 0;

    auto *base_graph = g.graph.base_graph;
    const auto link_count = base_graph->links.size();
    auto &curve = g.bezier_curves[i];

    // estimate bezier intersections
    // for each other curves
    for(std::size_t j = i + 1; j < link_count; ++j)
    {
        // for each our line segments
        for(std::size_t ii = 0;
            ii < curve.points.size() - 1; ++ii)
        {
            // test against their line segments
            for(std::size_t jj = 0;
                jj < g.bezier_curves[j].points.size() - 1; ++jj)
            {
                if(!curve.bbox.intersects(g.bezier_curves[j].bbox))
                    continue;
                auto x = get_line_intersection(
                    curve.points[ii],
                    curve.points[ii + 1],
                    g.bezier_curves[j].points[jj],
                    g.bezier_curves[j].points[jj + 1],
                    // don't count lines starting from the same port
                    curve.points.front(),
                    // don't count lines ending at the same port
                    curve.points.back()
                );
                if(x.has_value())
                {
                    g.crosses.push_back(x.value());
                    ++cross;
                }
                // g.f_link_crossing += edge_crossing_penalty;
            }
        }
    }
    return cross;
}

std::size_t PortGraphFitness::countNodeEdgeCrossings(
    PortGraphIndividual &g,
    const std::size_t i,
    const float control_factor_a,
    const float control_factor_b,
    const bool insert_crossings)
{
    auto *base_graph = g.graph.base_graph;
    const auto node_count = base_graph->nodes.size();

    auto &curve = g.bezier_curves[i];
    curve.factor_a = control_factor_a;
    curve.factor_b = control_factor_b;
    // build bezier curves and bounding box
    auto [p0, p1] = g.graph.mapLinkEndPoints(i);
    auto [a, b, c, d] = getBezierControlPoints(p0, p1, Vector2f::Zero(), curve.factor_a, curve.factor_b);
    // PathBezierToCasteljau(bezier_points[i], a, b, c, d);
    PathBezierCurveTo(curve.points, a, b, c, d);
    curve.bbox = AlignedBox2f();
    for(auto &&p : curve.points)
    {
        curve.bbox.extend(p);
    }

    std::size_t cross = 0;

    // estimate bezier and node intersections

    // for each node box
    for(std::size_t j = 0; j < node_count; ++j)
    {
        auto r = g.graph.mapNodeRegion(j);
        // the curve cannot intersect with this node
        if(!curve.bbox.intersects(r))
            continue;
        // test our line segments with each of the node box edges
        for(std::size_t ii = 0;
            ii < curve.points.size() - 1; ++ii)
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
                auto x = get_line_intersection(
                    curve.points[ii],
                    curve.points[ii + 1],
                    r.corner(e.first),
                    r.corner(e.second),
                    // don't count line beginning and ending as crossings
                    curve.points.front(),
                    curve.points.back()
                );
                if(x.has_value())
                {
                    // tentatively test to find the best routing
                    if(insert_crossings)
                        g.crosses.push_back(x.value());
                    ++cross;
                }
                // g.f_link_node_crossing += edge_node_crossing_penalty;
            }
        }
    }
    return cross;
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
    g.c_angle = 0;
    g.c_invert_pos = 0;
    const auto node_count = base_graph->nodes.size();
    const auto link_count = base_graph->links.size();
    g.bezier_curves.resize(link_count);
    g.crosses.clear();
    // calculate overlapped area
    for(std::size_t i = 0; i < node_count; ++i)
    {
        auto r0 = g.graph.mapNodeRegion(i);
        for(auto j = i + 1; j < node_count; ++j)
        {
            auto r1 = g.graph.mapNodeRegion(j);
            const auto overlapped = r0.intersection(r1);
            if(!overlapped.isEmpty())
                g.f_overlap += node_overlap_penalty;
        }
    }
    // measure angles and edge directions
    for(std::size_t i = 0; i < link_count; ++i)
    {
        auto[p0, p1] = g.graph.mapLinkEndPoints(i);
        Vector2f edge_diff = p1 - p0;
        Vector2f normalized_edge = edge_diff.normalized();
        // normalized edge direction using dot product. prefer edge towards
        // right.
        const auto angle = std::acos(normalized_edge.dot(Vector2f::UnitX()));
        // output port is to the left of input port
        g.f_link_pos += std::min(edge_diff.x(), p_min_pos_x);
        if(edge_diff.x() < p_min_pos_x)
            ++g.c_invert_pos;
        // prefer smaller angle
        const auto deg_angle = radiansToDegrees(angle);
        g.f_link_angle -= std::max(p_max_angle, deg_angle);
        if(deg_angle > p_max_angle)
            ++g.c_angle;
    }
    // calculate link position
    // const auto link_count = base_graph->links.size();
    // for(std::size_t i = 0; i < link_count; ++i)
    // {
    //     auto [pos0, pos1] = g.graph.mapLinkEndPoints(i);
    //     g.f_link_pos -= (pos0 - pos1).norm();
    // }
    // calculate link angle

    for(std::size_t m = 0; m < link_count; ++m)
    {
        if(heuristic)
        {
            std::array<float, 4> ctrl_factor = { 0.8f, 0.6f, 0.4f, 0.2f };
            constexpr auto size = ctrl_factor.size();
            struct setting
            {
                float ca, cb;
                std::size_t en_cross;

                bool operator<(setting &rhs) const
                {
                    // try to reduce edge-node crossings
                    return en_cross < rhs.en_cross;
                }
            };
            std::array<setting, size * size> cross_count;
            // fill combinations
            {
                std::size_t k = 0;
                for(std::size_t i = 0; i < ctrl_factor.size(); ++i)
                {
                    for(std::size_t j = 0; j < ctrl_factor.size(); ++j)
                    {
                        cross_count[k++] = { ctrl_factor[i], ctrl_factor[j], 0 };
                    }
                }
            }
            std::stable_sort(cross_count.begin(), cross_count.end(),
                [](auto &a, auto &b) {
                    return std::abs(a.ca - a.cb) < std::abs(b.ca - b.cb);
                });
            for(auto &&s : cross_count)
            {
                s.en_cross = countNodeEdgeCrossings(
                    g, m, s.ca, s.cb, false);
            }
            const auto min = std::min_element(
                cross_count.begin(), cross_count.end());
            // generating bezier curve segments here
            const auto x = countNodeEdgeCrossings(g, m, min->ca, min->cb, true);
            g.f_link_node_crossing += x * edge_node_crossing_penalty;
        }
        else
        {
            g.f_link_node_crossing += edge_node_crossing_penalty * countNodeEdgeCrossings(g, m, 0.8f, 0.8f, true);
        }
    }
    for(std::size_t m = 0; m < link_count; ++m)
    {
        g.f_link_crossing += edge_crossing_penalty * countEdgeCrossings(g, m);
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

void PortGraphObserver::performRandomizedTest(int node_amount)
{
    if(!mContinueTests) return;
    assert(node_amount >= 0);

    LOG(info, "Starting randomized test with {} nodes", node_amount);
    const auto filename = mTestFolder / fmt::format("test_{}.csv", node_amount);
    std::ofstream log { filename };
    if(!log)
    {
        LOG(error, "Falied to open: {}, aborting test.", filename);
        return;
    }

    OptimizerT optimizer;
    auto &proto = optimizer.generator.prototype;
    optimizer.fitness.heuristic = mTest.heuristic;

    const auto canvas_size = mTest.canvas_size_per_node * node_amount;
    // set canvas size proportionate to the amount of nodes
    const auto domain = std::uniform_real_distribution<float> {
        0.f, canvas_size
    };
    optimizer.generator.domain = domain;
    // proportional to canvas size of node graph
    optimizer.mutation.domain = domain;

    // create one prototype which we will use through out the test
    proto.prototypes.emplace_back(
        std::string {}, Vector2f { 100, 100 },
        mTest.pin_amount, mTest.pin_amount);

    // insert nodes
    proto.nodes.assign(
        node_amount,  { &proto.prototypes.front(), std::string {} }
    );

    const auto pin_count = int(mTest.pin_connection_rate * node_amount);
    assert(pin_count >= 0);
    assert(proto.nodes.size() > 0);
    const std::uniform_int_distribution<std::size_t> node_dist {
        0, proto.nodes.size() - 1
    };
    assert(mTest.pin_amount > 0);
    const std::uniform_int_distribution<std::size_t> pin_dist {
        0, std::size_t(mTest.pin_amount - 1)
    };
    std::mt19937 rng { std::random_device()() };
    // for each random graph, create random links
    for(int i = 0; i < mTest.generation; ++i)
    {
        if(!mContinueTests) goto abort;

        proto.links.clear();
        // generate random links
        for(int j = 0; j < pin_count; ++j)
        {
            proto.links.emplace_back(
                node_dist(rng), pin_dist(rng),
                node_dist(rng), pin_dist(rng)
            );
        }
        // repeat optimization process
        for(int j = 0; j < mTest.repeat; ++j)
        {
            if(!mContinueTests) goto abort;

            optimizer.initializePopulation(mTest.population);
            optimizer.stop_condition = mTest.stop;

            const auto begin_time = std::chrono::high_resolution_clock::now();
            while(mContinueTests && !optimizer.stopCondition())
                optimizer.step();
            const auto end_time = std::chrono::high_resolution_clock::now();
            const std::chrono::duration<double> delta_time
                = end_time - begin_time;
            // nodes, links, unit_canvas, canvas, ports, connection_rate,
            // population, finish_iterations, time, fitness,
            // edge_crossings, edge_node_crossings, overlap,
            // c_invert_pos, f_link_pos, c_angle, f_link_angle,
            // stop_threshold, stop_period,
            // heuristic
            if(mContinueTests)
            {
                LOG(info, "{} nodes: graph {}, opti {}, time {}",
                    node_amount, i, j, delta_time.count());
                auto out = fmt::format(
                    "{}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}",
                    proto.nodes.size(),
                    proto.links.size(),
                    mTest.canvas_size_per_node,
                    canvas_size,
                    mTest.pin_amount,
                    mTest.pin_connection_rate,
                    optimizer.population.size(),
                    optimizer.year,
                    delta_time.count(),
                    optimizer.best.top()->fitness,
                    optimizer.best.top()->f_link_crossing /
                        optimizer.fitness.edge_crossing_penalty,
                    optimizer.best.top()->f_link_node_crossing /
                        optimizer.fitness.edge_node_crossing_penalty,
                    optimizer.best.top()->f_overlap /
                        optimizer.fitness.node_overlap_penalty,
                    optimizer.best.top()->c_invert_pos,
                    optimizer.best.top()->f_link_pos,
                    optimizer.best.top()->c_angle,
                    optimizer.best.top()->f_link_angle,
                    optimizer.stop_condition.significant_improvement_threshold,
                    optimizer.stop_condition.significant_improvement_period,
                    optimizer.fitness.heuristic
                );
                LOG(info, out);
                log << out << std::endl;
            }
            else
            {
                goto abort;
            }
        }
    }

    LOG(info, "Finished randomized test with {} nodes", node_amount);
    return;

abort:
    LOG(info, "Test with {} nodes aborted.", node_amount);
}

// https://stackoverflow.com/questions/9094422/how-to-check-if-a-stdthread-is-still-running
void PortGraphObserver::performRandomizedTests()
{
    if(mTestName.empty())
        mTestFolder = fmt::format("tests/{}", time(nullptr));
    else
        mTestFolder = fmt::format("tests/{}_{}", time(nullptr), mTestName.c_str());
    create_directories(mTestFolder);

    using namespace std::chrono_literals;

    assert(!mTestThread.valid()
        || mTestThread.wait_for(0s) == std::future_status::ready);
    mTestThread = std::async(std::launch::async, [this] {
#ifdef _WIN32
        // keep system responsive while running computation intensive task
        LOG(info, "Setting process priority to low.");
        SetPriorityClass(GetCurrentProcess(), IDLE_PRIORITY_CLASS);
#endif
        std::vector<int> indices;
        // indices.reserve(mTest.finish_node_amount - mTest.start_node_amount + 1);
        for(auto i = mTest.start_node_amount;
            i <= mTest.finish_node_amount; i += mTest.step)
        {
            indices.emplace_back(i);
        }
        LOG(info, "Launching test threads...");
        std::for_each(
            std::execution::par,
            indices.begin(), indices.end(), [this](auto i) {
                performRandomizedTest(i);
            }
        );

#ifdef _WIN32
        LOG(info, "Restoring process priority to normal.");
        SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
#endif
    });
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
        // draw_list->AddRect(
        //     scr({ 0, 0 }), scr({ mCanvasSize, mCanvasSize }),
        //     IM_COL32(205, 92, 92, 255));
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
            auto &curve = show->bezier_curves[i];
            auto &points = curve.points;
            auto [p0, p1] = g.mapLinkEndPoints(i);
            auto [a, b, c, d] = getBezierControlPoints(
                p0, p1, (Vector2f&)p,
                curve.factor_a, curve.factor_b);

            if(mShowDebugBezierCurves)
            {

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
        if(CollapsingHeader("Test", ImGuiTreeNodeFlags_DefaultOpen))
        {
            SliderInt("Start Node Amount", &mTest.start_node_amount,
                1, 200);
            SliderInt("Finish Node Amount", &mTest.finish_node_amount,
                mTest.start_node_amount, 200);
            SliderInt("Step", &mTest.step,
                1, 20);
            SliderInt("# Graph", &mTest.generation,
                1, 20);
            SliderInt("# Optimization", &mTest.repeat,
                1, 20);
            SliderInt("Amount of Ports", &mTest.pin_amount,
                1, 20);
            SliderInt("Population", &mTest.population,
                100, 200);
            SliderFloat("Connection Rate (#Edges/#Nodes)",
                &mTest.pin_connection_rate, 0, 20);
            SliderFloat("Canvas Size Per Node",
                &mTest.canvas_size_per_node, 100, 500);

            Checkbox("Use Bezier Control Point Heuristic",
                &mTest.heuristic);

            SliderFloat("Stop Threshold",
                &mTest.stop.significant_improvement_threshold, 50, 500);
            int iterations = mTest.stop.significant_improvement_period;
            SliderInt("Stop Iterations",
                &iterations, 1000, 20'000);
            mTest.stop.significant_improvement_period = iterations;

            mTestName.resize(128);
            InputText("Test Name", mTestName.data(), mTestName.size());
            using namespace std::chrono_literals;
            if(!mTestThread.valid() || mTestThread.wait_for(0s) == std::future_status::ready)
            {
                if(Button("Launch Tests"))
                    performRandomizedTests();
            }
            else
            {
                if(Button("Abort Tests"))
                {
                    mContinueTests = false;
                    mTestThread.wait();
                    mContinueTests = true;
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
                &mOptimizer.stop_condition.significant_improvement_threshold,
                1, 1000);
            int period = mOptimizer.stop_condition.significant_improvement_period;
            SliderInt("Significant Improvement Period",
                &period,
                1'000, 100'000);
            mOptimizer.stop_condition.significant_improvement_period = period;
            Checkbox("Use Bezier Heuristic",
                &mOptimizer.fitness.heuristic);
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
