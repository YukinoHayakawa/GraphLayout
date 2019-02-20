#include "GraphEditor.hpp"

#include <Usagi/Utility/Functional.hpp>
#include <Usagi/Sampler/RandomSampler.hpp>
#include <Usagi/Extension/ImGui/ImGui.hpp>
#include <Usagi/Interactive/InputMapping.hpp>
#include <Usagi/Extension/DebugDraw/DelegatedDebugDrawComponent.hpp>

template <>
struct usagi::ElementBasedPointGraph::Traits::property_accessor<usagi::ForcePropertyTag>
{
    // todo per graph storage
    inline static std::unordered_map<Element*, Vector3f> forces;

    auto & operator()(vertex_iterator_t i)
    {
        return forces[i->get()];
    }
};

void usagi::GraphEditor::drawEditor(const Clock &clock)
{
    using namespace ImGui;

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

    removeChild(mGraph);
    mGraph = addChild<ElementBasedPointGraph>("Graph");
    mLayout = { mGraph };

    for(int i = 0; i < mVertexCount; ++i)
    {
        mGraph->addVertex(s.next3D() * 5);
        for(int j = 0; j < i; ++j)
        {
            assert(j != i);
            if(mD(mRng) < mEdgeConnectP)
                mGraph->addEdge(i, j);
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
}
