#ifndef __MODULE_UI_H__
#define __MODULE_UI_H__

#include "Module.h"

#include "MathGeoLib/include/Math/float4.h"
#include <list>

#define UI_XRECT 0
#define UI_YRECT 1
#define UI_WIDTHRECT 2
#define UI_HEIGHTRECT 3

//Possible Solution
//https://stackoverflow.com/questions/47026863/opengl-geometry-shader-with-orthographic-projection

enum UIState
{
	IDLE,
	HOVERED,
	R_CLICK,
	L_CLICK
};

class ResourceShaderProgram;
class ResourceShaderObject;
class ComponentRectTransform;
class TextureImportSettings;
class ResourceTexture;

class ModuleUI : public Module
{
public:
	ModuleUI(bool start_enabled = true);
	~ModuleUI();

	void DrawCanvas();

	bool GetUIMode() const;
	void SetUIMode(bool stat);

	void OnWindowResize(uint width, uint height);

	uint* GetRectUI();

	void LinkAllRectsTransform();

private:

	bool Init(JSON_Object* jObject);
	bool Start();
	update_status PreUpdate();
	update_status Update();
	update_status FixedUpdate();
	update_status PostUpdate();
	bool CleanUp();

	void OnSystemEvent(System_Event event);

	void initRenderData();
	void DrawUIColor(ComponentRectTransform* rect, math::float4& color, float rotation = 0.0f);
	void DrawUITexture(ComponentRectTransform* rect, uint texture = 0, float rotation = 0.0f);

	void SetRectToShader(ComponentRectTransform* rect);

	bool IsUIHovered();

public:
	std::list<Component*> componentsUI;

private:
	uint ui_size_draw[4];

	//math::float4x4 orthonormalMatrix = math::float4x4::identity;
	uint reference_vertex;

	uint ui_shader = 0;

	bool uiMode = false;
	
	bool anyItemIsHovered = false;

private:
	bool MouseInScreen();

	//Shader functions
		// use/activate the shader
	static void use(unsigned int ID);
	//Delete manually shader
	static void Delete(unsigned int ID);

	// utility uniform functions
	static void setBool(unsigned int ID, const char* name, bool value);
	static void setBool(unsigned int ID, const char* name, bool value, bool value2);
	static void setBool(unsigned int ID, const char* name, bool value, bool value2, bool value3);
	static void setBool(unsigned int ID, const char* name, bool value, bool value2, bool value3, bool value4);

	static void setInt(unsigned int ID, const char* name, int value);
	static void setInt(unsigned int ID, const char* name, int value, int value2);
	static void setInt(unsigned int ID, const char* name, int value, int value2, int value3);
	static void setInt(unsigned int ID, const char* name, int value, int value2, int value3, int value4);

	static void setFloat(unsigned int ID, const char* name, float value);
	static void setFloat(unsigned int ID, const char* name, float value, float value2);
	static void setFloat(unsigned int ID, const char* name, float value, float value2, float value3);
	static void setFloat(unsigned int ID, const char* name, float value, float value2, float value3, float value4);
	static void setFloat(unsigned int ID, const char* name, math::float3 value);

	static void setUnsignedInt(unsigned int ID, const char* name, unsigned int value);
	static void setUnsignedInt(unsigned int ID, const char* name, unsigned int value, unsigned int value2);
	static void setUnsignedInt(unsigned int ID, const char* name, unsigned int value, unsigned int value2, unsigned int value3);
	static void setUnsignedInt(unsigned int ID, const char* name, unsigned int value, unsigned int value2, unsigned int value3, unsigned int value4);

	static void setFloat3x3(unsigned int ID, const char* name, const float* trans);
	static void setFloat4x4(unsigned int ID, const char* name, const float* trans);
};

#endif