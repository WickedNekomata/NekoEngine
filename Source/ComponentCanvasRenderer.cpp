#include "ComponentCanvasRenderer.h"

#include "GameObject.h"

#include "imgui\imgui.h"
#include "imgui\imgui_internal.h"

ComponentCanvasRenderer::ComponentCanvasRenderer(GameObject * parent, ComponentTypes componentType) : Component(parent, ComponentTypes::CanvasRendererComponent)
{

}

ComponentCanvasRenderer::ComponentCanvasRenderer(const ComponentCanvasRenderer & componentRectTransform) : Component(parent, ComponentTypes::CanvasRendererComponent)
{
}

ComponentCanvasRenderer::~ComponentCanvasRenderer()
{
}

void ComponentCanvasRenderer::Update()
{
}

void ComponentCanvasRenderer::OnEditor()
{
}

uint ComponentCanvasRenderer::GetInternalSerializationBytes()
{
	return 0;
}

void ComponentCanvasRenderer::OnInternalSave(char *& cursor)
{
}

void ComponentCanvasRenderer::OnInternalLoad(char *& cursor)
{
}

void ComponentCanvasRenderer::OnUniqueEditor()
{
	ImGui::Text("Canvas Renderer");
	ImGui::Spacing();
}
