#pragma once

#include <Usagi/Core/Element.hpp>
#include <Usagi/Extension/ImGui/ImGuiComponent.hpp>
#include <GraphLayout/Graph/NodeGraph.hpp>

namespace usagi
{
class PortBasedGraph
    : public Element
    , public ImGuiComponent
{
    NodeGraph mGraph;

public:
    PortBasedGraph(Element *parent, std::string name);

    void draw(const Clock &clock) override;
};
}
