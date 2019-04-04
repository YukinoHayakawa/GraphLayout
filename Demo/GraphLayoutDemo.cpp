#include "GraphLayoutDemo.hpp"

#include <Usagi/Asset/AssetRoot.hpp>
#include <Usagi/Asset/Package/Filesystem/FilesystemAssetPackage.hpp>
#include <Usagi/Game/GameStateManager.hpp>
#include <Usagi/Runtime/Window/Window.hpp>

#include "GraphLayoutDemoState.hpp"
#include <GraphLayoutDemo/Editor/NodeEditorState.hpp>

// #include "InitState.hpp"

usagi::GraphLayoutDemo::GraphLayoutDemo(std::shared_ptr<Runtime> runtime)
    : GraphicalGame(std::move(runtime))
{
    assets()->addChild<FilesystemAssetPackage>("imgui", "Data/imgui");
    assets()->addChild<FilesystemAssetPackage>("nuklear", "Data/nuklear");
    assets()->addChild<FilesystemAssetPackage>("dd", "Data/debugdraw");
    createMainWindow(
        u8"Graph Layout Demo",
        Vector2i { 100, 100 },
        Vector2u32 { 1920, 1080 }
    );
    setupRenderTargets(true);

    // mStateManager->pushState<GraphLayoutDemoState>(
    //     "GraphLayoutDemoState", this);
    mStateManager->pushState<NodeEditorState>(
        "NodeEditorState", this);
}
