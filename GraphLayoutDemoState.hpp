#pragma once

#include <Usagi/Graphics/Game/GraphicalGameState.hpp>

namespace usagi
{
class GraphLayoutDemoState : public GraphicalGameState
{
public:
    GraphLayoutDemoState(
        Element *parent,
        std::string name,
        GraphicalGame *game);
};
}
