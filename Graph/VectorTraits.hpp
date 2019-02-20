#pragma once

#include <Usagi/Core/Math.hpp>

namespace usagi
{
struct Vector3fTraits
{
    static auto length(const Vector3f &v)
    {
        return v.norm();
    }

    static auto normalized(const Vector3f &v)
    {
        return v.normalized();
    }

    void set_zeros(Vector3f &v) const
    {
        v = Vector3f::Zero();
    }
};
}
