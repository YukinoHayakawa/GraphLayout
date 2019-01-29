#pragma once

#include <Usagi/Core/PredefinedElement.hpp>
#include <Usagi/Extension/ImGui/DelegatedImGuiComponent.hpp>

namespace usagi
{
class GraphEditor : public PredefinedElement<DelegatedImGuiComponent>
{
    void drawEditor(const Clock &clock);

public:
    GraphEditor(Element *parent, std::string name);
};
}
