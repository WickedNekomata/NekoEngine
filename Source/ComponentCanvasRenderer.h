#ifndef __COMPONENT_CANVASRENDERER_H__
#define __COMPONENT_CANVASRENDERER_H__

#include "Component.h"

#include "Globals.h"

#include <vector>

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
		ToUIRend() {}
		
		void Set(RenderTypes t, Component* c)
		{
			type = t;
			cmp = c;
			isRendered_flag = false;
		}

		RenderTypes GetType()const
		{
			return type;
		}
		math::float4 GetColor();
		uint GetTexture();

		bool isRendered() {
			return isRendered_flag;
		}

	private:
		RenderTypes type = RenderTypes::RENDER_NULL;
		Component* cmp = nullptr;
		bool isRendered_flag = true;
	};

	ComponentCanvasRenderer(GameObject* parent, ComponentTypes componentType = ComponentTypes::CanvasRendererComponent);
	ComponentCanvasRenderer(const ComponentCanvasRenderer& componentRectTransform);
	~ComponentCanvasRenderer();

	void Update();
	void OnEditor();

	ToUIRend* GetDrawAvaiable() const;

private:
	uint GetInternalSerializationBytes();
	void OnInternalSave(char*& cursor);
	void OnInternalLoad(char*& cursor);
	void OnUniqueEditor();

	std::vector<ToUIRend* > rend_queue;
};

#endif