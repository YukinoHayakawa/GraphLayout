#include "ElementBasedPointGraph.hpp"

#include <fmt/format.h>

#include "GraphEdge.hpp"

usagi::ElementBasedPointGraph::ElementBasedPointGraph(
    Element *parent,
    std::string name)
    : Element(parent, std::move(name))
{
    mVertexRoot = addChild("Vertices");
    mEdgeRoot = addChild("Edges");
}

void usagi::ElementBasedPointGraph::addVertex(const Vector3f &position)
{
    mVertexRoot->addChild<GraphVertex>(
        fmt::format("{}", mChildren.size()),
        position
    );
}

void usagi::ElementBasedPointGraph::addEdge(std::size_t v0, std::size_t v1)
{
    mEdgeRoot->addChild<GraphEdge>(
        fmt::format("{}-{}", v0, v1),
        mChildren[v0]->as<GraphVertex>(),
        mChildren[v1]->as<GraphVertex>()
    );
    mEdges.insert({ mChildren[v0].get(), mChildren[v1].get() });
}
