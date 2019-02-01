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
        draw_list.AddCircleFilled(
            im(v), mVertexRadius,
            ColorConvertFloat4ToU32((ImVec4&)mVertexColor),
            5
        );
    }
    for(auto && e : mGraph.edges)
    {
        draw_list.AddLine(
            im(mGraph.vertices[e.first]),
            im(mGraph.vertices[e.second]),
            ColorConvertFloat4ToU32((ImVec4&)mEdgeColor)
        );
    }

    if(Begin("Graph Editor"))
    {
        SliderInt("Vertex Count", &mVertexCount, 0, 100);
        ColorEdit4("Vertex Color", mVertexColor);
        SliderInt("Edge Count", &mEdgeCount, 0, mVertexCount);
        ColorEdit4("Edge Color", mEdgeColor);
        if(Button("Generate Graph"))
            generateGraph();

        Button("Step Naive Spring Algorithm");
    }
    End();
}

void usagi::GraphEditor::generateGraph()
{
    RandomSampler s;
    mGraph.vertices.clear();
    mGraph.vertices.reserve(mVertexCount);
    mGraph.edges.clear();
    mGraph.edges.reserve(mEdgeCount * mVertexCount);

    for(int i = 0; i < mVertexCount; ++i)
    {
        mGraph.vertices.emplace_back(s.next2D() * 1000);
        for(int j = 0; j < mEdgeCount; ++j)
        {
            mGraph.edges.push_back({ i, (i + j) % mVertexCount });
        }
    }
}

usagi::GraphEditor::GraphEditor(Element *parent, std::string name)
    : PredefinedElement<DelegatedImGuiComponent>(parent, std::move(name))
{
    comp<DelegatedImGuiComponent>()->draw_func =
        partial_apply(&GraphEditor::drawEditor, this);

    generateGraph();
}
