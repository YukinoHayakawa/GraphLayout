#include "Node.hpp"

#include "Port.hpp"

void usagi::Node::addPort(Port p)
{
    p.node = this;
    ports.push_back(std::move(p));
}
