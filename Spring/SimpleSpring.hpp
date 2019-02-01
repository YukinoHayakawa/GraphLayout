#pragma once
#include <type_traits>

namespace usagi
{
/**
 * \brief Based on:
 * P. Eades. A heuristic for graph drawing.
 * Congressus Numerantium, 42:149–160, 1984.
 */
template <typename Graph, typename GraphTraits>
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
    using Vertex = typename GraphTraits::Vertex;
    GraphTraits mTraits;
    float c1, c2, c3, c4;

public:
    struct ForcePropertyTag;
    struct PositionPropertyTag;

    SimpleSpring()
    {
        // Initialize Positions: place vertices of G in random locations;
    }

    // single update iteration
    void update()
    {
        // Algorithm 1: SPRING
        // Input: Graph G
        // Output: Straight-line drawing of G

        auto i = mTraits.vertex_begin(mGraph);
        auto end = mTraits.vertex_end(mGraph);

        // calculate the force acting on each vertex
        for(; i != end; ++i)
        {
            // each vertex is affected by all other vertices
            typename GraphTraits::force_t force { };
            auto other_i = mTraits.vertex_begin(mGraph);
            auto other_end = mTraits.vertex_end(mGraph);
            for(; other_i != other_end; ++other_i)
            {
                auto &&p0 = mTraits.position(i);
                auto &&p1 = mTraits.position(other_i);
                // adjacent: edge = logarithmic springs
                if(mTraits.is_adjancent(i, other_i))
                {
                    // attractive force direction
                    auto dir = p1 - p0;
                    force += c1 * log(length(dir) / c2);
                }
                // nonadjacent: repulsive force
                else
                {
                    auto dir = p0 - p1;
                    force += c3 / sqrt(length(dir));
                }
            }
            typename GraphTraits::template property_accessor
                <ForcePropertyTag>()(i) = force;
        }

        // move the vertex c4 ∗ (force on vertex);
        i = mTraits.vertex_begin(mGraph);
        for(; i != end; ++i)
        {
            auto &&p = mTraits.position(i);
            p += c4 * typename GraphTraits::template property_accessor
                <ForcePropertyTag>()(i);
        }

        // draw a filled circle for each vertex;
        // draw a straight-line segment for each edge;
    }
};
}
