#pragma once

#include <cmath>

namespace usagi
{
struct ForcePropertyTag;

/**
 * \brief Based on:
 * P. Eades. A heuristic for graph drawing.
 * Congressus Numerantium, 42:149–160, 1984.
 */
template <typename Graph, typename GraphTraits = typename Graph::traits_t>
class SimpleSpring
{
    // graph representation
    // node properties - user/algorithm
    // edge properties
    /*
     * If we want efficiency, the locations of properties must be determined
     * on compile time. This lead to data-oriented programming.
     * sparse or dense?
     * if the attributes are needed by the algorithm, it is likely to be dense
     * constant access time it the best - hash map?
     * don't know initial nodes/edges. coming up dynamically/online
     *
     * intrusive/not?
     * data locality?
     * node/edge identity
     *
     */

    Graph &mGraph;
    GraphTraits mTraits;
    float c1 = 2 , c2 = 1, c3 = 1, c4 = 0.1f;

public:
    SimpleSpring(Graph &graph)
        : mGraph(graph)
    {
    }

    // single update iteration
    void update()
    {
        // Algorithm 1: SPRING
        // Input: Graph G
        // Output: Straight-line drawing of G

        auto i = mTraits.vertex_begin(mGraph);
        auto end = mTraits.vertex_end(mGraph);

        using ForceAccessor = typename GraphTraits::template property_accessor
            <ForcePropertyTag>;

        // calculate the force acting on each vertex
        for(; i != end; ++i)
        {
            // each vertex is affected by all other vertices
            typename GraphTraits::vector2_t force { 0, 0 };
            auto other_i = mTraits.vertex_begin(mGraph);
            auto other_end = mTraits.vertex_end(mGraph);
            for(; other_i != other_end; ++other_i)
            {
                if(i == other_i) continue;

                auto &&p0 = mTraits.position(i);
                auto &&p1 = mTraits.position(other_i);
                // adjacent: edge = logarithmic springs
                if(mTraits.is_adjacent(mGraph, i, other_i))
                {
                    // attractive force direction
                    const auto dir = normalized(p1 - p0);
                    const auto l = length(dir);
                    assert(l > 0);
                    force += dir * c1 * std::log(length(dir) / c2);
                }
                // nonadjacent: repulsive force
                else
                {
                    const auto dir = normalized(p0 - p1);
                    const auto l = length(dir);
                    assert(l > 0);
                    force += dir * c3 / std::sqrt(l);
                }
            }
            ForceAccessor force_accessor;
            force_accessor(i) = force;
        }

        // move the vertex c4 ∗ (force on vertex);
        i = mTraits.vertex_begin(mGraph);
        for(; i != end; ++i)
        {
            auto &&p = mTraits.position(i);
            p += c4 * ForceAccessor()(i);
        }

        // draw a filled circle for each vertex;
        // draw a straight-line segment for each edge;
    }
};
}
