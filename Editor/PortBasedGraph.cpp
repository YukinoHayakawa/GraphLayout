#include "PortBasedGraph.hpp"

#include <fmt/format.h>

#include <Usagi/Extension/ImGui/ImGui.hpp>

usagi::PortBasedGraph::PortBasedGraph(Element *parent, std::string name)
    : Element(parent, std::move(name))
{
    addComponent(this);

    static NodePrototype p0 { "Node", { 150, 100 } };
    p0.in_ports.emplace_back("in0", Port::Edge::WEST, 0.25f);
    p0.in_ports.emplace_back("in1", Port::Edge::WEST, 0.5f);
    p0.in_ports.emplace_back("in2", Port::Edge::WEST, 0.75f);
    p0.out_ports.emplace_back("out0", Port::Edge::EAST, 0.25f);
    p0.out_ports.emplace_back("out1", Port::Edge::EAST, 0.5f);
    p0.out_ports.emplace_back("out2", Port::Edge::EAST, 0.75f);

    mGraph.nodes.emplace_back(&p0, Vector2f { 70, 200 });
    mGraph.nodes.emplace_back(&p0, Vector2f { 350, 60 });
    mGraph.nodes.emplace_back(&p0, Vector2f { 350, 200 });
    mGraph.nodes.emplace_back(&p0, Vector2f { 350, 400 });
    mGraph.nodes.emplace_back(&p0, Vector2f { 700, 200 });

    mGraph.links.emplace_back(0, 0, 1, 1);
    mGraph.links.emplace_back(0, 1, 2, 1);
    mGraph.links.emplace_back(0, 2, 3, 2);
    mGraph.links.emplace_back(1, 1, 2, 0);
    mGraph.links.emplace_back(2, 1, 4, 1);
    mGraph.links.emplace_back(3, 1, 2, 2);
    mGraph.links.emplace_back(3, 2, 4, 2);
}

void usagi::PortBasedGraph::draw(const Clock &clock, nk_context *ctx)
{
    if(nk_begin(ctx, fmt::format("Graph {}", (void*)this).c_str(),
        nk_rect(0, 0, 1000, 1000),
        NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_MINIMIZABLE |
        NK_WINDOW_SCALABLE | NK_WINDOW_CLOSABLE))
    {
        auto *canvas = nk_window_get_canvas(ctx);
        const auto total_space = nk_window_get_content_region(ctx);
        const auto bound = nk_layout_space_bounds(ctx);
        nk_layout_space_begin(ctx, NK_STATIC, total_space.h,
            (int)mGraph.nodes.size());

        for(auto &&n : mGraph.nodes)
        {
            auto *proto = n.prototype;
            nk_layout_space_push(ctx, nk_rect(
                n.position.x(), n.position.y(),
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
            auto pos0 = node0.portPosition(port0);
            auto pos1 = node1.portPosition(port1);
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
