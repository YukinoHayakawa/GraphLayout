#include "GraphVertex.hpp"

#include <Usagi/Geometry/Shape/Common/Sphere.hpp>

void usagi::GraphVertex::draw(dd::ContextHandle ctx)
{
    float color[] = { 1, 1, 1 };
    dd::point(ctx,
        comp<PositionComponent>()->position.data(),
        color,
        10
    );
}

usagi::GraphVertex::GraphVertex(
    Element *parent,
    std::string name,
    const Vector3f &position)
    : PredefinedElement(parent, std::move(name))
{
    comp<PositionComponent>()->position = position;
    comp<ShapeComponent>()->shape = std::make_shared<Sphere>(position, 0.25f);

    addComponent(this);
}
