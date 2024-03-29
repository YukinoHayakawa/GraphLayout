﻿#pragma once

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

    using vector_t = typename GraphTraits::vector_t;
    using vector_traits_t = typename GraphTraits::vector_traits_t;
    using ForceAccessor = typename GraphTraits::template property_accessor
        <ForcePropertyTag>;

    Graph *mGraph;
    GraphTraits mTraits;
    ForceAccessor force_accessor;

    struct Parameters
    {
        // spring force constant
        float c1 = 2;
        // original (unloaded/stretched/compressed) spring length
        float c2 = 1;
        // repel factor
        float c3 = 1;
        // update rate
        float c4 = 0.1f;
    } *param = nullptr;

    SimpleSpring() = default;

    SimpleSpring(
        Graph *graph,
        Parameters *param)
        : mGraph(graph)
        , param(param)
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

        // calculate the force acting on each vertex
        for(; i != end; ++i)
        {
            // each vertex is affected by all other vertices
            vector_t force;
            vector_traits_t().set_zeros(force);
            auto other_i = mTraits.vertex_begin(mGraph);
            auto other_end = mTraits.vertex_end(mGraph);
            for(; other_i != other_end; ++other_i)
            {
                if(i == other_i) continue;

                auto &&p0 = mTraits.position(i);
                auto &&p1 = mTraits.position(other_i);
                // attractive force direction
                const auto dist = p1 - p0;
                const auto dir = vector_traits_t().normalized(dist);
                const auto l = vector_traits_t().length(dist);
                assert(l > 0);

                // adjacent: edge = logarithmic springs
                if(mTraits.is_adjacent(mGraph, i, other_i))
                {
                    // log(x) > 0 when x > 1 and < 0 when 0 < x < 1.
                    // therefore, when l (real spring length is longer than
                    // c2 (steady spring length, the force is attractive.
                    // otherwise it is repulsive.
                    force += dir * param->c1 * std::log(l / param->c2);
                }
                // nonadjacent: repulsive force (always)
                else
                {
                    force -= dir * param->c3 / std::sqrt(l);
                }
            }
            force_accessor(i) = force;
        }

        // move the vertex c4 ∗ (force on vertex);
        i = mTraits.vertex_begin(mGraph);
        for(; i != end; ++i)
        {
            auto &&p = mTraits.position(i);
            p += param->c4 * force_accessor(i);
        }

        // draw a filled circle for each vertex;
        // draw a straight-line segment for each edge;
    }
};
}
