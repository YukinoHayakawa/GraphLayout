#include "NodeEditorState.hpp"

#include <Usagi/Graphics/Game/GraphicalGame.hpp>
#include <Usagi/Runtime/Runtime.hpp>
#include <Usagi/Runtime/Input/InputManager.hpp>
#include <Usagi/Extension/ImGui/ImGuiSystem.hpp>
#include <Usagi/Extension/Nuklear/NuklearSystem.hpp>

#include "PortGraphObserver.hpp"

usagi::NodeEditorState::NodeEditorState(
    Element *parent,
    std::string name,
    GraphicalGame *game)
    : GraphicalGameState(parent, std::move(name), game)
{
    const auto input_manager = mGame->runtime()->inputManager();
    const auto imgui = addSystem("imgui", std::make_unique<ImGuiSystem>(
        mGame,
        mGame->mainWindow()->window,
        input_manager->virtualKeyboard(),
        input_manager->virtualMouse()
    ));
    imgui->setSizeFunctionsFromRenderWindow(mGame->mainWindow());
    const auto nk = addSystem("nuklear", std::make_unique<NuklearSystem>(
        mGame,
        mGame->mainWindow()->window,
        input_manager->virtualKeyboard(),
        input_manager->virtualMouse()
    ));
    nk->setSizeFunctionsFromRenderWindow(mGame->mainWindow());

    addChild<PortGraphObserver>("GraphBase");
}
