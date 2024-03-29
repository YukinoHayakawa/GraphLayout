﻿#pragma once

#include <random>

#include <Usagi/Core/PredefinedElement.hpp>
#include <Usagi/Extension/ImGui/DelegatedImGuiComponent.hpp>
#include <Usagi/Extension/DebugDraw/DebugDrawComponent.hpp>
#include <Usagi/Interactive/InputComponent.hpp>

#include <GraphLayout/Spring/SimpleSpring.hpp>
#include <GraphLayoutDemo/ElementGraph/ElementBasedPointGraph.hpp>

namespace usagi
{
template <>
struct ElementBasedPointGraph::Traits::property_accessor<ForcePropertyTag>
{
    std::unordered_map<Element*, Vector3f> forces;

    auto & operator()(vertex_iterator_t i)
    {
        return forces[i->get()];
    }
};

class GraphEditor
    : public PredefinedElement<
        DelegatedImGuiComponent,
        InputComponent
    >, public DebugDrawComponent
{
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
    ElementBasedPointGraph *mGraph = nullptr;
    using LayoutAlgorithm = SimpleSpring<ElementBasedPointGraph>;
    LayoutAlgorithm::Parameters mParameters;
    LayoutAlgorithm mLayout;

    void drawEditor(const Clock &clock);
    void generateGraph();

public:
    GraphEditor(Element *parent, std::string name);

    void draw(dd::ContextHandle ctx) override;
};
}
