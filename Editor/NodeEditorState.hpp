#pragma once

#include <Usagi/Graphics/Game/GraphicalGameState.hpp>

namespace usagi
{
class NodeEditorState : public GraphicalGameState
{
public:
    NodeEditorState(
        Element *parent,
        std::string name,
        GraphicalGame *game);
};
}
