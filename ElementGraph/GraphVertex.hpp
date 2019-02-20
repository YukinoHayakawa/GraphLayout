#pragma once

#include <Usagi/Core/PredefinedElement.hpp>
#include <Usagi/Extension/DebugDraw/DelegatedDebugDrawComponent.hpp>
#include <Usagi/Geometry/ShapeComponent.hpp>
#include <Usagi/Geometry/RayCastComponent.hpp>

#include "PositionComponent.hpp"
#include "GraphVertexComponent.hpp"

namespace usagi
{
class GraphVertex
    : public PredefinedElement<
        ShapeComponent,
        RayCastComponent,
        PositionComponent,
        GraphVertexComponent,
        DelegatedDebugDrawComponent
    >
{
    void draw(dd::ContextHandle ctx);

public:
    GraphVertex(Element *parent, std::string name, const Vector3f &position);
};
}
