#ifndef __COMPONENT_CANVASRENDERER_H__
#define __COMPONENT_CANVASRENDERER_H__

#include "Component.h"

#include "Globals.h"

class ComponentCanvasRenderer : public Component
{
	ComponentCanvasRenderer(GameObject* parent, ComponentTypes componentType = ComponentTypes::CanvasRendererComponent);
	ComponentCanvasRenderer(const ComponentCanvasRenderer& componentRectTransform);
	~ComponentCanvasRenderer();

	void Update();
	void OnEditor();

private:
	uint GetInternalSerializationBytes();
	void OnInternalSave(char*& cursor);
	void OnInternalLoad(char*& cursor);
	void OnUniqueEditor();
};

#endif