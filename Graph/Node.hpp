#pragma once

#include <vector>

#include <Usagi/Core/Math.hpp>

namespace usagi
{
struct Port;

struct Node
{
    std::vector<Port> ports;
    // std::vector<Port> output_ports;
    Eigen::AlignedBox2f rect;
    Vector2f center;

    void addPort(Port p);
};
}
