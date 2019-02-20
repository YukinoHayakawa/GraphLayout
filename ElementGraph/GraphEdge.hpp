#pragma once

#include <Usagi/Core/PredefinedElement.hpp>

namespace usagi
{
class GraphVertex;

class GraphEdge : public PredefinedElement<>
{
public:
    const GraphVertex *v0 = nullptr, *v1 = nullptr;

    GraphEdge(
        Element *parent,
        std::string name,
        const GraphVertex *v0,
        const GraphVertex *v1)
        : PredefinedElement(parent, std::move(name))
        , v0(v0)
        , v1(v1)
    {
    }
};
}
