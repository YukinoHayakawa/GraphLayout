#pragma once

#include <Usagi/Core/Element.hpp>
#include <Usagi/Extension/ImGui/ImGuiComponent.hpp>
#include <GraphLayout/Graph/NodeGraph.hpp>
#include <Usagi/Extension/Nuklear/NuklearComponent.hpp>

namespace usagi
{
class PortBasedGraph
    : public Element
    , public NuklearComponent
{
    NodeGraph mGraph;

public:
    PortBasedGraph(Element *parent, std::string name);

    void draw(const Clock &clock, nk_context *ctx) override;
};
}
