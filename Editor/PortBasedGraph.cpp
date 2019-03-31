#include "PortBasedGraph.hpp"

#include <Usagi/Extension/ImGui/ImGui.hpp>

usagi::PortBasedGraph::PortBasedGraph(Element *parent, std::string name)
    : Element(parent, std::move(name))
{
    addComponent(this);

    mGraph.nodes.emplace_back();
    mGraph.nodes.back().rect = Eigen::AlignedBox2f {
        Vector2f { 100, 100 }, Vector2f { 150, 120 }
    };
    mGraph.nodes.back().addPort({ { }, Port::Direction::INCOMING, { } });
    mGraph.nodes.back().addPort({ { }, Port::Direction::INCOMING, { } });
    mGraph.nodes.back().addPort({ { }, Port::Direction::OUTGOING, { } });
    mGraph.nodes.back().addPort({ { }, Port::Direction::OUTGOING, { } });
    mGraph.nodes.back().addPort({ { }, Port::Direction::OUTGOING, { } });
}

void usagi::PortBasedGraph::draw(const Clock &clock)
{
    /*auto draw = ImGui::GetOverlayDrawList();

    constexpr auto iv = [](auto &&v) {
        return ImVec2 { (float)v.x(), (float)v.y() };
    };
    for(auto &&n : mGraph.nodes)
    {
        auto a = n.rect.corner(Eigen::AlignedBox2i::TopLeft);
        auto b = n.rect.corner(Eigen::AlignedBox2i::BottomRight);
        draw->AddRectFilled(iv(a), iv(b), ImColor(255, 0, 0));
    }

    for(auto &&l : mGraph.links)
    {
        /*l.first->position + #1#
        draw->AddLine(
            iv(l.first->node->rect.center()),
            iv(l.second->node->rect.center()),
            ImColor(255, 255, 0)
        );
    }*/
}
