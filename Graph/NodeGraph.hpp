#pragma once

#include <vector>

#include <Usagi/Core/Math.hpp>

namespace usagi
{
struct Port
{
    std::string name;

    // enum class Direction : bool
    // {
    //     OUTGOING,
    //     INCOMING
    // } direction;

    enum class Edge
    {
        NORTH,
        SOUTH,
        WEST,
        EAST,
    } edge;

    // 0~1 on the edge
    float edge_pos;

    Port(std::string name, Edge edge, float edge_pos)
        : name(std::move(name))
        , edge(edge)
        , edge_pos(edge_pos)
    {
    }
};

struct NodePrototype
{
    std::string name;
    Vector2f size;
    std::vector<Port> out_ports, in_ports;

    NodePrototype(std::string name, Vector2f size)
        : name(std::move(name))
        , size(std::move(size))
    {
    }
};

template <
	typename PositionGetter
>
struct Node
{
    NodePrototype *prototype = nullptr;

    Node(NodePrototype *prototype)
	    : prototype(prototype)
    {
    }

    Vector2f portPosition(const Port &p, const Vector2f &position) const
    {
        Vector2f pos;
        switch(p.edge)
        {
            case Port::Edge::NORTH:
                pos.x() = lerp(p.edge_pos, 0.f, prototype->size.x())
                    + position.x();
                pos.y() = position.y();
                break;
            case Port::Edge::SOUTH:
                pos.x() = lerp(p.edge_pos, 0.f, prototype->size.x())
                    + position.x();
                pos.y() = position.y() + prototype->size.y();
                break;
            case Port::Edge::WEST:
                pos.x() = position.x();
                pos.y() = lerp(p.edge_pos, 0.f, prototype->size.y())
                    + position.y();
                break;
            case Port::Edge::EAST:
                pos.x() = position.x() + prototype->size.x();
                pos.y() = lerp(p.edge_pos, 0.f, prototype->size.y())
                    + position.y();
                break;
            default: ;
        }
        return pos;
    }
};

template <
	typename Node
>
struct NodeGraph
{
    struct Link
    {
        std::size_t node0;
        std::size_t port0;
        std::size_t node1;
        std::size_t port1;

        Link(
            std::size_t node0,
            std::size_t port0,
            std::size_t node1,
            std::size_t port1)
            : node0(node0)
            , port0(port0)
            , node1(node1)
            , port1(port1)
        {
        }
    };

    std::vector<Node> nodes;
    std::vector<Link> links;
};
}
