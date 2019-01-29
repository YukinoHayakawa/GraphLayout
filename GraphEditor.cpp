#include "GraphEditor.hpp"

#include <Usagi/Utility/Functional.hpp>
#include <Usagi/Extension/ImGui/ImGui.hpp>

void usagi::GraphEditor::drawEditor(const Clock &clock)
{
    using namespace  ImGui;
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
}
