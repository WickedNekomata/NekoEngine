#ifndef __COMPONENT_CANVASRENDERER_H__
#define __COMPONENT_CANVASRENDERER_H__

#include "Component.h"

#include "Globals.h"

#include <queue>

#include "MathGeoLib/include/Math/float4.h"

class ComponentCanvasRenderer : public Component
{
public:

	enum RenderTypes
	{
		RENDER_NULL,
		COLOR_VECTOR,
		TEXTURE,
		FONT
	};

	struct ToUIRend
	{
	public:
		ToUIRend(RenderTypes t, Component* c) : type(t), cmp(c) {}

		RenderTypes GetType()const
		{
			return type;
		}
		math::float4 GetColor();
		uint GetTexture();

	private:
		RenderTypes type = RenderTypes::RENDER_NULL;
		Component* cmp = nullptr;
	};

	ComponentCanvasRenderer(GameObject* parent, ComponentTypes componentType = ComponentTypes::CanvasRendererComponent);
	ComponentCanvasRenderer(const ComponentCanvasRenderer& componentRectTransform);
	~ComponentCanvasRenderer();

	void Update();
	void OnEditor();

	ToUIRend* GetFistQueue() const;
	void Drawed();

private:
	uint GetInternalSerializationBytes();
	void OnInternalSave(char*& cursor);
	void OnInternalLoad(char*& cursor);
	void OnUniqueEditor();

	std::queue<ToUIRend* > rend_queue;
};

#endif