#pragma once

#include <Usagi/Core/PredefinedElement.hpp>
#include <Usagi/Extension/DebugDraw/DebugDrawComponent.hpp>

namespace usagi
{
class GraphVertex;

class GraphEdge
    : public PredefinedElement<
    >, public DebugDrawComponent
{
public:
    GraphVertex *const v0 = nullptr;
    GraphVertex *const v1 = nullptr;

    GraphEdge(
        Element *parent,
        std::string name,
        GraphVertex *v0,
        GraphVertex *v1);

    void draw(dd::ContextHandle ctx) override;
};
}
