#pragma once

#include <Usagi/Graphics/Game/GraphicalGameState.hpp>

namespace usagi
{
class InputMapping;

class GraphLayoutDemoState : public GraphicalGameState
{
    InputMapping *mInputMapping = nullptr;

public:
    GraphLayoutDemoState(
        Element *parent,
        std::string name,
        GraphicalGame *game);

    void resume() override;
};
}
