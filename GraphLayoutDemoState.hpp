#pragma once

#include <Usagi/Graphics/Game/GraphicalGameState.hpp>
#include <Usagi/Camera/Controller/CameraMan.hpp>

namespace usagi
{
class InputMapping;
class PerspectiveCamera;
class ModelViewCameraController;

class GraphLayoutDemoState : public GraphicalGameState
{
    using ModelCameraMan =
        CameraMan<PerspectiveCamera, ModelViewCameraController>;
    ModelCameraMan *mCameraElement = nullptr;
    InputMapping *mInputMapping = nullptr;

public:
    GraphLayoutDemoState(
        Element *parent,
        std::string name,
        GraphicalGame *game);

    void update(const Clock &clock) override;
    void resume() override;
};
}
