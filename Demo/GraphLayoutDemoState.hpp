#pragma once

#include <Usagi/Graphics/Game/GraphicalGameState.hpp>
#include <Usagi/Camera/Controller/CameraMan.hpp>
#include <Usagi/Extension/DebugDraw/DebugDrawComponent.hpp>

namespace usagi
{
class RayCastSystem;
class InputMapping;
class PerspectiveCamera;
class ModelViewCameraController;

class GraphLayoutDemoState
    : public GraphicalGameState
    , public DebugDrawComponent
{
    using ModelCameraMan =
        CameraMan<PerspectiveCamera, ModelViewCameraController>;
    ModelCameraMan *mCameraElement = nullptr;
    InputMapping *mInputMapping = nullptr;
    RayCastSystem *mRayCast = nullptr;

public:
    GraphLayoutDemoState(
        Element *parent,
        std::string name,
        GraphicalGame *game);

    void update(const Clock &clock) override;
    void resume() override;
    void draw(dd::ContextHandle ctx) override;
};
}
