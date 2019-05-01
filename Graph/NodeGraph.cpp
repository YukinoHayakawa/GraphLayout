#include "NodeGraph.hpp"

#include <fstream>

#include <Usagi/Core/Logging.hpp>

usagi::node_graph::Port::Port(std::string name, Edge edge, float edge_pos)
	: name(std::move(name))
	, edge(edge)
	, edge_pos(edge_pos)
{
}

usagi::node_graph::NodePrototype::NodePrototype(std::string name, Vector2f size)
	: name(std::move(name))
	, size(std::move(size))
{
}

usagi::node_graph::NodePrototype::NodePrototype(
	std::string name,
	Vector2f size,
	std::size_t in,
	std::size_t out)
	: NodePrototype(name, size)
{
	createInPorts(in);
	createOutPorts(out);
}

void usagi::node_graph::NodePrototype::createPorts(
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

usagi::Vector2f usagi::node_graph::NodePrototype::portPosition(
	const Port &p,
	const Vector2f &position) const
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

usagi::node_graph::Node::Node(NodePrototype *prototype, std::string name)
	: prototype(prototype)
	, name(std::move(name))
{
}

usagi::node_graph::Link::Link(
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

std::tuple<const usagi::node_graph::Node &, const usagi::node_graph::Port &,
	const usagi::node_graph::Node &, const usagi::node_graph::Port &> usagi::
node_graph::NodeGraph::mapLink(std::size_t i) const
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

usagi::node_graph::NodeGraph usagi::node_graph::NodeGraph::readFromFile(
	const std::filesystem::path &path)
{
	std::ifstream in { path };
	std::string buf;
	in.exceptions(std::ios::badbit);
	NodeGraph g;

	while(in >> buf)
	{
		// ignore comment lines
		if(buf == "#")
		{
			std::getline(in, buf);
		}
		else if(buf == "canvas")
		{
			in >> g.size.x() >> g.size.y();
			LOG(info, "Canvas Size: {}, {}", g.size.x(), g.size.y());
		}
		else if(buf == "proto")
		{
			std::size_t id;
			std::string display_name;
			Vector2f size;
			std::size_t in_pins, out_pins;
			in >> id >> std::quoted(display_name)
				>> size.x() >> size.y()
				>> in_pins >> out_pins;
			assert(g.prototypes.size() == id);
			g.prototypes.emplace_back(
				display_name, size, in_pins, out_pins);
			LOG(info, "Node Prototype: #{} ({}) <{}, {}> {}:{}",
				id, display_name, size.x(), size.y(), in_pins, out_pins
			);
		}
		else if(buf == "node")
		{
			std::size_t id;
			std::size_t proto;
			std::string name;
			in >> id >> proto >> std::quoted(name);
			assert(g.nodes.size() == id);
			assert(proto < g.prototypes.size());
			g.nodes.emplace_back(&g.prototypes[proto], name);
			LOG(info, "Node: #{}({}) <- {}",
				id, name, g.prototypes[proto].name
			);
		}
		else if(buf == "link")
		{
			std::size_t out_node, out_pin, in_node, in_pin;
			in >> out_node >> out_pin >> in_node >> in_pin;
			assert(out_node < g.nodes.size());
			assert(out_pin < g.nodes[out_node].prototype->out_ports.size());
			assert(in_node < g.nodes.size());
			assert(in_pin < g.nodes[in_node].prototype->in_ports.size());
			g.links.emplace_back(out_node, out_pin, in_node, in_pin);
			LOG(info, "Link: {}:{}->{}:{}",
				out_node, out_pin, in_node, in_pin
			);
		}
		else
		{
			assert(false);
		}
	}

	return g;
}

std::tuple<usagi::Vector2f, usagi::Vector2f> usagi::node_graph::NodeGraphInstance::
mapLinkEndPoints(std::size_t i) const
{
	auto [n0, p0, n1, p1] = base_graph->mapLink(i);
	auto &l = base_graph->link(i);
	auto pos0 = n0.prototype->portPosition(p0, node_positions[l.node0]);
	auto pos1 = n1.prototype->portPosition(p1, node_positions[l.node1]);
	return { pos0, pos1 };
}

usagi::AlignedBox2f usagi::node_graph::NodeGraphInstance::mapNodeRegion(
	std::size_t node_index) const
{
	const auto node_position = mapNodePosition(node_index);
	return {
		node_position,
		node_position + base_graph->node(node_index).prototype->size
	};
}
