#include "ComponentCanvasRenderer.h"

#include "GameObject.h"

#include "ComponentImage.h"

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
	ComponentImage* cmp_image = (ComponentImage*)parent->GetComponent(ComponentTypes::ImageComponent);
	if (cmp_image)
		if (cmp_image->UseColor())
			rend_queue.push(new ComponentCanvasRenderer::ToUIRend(RenderTypes::COLOR_VECTOR, cmp_image));
}

void ComponentCanvasRenderer::OnEditor()
{
	OnUniqueEditor();
}

ComponentCanvasRenderer::ToUIRend* ComponentCanvasRenderer::GetFistQueue() const
{
	if (rend_queue.size() > NULL)
		return	rend_queue.front();
	else
		return nullptr;
}

void ComponentCanvasRenderer::Drawed() 
{
	RELEASE(rend_queue.front());
	rend_queue.pop();
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
}

//Rend Queue Struct

math::float4 ComponentCanvasRenderer::ToUIRend::GetColor()
{
	const float* colors = ((ComponentImage*)cmp)->GetColor();
	return { colors[COLOR_R], colors[COLOR_G], colors[COLOR_B], colors[COLOR_A] };
}

uint ComponentCanvasRenderer::ToUIRend::GetTexture()
{
	return ((ComponentImage*)cmp)->GetResImage();
}
