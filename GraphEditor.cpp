#include "GraphEditor.hpp"

#include <Usagi/Utility/Functional.hpp>
#include <Usagi/Sampler/RandomSampler.hpp>
#include <Usagi/Extension/ImGui/ImGui.hpp>
#include <imgui/imgui_internal.h>

void usagi::GraphEditor::drawEditor(const Clock &clock)
{
    using namespace ImGui;

    auto &draw_list = GetCurrentContext()->OverlayDrawList;
    draw_list.Flags &= ~ImDrawListFlags_AntiAliasedFill;
    draw_list.Flags &= ~ImDrawListFlags_AntiAliasedLines;

    const auto im = [](const Vector2f &v) { return ImVec2 {
        v.x(), v.y()
    }; };

    for(auto && v : mGraph.vertices)
    {
        draw_list.AddCircleFilled(im(v), 4, 0xFFFFFFFF, 4);
    }

    if(Begin("Graph Editor"))
    {

    }
    End();
}

usagi::GraphEditor::GraphEditor(Element *parent, std::string name)
    : PredefinedElement<DelegatedImGuiComponent>(parent, std::move(name))
{
    comp<DelegatedImGuiComponent>()->draw_func =
        partial_apply(&GraphEditor::drawEditor, this);

    RandomSampler s;
    mGraph.vertices.clear();
    mGraph.vertices.reserve(100);
    for(int i = 0; i < 100; ++i)
    {
        mGraph.vertices.emplace_back(s.next2D() * 1000);
    }
}
