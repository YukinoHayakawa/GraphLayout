#pragma once

#include <vector>

#include <Usagi/Core/Math.hpp>

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

	NodePrototype(
		std::string name,
		Vector2f size,
		std::size_t in,
		std::size_t out)
		: NodePrototype(name, size)
	{
		createInPorts(in);
		createOutPorts(out);
	}

	static void createPorts(
		std::vector<Port> &sink,
		std::size_t amount,
		Port::Edge edge)
	{
		sink.resize(amount);
		const float step = 1.f / (amount + 1);
		for(std::size_t i = 0; i < amount; ++i)
		{
			sink[i].edge = edge;
			sink[i].edge_pos = step * (i + 1);
		}
	}

	void createOutPorts(std::size_t amount)
	{
		createPorts(out_ports, amount, Port::Edge::EAST);
	}

	void createInPorts(std::size_t amount)
	{
		createPorts(in_ports, amount, Port::Edge::WEST);
	}

	Vector2f portPosition(const Port &p, const Vector2f &position) const
	{
		Vector2f pos;
		switch(p.edge)
		{
			case Port::Edge::NORTH:
				pos.x() = lerp(p.edge_pos, 0.f, size.x())
					+ position.x();
				pos.y() = position.y();
				break;
			case Port::Edge::SOUTH:
				pos.x() = lerp(p.edge_pos, 0.f, size.x())
					+ position.x();
				pos.y() = position.y() + size.y();
				break;
			case Port::Edge::WEST:
				pos.x() = position.x();
				pos.y() = lerp(p.edge_pos, 0.f, size.y())
					+ position.y();
				break;
			case Port::Edge::EAST:
				pos.x() = position.x() + size.x();
				pos.y() = lerp(p.edge_pos, 0.f, size.y())
					+ position.y();
				break;
			default: ;
		}
		return pos;
	}

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

	Node(NodePrototype *prototype)
		: prototype(prototype)
	{
	}
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
		std::size_t port1)
		: node0(node0)
		, port0(port0)
		, node1(node1)
		, port1(port1)
	{
	}
};

struct NodeGraph
{
	std::vector<Node> nodes;
	std::vector<Link> links;

	const Node & node(std::size_t i) const
	{
		return nodes[i];
	}

	const Link & link(std::size_t i) const
	{
		return links[i];
	}

	std::tuple<const Node&, const Port&, const Node&, const Port&>
	mapLink(std::size_t i) const
	{
		auto &l = link(i);
		auto &n0 = node(l.node0);
		auto &n1 = node(l.node1);
		auto &p0 = n0.prototype->outPort(l.port0);
		auto &p1 = n1.prototype->inPort(l.port1);
		return std::make_tuple(
			std::ref(n0), std::ref(p0),
			std::ref(n1), std::ref(p1)
		);
	}
};

struct PortGraph
{
	const NodeGraph *base_graph = nullptr;
	Vector2f *node_positions = nullptr;

	std::tuple<Vector2f, Vector2f> mapLinkEndPoints(std::size_t i) const
	{
		auto [n0, p0, n1, p1] = base_graph->mapLink(i);
		auto &l = base_graph->link(i);
		auto pos0 = n0.prototype->portPosition(p0, node_positions[l.node0]);
		auto pos1 = n1.prototype->portPosition(p1, node_positions[l.node1]);
		return { pos0, pos1 };
	}

	Vector2f mapNodePosition(std::size_t node_index) const
	{
		return node_positions[node_index];
	}

	AlignedBox2f mapNodeRegion(std::size_t node_index) const
	{
		const auto node_position = mapNodePosition(node_index);
		return {
			node_position,
			node_position + base_graph->node(node_index).prototype->size
		};
	}
};
}
