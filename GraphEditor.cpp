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

    const auto im = [this](const Vector2f &v) {
        const auto t = v * mScale + mOffset;
        return ImVec2 { t.x(), t.y() };
    };

    for(auto && v : mGraph.vertices)
    {
        draw_list.AddCircleFilled(
            im(v.position), mVertexRadius,
            ColorConvertFloat4ToU32((ImVec4&)mVertexColor),
            5
        );
    }
    for(auto && e : mGraph.edges)
    {
        draw_list.AddLine(
            im(mGraph.vertices[e.first].position),
            im(mGraph.vertices[e.second].position),
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

        Checkbox("Update", &mUpdate);
        if(Button("Step Naive Spring Algorithm"))
        {
            mLayout.update();
        }

        DragFloat("c1 (spring constant)", &mLayout.c1,
            0.1f, 0.1f, 1000.f);
        DragFloat("c2 (original spring length)", &mLayout.c2,
            0.1f, 0.1f, 1000.f);
        DragFloat("c3 (repel factor)", &mLayout.c3,
            0.1f, 0.1f, 1000.f);
        DragFloat("c4 (update rate)", &mLayout.c4,
            0.1f, 0.1f, 1000.f);

        DragFloat("View Scale", &mScale, 0.01f);
        DragFloat2("View Offset", mOffset.data());
    }
    End();

    if(mUpdate)
        mLayout.update();
}

void usagi::GraphEditor::generateGraph()
{
    RandomSampler s;
    mGraph.vertices.clear();
    mGraph.vertices.reserve(mVertexCount);
    mGraph.edges.clear();

    for(int i = 0; i < mVertexCount; ++i)
    {
        mGraph.vertices.push_back({ s.next2D(), Vector2f::Zero() });
        for(int j = 0; j < mEdgeCount; ++j)
        {
            const auto i2 = (i + j) % mVertexCount;
            if(i != i2)
                mGraph.edges.insert({ i, i2 });
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
