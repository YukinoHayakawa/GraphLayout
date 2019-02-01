#pragma once

#include <Usagi/Core/Math.hpp>

struct SimplePointGraph
{
    std::vector<usagi::Vector2f> vertices;
    std::vector<std::pair<size_t, size_t>> edges;
};
