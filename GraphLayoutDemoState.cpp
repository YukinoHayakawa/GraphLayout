#include "GraphLayoutDemoState.hpp"

#include <Usagi/Camera/PerspectiveCamera.hpp>
#include <Usagi/Extension/DebugDraw/DebugDrawSubsystem.hpp>
#include <Usagi/Extension/ImGui/ImGuiSubsystem.hpp>
#include <Usagi/Graphics/Game/GraphicalGame.hpp>
#include <Usagi/Interactive/InputMapping.hpp>
#include <Usagi/Interactive/InputSubsystem.hpp>
#include <Usagi/Runtime/Input/InputManager.hpp>
#include <Usagi/Runtime/Input/Keyboard/Keyboard.hpp>
#include <Usagi/Runtime/Input/Mouse/Mouse.hpp>
#include <Usagi/Runtime/Runtime.hpp>
#include <Usagi/Transform/TransformComponent.hpp>
#include <Usagi/Utility/Functional.hpp>
#include <Usagi/Camera/Controller/ModelViewCameraController.hpp>
#include <Usagi/Runtime/Graphics/Swapchain.hpp>
#include <Usagi/Runtime/Window/Window.hpp>
#include <Usagi/Camera/CameraSample.hpp>
#include <Usagi/Geometry/RayCastSubsystem.hpp>

#include "GraphEditor.hpp"

usagi::GraphLayoutDemoState::GraphLayoutDemoState(
    Element *parent,
    std::string name,
    GraphicalGame *game)
    : GraphicalGameState(parent, std::move(name), game)
{
    const auto input_manager = mGame->runtime()->inputManager();

    // debug draw
    const auto dd = addSubsystem<DebugDrawSubsystem>(
        "DebugDrawRender",
        game
    );
    dd->setSizeFunctionsFromRenderWindow(mGame->mainWindow());
    dd->setWorldToNdcFunc([=]() {
        // world -> camera local -> NDC
        return mCameraElement->camera()->localToNDC() *
            mCameraElement->comp<TransformComponent>()->worldToLocal();
    });

    // imgui
    const auto imgui = addSubsystem<ImGuiSubsystem>(
        "ImGuiRender",
        mGame,
        mGame->mainWindow()->window,
        input_manager->virtualKeyboard(),
        input_manager->virtualMouse()
    );
    imgui->setSizeFunctionsFromRenderWindow(mGame->mainWindow());

    // input mapping
    mInputMapping = addChild<InputMapping>("InputMapping");
    input_manager->virtualMouse()->addEventListener(mInputMapping);
    input_manager->virtualKeyboard()->addEventListener(mInputMapping);
    addSubsystem<InputSubsystem>("Input", mInputMapping);

    // camera
    mCameraElement = addChild<ModelCameraMan>(
        "Camera",
        std::make_shared<PerspectiveCamera>(),
        std::make_shared<ModelViewCameraController>(
            Vector3f::Zero(), 10.f
        ));

    auto cam_actions = mInputMapping->addActionGroup("Camera");
    cam_actions->setAnalogAction2DHandler("Rotate", partial_apply(
        &ModelViewCameraController::rotate,
        mCameraElement->cameraController()));
    cam_actions->bindMouseRelativeMovement("Rotate");

    // ray cast
    mRayCast = addSubsystem<RayCastSubsystem>("RayCast");

    // graph editor
    mInputMapping->addActionGroup("GraphEditorUI");
    addChild<GraphEditor>("GraphEditor");

    addComponent(this);
}

void usagi::GraphLayoutDemoState::update(const Clock &clock)
{
    // todo better way?
    mCameraElement->camera()->setPerspective(
        mGame->mainWindow()->window->size().cast<float>(),
        degreesToRadians(90.f),
        mGame->mainWindow()->swapchain->aspectRatio(), 0.5f, 100.f);

    GraphicalGameState::update(clock);
}

void usagi::GraphLayoutDemoState::resume()
{
    mInputMapping->actionGroup("GraphEditorUI")->activate();
    // mInputMapping->actionGroup("Camera")->activate();
}

void usagi::GraphLayoutDemoState::draw(dd::ContextHandle ctx)
{
    auto cur = mGame->runtime()->inputManager()->virtualMouse()->
        cursorPositionInActiveWindow();
    CameraSample s;
    s.screen = cur;
    auto ray =
        mCameraElement->cameraController()->transform()->localToWorld() *
         mCameraElement->camera()->generateRay(s);
    auto x = mRayCast->intersect(ray);
    if(x.has_value())
    {
        float color[] = { 1.f, 0.f, 0.f };
        dd::sphere(ctx, x.value().position.data(), color, 0.1f);
    }
}
