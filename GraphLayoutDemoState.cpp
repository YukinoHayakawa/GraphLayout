#include "GraphLayoutDemoState.hpp"

#include <Usagi/Extension/ImGui/ImGuiSubsystem.hpp>
#include <Usagi/Graphics/Game/GraphicalGame.hpp>
#include <Usagi/Interactive/InputSubsystem.hpp>
#include <Usagi/Interactive/InputMapping.hpp>
#include <Usagi/Runtime/Input/InputManager.hpp>
#include <Usagi/Runtime/Input/Keyboard/Keyboard.hpp>
#include <Usagi/Runtime/Input/Mouse/Mouse.hpp>
#include <Usagi/Runtime/Runtime.hpp>

#include "GraphEditor.hpp"

usagi::GraphLayoutDemoState::GraphLayoutDemoState(
    Element *parent,
    std::string name,
    GraphicalGame *game)
    : GraphicalGameState(parent, std::move(name), game)
{
    const auto input_manager = mGame->runtime()->inputManager();
    const auto imgui = addSubsystem<ImGuiSubsystem>(
        "imgui",
        mGame,
        mGame->mainWindow()->window,
        input_manager->virtualKeyboard(),
        input_manager->virtualMouse()
    );
    imgui->setSizeFunctionsFromRenderWindow(mGame->mainWindow());

    mInputMapping = addChild<InputMapping>("InputMapping");
    input_manager->virtualMouse()->addEventListener(mInputMapping);
    input_manager->virtualKeyboard()->addEventListener(mInputMapping);
    mInputMapping->addActionGroup("GraphEditorUI");
    addSubsystem<InputSubsystem>("input", mInputMapping);

    addChild<GraphEditor>("GraphEditor");
}

void usagi::GraphLayoutDemoState::resume()
{
    mInputMapping->actionGroup("GraphEditorUI")->activate();
}
