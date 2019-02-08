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
struct SimpleSpring
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

    // spring force constant
    float c1 = 2;
    // original (unloaded/stretched/compressed) spring length
    float c2 = 1;
    // repel factor
    float c3 = 1;
    // update rate
    float c4 = 0.1f;

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
            typename GraphTraits::vector_t force;
            set_zeros(force);
            auto other_i = mTraits.vertex_begin(mGraph);
            auto other_end = mTraits.vertex_end(mGraph);
            for(; other_i != other_end; ++other_i)
            {
                if(i == other_i) continue;

                auto &&p0 = mTraits.position(i);
                auto &&p1 = mTraits.position(other_i);
                // attractive force direction
                const auto dist = p1 - p0;
                const auto dir = normalized(dist);
                const auto l = length(dist);
                assert(l > 0);

                // adjacent: edge = logarithmic springs
                if(mTraits.is_adjacent(mGraph, i, other_i))
                {
                    // log(x) > 0 when x > 1 and < 0 when 0 < x < 1.
                    // therefore, when l (real spring length is longer than
                    // c2 (steady spring length, the force is attractive.
                    // otherwise it is repulsive.
                    force += dir * c1 * std::log(l / c2);
                }
                // nonadjacent: repulsive force (always)
                else
                {
                    force -= dir * c3 / std::sqrt(l);
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
