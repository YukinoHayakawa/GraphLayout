#pragma once

#include <vector>

#include "Node.hpp"
#include "Port.hpp"

namespace usagi
{
class Pin;

struct NodeGraph
{
    struct Link
    {
        std::size_t node0;
        std::size_t port0;
        std::size_t node1;
        std::size_t port1;
    };
    std::vector<Node> nodes;
    std::vector<Link> links;
};
}
