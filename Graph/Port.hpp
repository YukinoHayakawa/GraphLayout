#pragma once

#include <string>

#include <Usagi/Core/Math.hpp>

namespace usagi
{
struct Node;

struct Port
{
    Node *node = nullptr;
    std::string name;
    enum class Direction : bool
    {
        OUTGOING,
        INCOMING
    } direction;
    Vector2f position;

    Port(std::string name, Direction direction, Vector2f position)
        : name(std::move(name))
        , direction(direction)
        , position(std::move(position))
    {
    }
};

}
