#pragma once

#include <Usagi/Core/Component.hpp>
#include <Usagi/Core/Math.hpp>

namespace usagi
{
struct PositionComponent : Component
{
    Vector3f position = Vector3f::Zero();

    const std::type_info & baseType() override
    {
        return typeid(PositionComponent);
    }
};
}
