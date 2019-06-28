#pragma once

#include <vector>

#include <Usagi/Core/Math.hpp>
#include <filesystem>

namespace usagi::node_graph
{
struct Port
{
    std::string name;

    enum class Edge
    {
        NORTH,
        SOUTH,
        WEST,
        EAST,
    } edge;

    // 0~1 on the edge
    float edge_pos;

    Port() = default;
    Port(std::string name, Edge edge, float edge_pos);
};

struct NodePrototype
{
    std::string name;
    Vector2f size;
    std::vector<Port> out_ports, in_ports;

    NodePrototype(std::string name, Vector2f size);
    NodePrototype(
        std::string name,
        Vector2f size,
        std::size_t in,
        std::size_t out);

    static void createPorts(
        std::vector<Port> &sink,
        std::size_t amount,
        Port::Edge edge);

    void createOutPorts(std::size_t amount)
    {
        createPorts(out_ports, amount, Port::Edge::EAST);
    }

    void createInPorts(std::size_t amount)
    {
        createPorts(in_ports, amount, Port::Edge::WEST);
    }

    Vector2f portPosition(const Port &p, const Vector2f &position) const;

    const Port & outPort(std::size_t i) const
    {
        return out_ports[i];
    }

    const Port & inPort(std::size_t i) const
    {
        return in_ports[i];
    }
};

struct Node
{
    NodePrototype *prototype = nullptr;
    std::string name;

    Node(NodePrototype *prototype, std::string name);
};

struct Link
{
    std::size_t node0;
    std::size_t port0; // outgoing
    std::size_t node1;
    std::size_t port1; // incoming

    // ctor for emplace_back
    Link(
        std::size_t node0,
        std::size_t port0,
        std::size_t node1,
        std::size_t port1);
};

struct NodeGraph
{
    std::vector<NodePrototype> prototypes;
    std::vector<Node> nodes;
    std::vector<Link> links;
    Vector2f size { 1000, 1000 };

    const Node & node(std::size_t i) const
    {
        return nodes[i];
    }

    const Link & link(std::size_t i) const
    {
        return links[i];
    }

    std::tuple<const Node&, const Port&, const Node&, const Port&>
    mapLink(std::size_t i) const;

    static NodeGraph readFromFile(const std::filesystem::path &path);
};

struct NodeGraphInstance
{
    const NodeGraph *base_graph = nullptr;
    Vector2f *node_positions = nullptr;

    std::tuple<Vector2f, Vector2f> mapLinkEndPoints(std::size_t i) const;

    Vector2f mapNodePosition(std::size_t node_index) const
    {
        return node_positions[node_index];
    }

    AlignedBox2f mapNodeRegion(std::size_t node_index) const;
};
}
