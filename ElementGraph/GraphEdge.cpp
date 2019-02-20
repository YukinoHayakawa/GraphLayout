#include "GraphEdge.hpp"

#include "GraphVertex.hpp"

usagi::GraphEdge::GraphEdge(
    Element *parent,
    std::string name,
    GraphVertex *v0,
    GraphVertex *v1)
    : PredefinedElement(parent, std::move(name))
    , v0(v0)
    , v1(v1)
{
    addComponent(this);
}

void usagi::GraphEdge::draw(dd::ContextHandle ctx)
{
    float color[] = { 1, 1, 0 };
    dd::line(ctx,
        v0->comp<PositionComponent>()->position.data(),
        v1->comp<PositionComponent>()->position.data(),
        color
    );
}
