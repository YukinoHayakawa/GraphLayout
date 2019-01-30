#pragma once

#include <Usagi/Core/PredefinedElement.hpp>
#include <Usagi/Extension/ImGui/DelegatedImGuiComponent.hpp>

#include "../GraphLayout/Graph/SimplePointGraph.hpp"

namespace usagi
{
class GraphEditor : public PredefinedElement<DelegatedImGuiComponent>
{
    SimplePointGraph mGraph;

    void drawEditor(const Clock &clock);

public:
    GraphEditor(Element *parent, std::string name);
};
}
