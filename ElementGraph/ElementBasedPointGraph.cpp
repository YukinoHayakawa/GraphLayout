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
        fmt::format("{}", mVertexRoot->childrenCount()),
        position
    );
}

void usagi::ElementBasedPointGraph::addEdge(std::size_t v0, std::size_t v1)
{
    auto c0 = mVertexRoot->childByIndex(v0);
    auto c1 = mVertexRoot->childByIndex(v1);
    mEdgeRoot->addChild<GraphEdge>(
        fmt::format("{}->{}", v0, v1),
        c0->as<GraphVertex>(),
        c1->as<GraphVertex>()
    );
    mEdges.insert({ c0, c1 });
}
