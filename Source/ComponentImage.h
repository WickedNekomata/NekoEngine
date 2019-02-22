#ifndef __COMPONENT_IMAGE_H__
#define __COMPONENT_IMAGE_H__

#define MAX_COLOR 255
#define MAX_ALPHA 1

#define COLOR_R 0
#define COLOR_G 1
#define COLOR_B 2
#define COLOR_A 3

#include "Component.h"

#include "Globals.h"

class ComponentImage : public Component
{
public:
	ComponentImage(GameObject* parent, ComponentTypes componentType = ComponentTypes::ImageComponent);
	ComponentImage(const ComponentImage& componentRectTransform);
	~ComponentImage();

	void Update();
	void OnEditor();

private:
	uint GetInternalSerializationBytes();
	void OnInternalSave(char*& cursor);
	void OnInternalLoad(char*& cursor);
	void OnUniqueEditor();

	uint res_image = 0;

	bool use_color_vec = true;
	float color[4] = { 0.0f,0.0f,0.0f,1.0f };
};

#endif