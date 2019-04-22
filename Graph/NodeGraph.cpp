#include "NodeGraph.hpp"

#include <fstream>

#include <Usagi/Core/Logging.hpp>

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
