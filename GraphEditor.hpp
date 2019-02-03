#pragma once

#include <random>

#include <Usagi/Core/PredefinedElement.hpp>
#include <Usagi/Extension/ImGui/DelegatedImGuiComponent.hpp>

#include "../GraphLayout/Graph/SimplePointGraph.hpp"
#include "../GraphLayout/Spring/SimpleSpring.hpp"

namespace usagi
{
class GraphEditor : public PredefinedElement<
    DelegatedImGuiComponent
>
{
    SimplePointGraph mGraph;
    float mVertexRadius = 10;
    float mVertexColor[4] = { 1, 1, 1, 1 };
    float mEdgeColor[4] = { 1, 1, 0.5, 1 };
    int mVertexCount = 20;
    int mEdgeCount = 5;
    bool mUpdate = false;
    float mScale = 100.f;
    float mEdgeConnectP = 0.5;
    std::mt19937 mRng;
    std::uniform_real_distribution<float> mD { 0, 1 };

    Vector2f mOffset = { 1920, 1080 };
    SimpleSpring<SimplePointGraph> mLayout { mGraph };

    void drawEditor(const Clock &clock);
    void generateGraph();

public:
    GraphEditor(Element *parent, std::string name);

};
}
