#include "GraphEditor.hpp"

#include <Usagi/Utility/Functional.hpp>
#include <Usagi/Sampler/RandomSampler.hpp>
#include <Usagi/Extension/ImGui/ImGui.hpp>
#include <Usagi/Interactive/InputMapping.hpp>
#include <imgui/imgui_internal.h>
#include <Usagi/Extension/DebugDraw/DelegatedDebugDrawComponent.hpp>
#include <GraphLayoutDemo/ElementGraph/ElementBasedPointGraph.hpp>

void usagi::GraphEditor::drawEditor(const Clock &clock)
{
    using namespace ImGui;

    auto &draw_list = GetCurrentContext()->OverlayDrawList;
    draw_list.Flags &= ~ImDrawListFlags_AntiAliasedFill;
    draw_list.Flags &= ~ImDrawListFlags_AntiAliasedLines;

    /*const auto im = [this](const Vector3f &v) {
        const auto t = v * mScale + mOffset;
        return ImVec2 { t.x(), t.y() };
    };

    for(auto &&v : mGraph.vertices)
    {
        draw_list.AddCircleFilled(
            im(v.position), mVertexRadius,
            ColorConvertFloat4ToU32((ImVec4&)mVertexColor),
            5
        );
    }
    for(auto &&e : mGraph.edges)
    {
        draw_list.AddLine(
            im(mGraph.vertices[e.first].position),
            im(mGraph.vertices[e.second].position),
            ColorConvertFloat4ToU32((ImVec4&)mEdgeColor)
        );
    }*/

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

        DragFloat("Edge Connect Possibility", &mEdgeConnectP,
            0.01f, 0, 1);
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
    // mGraph.vertices.clear();
    // mGraph.vertices.reserve(mVertexCount);
    // mGraph.edges.clear();
    //
    // for(int i = 0; i < mVertexCount; ++i)
    // {
    //     mGraph.vertices.push_back({ s.next3D(), Vector3f::Zero() });
    //     for(int j = 0; j < mEdgeCount; ++j)
    //     {
    //         const auto i2 = (i + j) % mVertexCount;
    //         if(i != i2 && mD(mRng) < mEdgeConnectP)
    //             mGraph.edges.insert({ i, i2 });
    //     }
    // }

    removeChild(mGraph2);
    mGraph2 = addChild<ElementBasedPointGraph>("Graph");

    for(int i = 0; i < mVertexCount; ++i)
    {
        mGraph2->addVertex(s.next3D() * 5);
        for(int j = 0; j < mEdgeCount; ++j)
        {
            const auto i2 = (i + j) % mVertexCount;
            if(i != i2 && mD(mRng) < mEdgeConnectP)
                mGraph.edges.insert({ i, i2 });
        }
    }
}

usagi::GraphEditor::GraphEditor(Element *parent, std::string name)
    : PredefinedElement(parent, std::move(name))
{
    comp<DelegatedImGuiComponent>()->draw_func =
        partial_apply(&GraphEditor::drawEditor, this);

    generateGraph();

    const auto input_mapping = comp<InputComponent>()->input_mapping;
    auto ui_actions = input_mapping->actionGroup("GraphEditorUI");
    // zoom in
    ui_actions->setBinaryActionHandler("ZoomIn",
        [this](bool pressed) {
            if(pressed) mScale += 10;
        });
    ui_actions->bindMouseButton("ZoomIn", MouseButtonCode::WHEEL_UP);
    // zoom out
    ui_actions->setBinaryActionHandler("ZoomOut",
        [this](bool pressed) {
            if(pressed) mScale -= 10;
        });
    ui_actions->bindMouseButton("ZoomOut", MouseButtonCode::WHEEL_DOWN);
    // generate graph
    ui_actions->setBinaryActionHandler("GenerateGraph",
        [this](bool pressed) {
            if(pressed) generateGraph();
        });
    ui_actions->bindKey("GenerateGraph", KeyCode::F5);
    // update
    ui_actions->setBinaryActionHandler("ToggleUpdateGraph",
        [this](bool pressed) {
            if(pressed) mUpdate = !mUpdate;
        });
    ui_actions->bindKey("ToggleUpdateGraph", KeyCode::F6);

    addComponent(this);
}

void usagi::GraphEditor::draw(dd::ContextHandle ctx)
{
    const Vector3f origin = Vector3f::Zero();
    // sphere(ctx, origin.data(), dd::colors::Azure, 5.0f);

    const Matrix4f transform = Matrix4f::Identity();
    dd::axisTriad(ctx, transform.data(), 0.5, 5);

    // for(auto &&v : mGraph.vertices)
    // {
    //     dd::point(ctx, v.position.data(), mVertexColor, 10);
    // }
    // for(auto &&e : mGraph.edges)
    // {
    //     dd::line(ctx,
    //         mGraph.vertices[e.first].position.data(),
    //         mGraph.vertices[e.second].position.data(),
    //         mEdgeColor
    //     );
    // }
}
