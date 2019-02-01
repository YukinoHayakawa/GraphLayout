#pragma once

#include <set>

#include <Usagi/Core/Math.hpp>

namespace usagi
{
struct SimplePointGraphTraits;

struct SimplePointGraph
{
    struct Vertex
    {
        Vector2f position;
        Vector2f force;
    };

    using vertex_container_t = std::vector<Vertex>;
    vertex_container_t vertices;
    std::set<std::pair<size_t, size_t>> edges;

    using traits_t = SimplePointGraphTraits;
};

struct SimplePointGraphTraits
{
    static auto vertex_begin(SimplePointGraph &g)
    {
        return g.vertices.begin();
    }

    static auto vertex_end(SimplePointGraph &g)
    {
        return g.vertices.end();
    }

    static auto & position(
        const SimplePointGraph::vertex_container_t::iterator &v)
    {
        return v->position;
    }

    static bool is_adjacent(
        SimplePointGraph &g,
        const SimplePointGraph::vertex_container_t::iterator &v0,
        const SimplePointGraph::vertex_container_t::iterator &v1)
    {
        const auto pos0 = v0 - g.vertices.begin();
        const auto pos1 = v1 - g.vertices.begin();
        return g.edges.count({ pos0, pos1 })
            || g.edges.count({ pos1, pos0 });
    }

    template <typename PropertyTag>
    struct property_accessor;

    using vector2_t = Vector2f;
};

template <>
struct SimplePointGraphTraits::property_accessor<struct ForcePropertyTag>
{
    auto & operator()(
        const SimplePointGraph::vertex_container_t::iterator &v) const
    {
        return v->force;
    }
};

inline auto length(const Vector2f &v)
{
    return v.norm();
}

inline auto normalized(const Vector2f &v)
{
    return v.normalized();
}
}
