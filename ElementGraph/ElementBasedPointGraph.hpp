#pragma once

#include <set>

#include <Usagi/Core/Element.hpp>
#include <Usagi/Core/Math.hpp>

#include <GraphLayout/Graph/VectorTraits.hpp>

#include "GraphVertex.hpp"
#include "PositionComponent.hpp"

namespace usagi
{
class ElementBasedPointGraph : public Element
{
    Element *mVertexRoot = nullptr;
    Element *mEdgeRoot = nullptr;
    std::set<std::pair<Element*, Element*>> mEdges;

public:
    ElementBasedPointGraph(Element *parent, std::string name);

    void addVertex(const Vector3f &position);
    void addEdge(std::size_t v0, std::size_t v1);

    using traits_t = struct Traits;

    struct Traits
    {
        using vector_t = Vector3f;
        using vector_traits_t = Vector3fTraits;
        using vertex_iterator_t = decltype(mVertexRoot->childrenBegin());

        static auto vertex_begin(ElementBasedPointGraph *g)
        {
            return g->mVertexRoot->childrenBegin();
        }

        static auto vertex_end(ElementBasedPointGraph *g)
        {
            return g->mVertexRoot->childrenEnd();
        }

        static auto & position(const vertex_iterator_t &v)
        {
            return v->get()->as<GraphVertex>()->comp<PositionComponent>()->
                position;
        }

        static bool is_adjacent(
            ElementBasedPointGraph *g,
            const vertex_iterator_t &v0,
            const vertex_iterator_t &v1)
        {
            // todo c++20 contains
            return g->mEdges.count({ v0->get(), v1->get() })
                || g->mEdges.count({ v1->get(), v0->get() });
        }

        template <typename PropertyTag>
        struct property_accessor;
    };
};
}
