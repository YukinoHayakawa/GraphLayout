#pragma once

#include <Usagi/Core/Component.hpp>

namespace usagi
{
// tags graph vertices which are processed by graph layout algorithms
struct GraphVertexComponent : Component
{
    const std::type_info & baseType() override
    {
        return typeid(GraphVertexComponent);
    }
};
}
