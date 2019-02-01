#pragma once

#include <Usagi/Core/PredefinedElement.hpp>
#include <Usagi/Extension/ImGui/DelegatedImGuiComponent.hpp>

#include "../GraphLayout/Graph/SimplePointGraph.hpp"
#include "../GraphLayout/Spring/SimpleSpring.hpp"

namespace usagi
{
class GraphEditor : public PredefinedElement<DelegatedImGuiComponent>
{
    SimplePointGraph mGraph;
    float mVertexRadius = 10;
    float mVertexColor[4] = { 1, 1, 1, 1 };
    float mEdgeColor[4] = { 1, 1, 0.5, 1 };
    int mVertexCount = 20;
    int mEdgeCount = 5;
    bool mUpdate = false;
    SimpleSpring<SimplePointGraph> mLayout { mGraph };

    void drawEditor(const Clock &clock);
    void generateGraph();

public:
    GraphEditor(Element *parent, std::string name);

};
}
