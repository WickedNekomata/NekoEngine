#ifndef __MODULE_UI_H__
#define __MODULE_UI_H__

#include "Module.h"

#include "MathGeoLib/include/MathGeoLib.h"

//Possible Solution
//https://stackoverflow.com/questions/47026863/opengl-geometry-shader-with-orthographic-projection

class ResourceShaderProgram;
class ResourceShaderObject;
class ComponentRect;
class TextureImportSettings;
class ResourceTexture;

class ModuleUI : public Module
{
public:
	ModuleUI(bool start_enabled = true);
	~ModuleUI();

	void DrawTest();

	bool GetUIMode() const;
	void SetUIMode(bool stat);

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
	void DrawSquare(ComponentRect* rect, float rotation = 0.0f, math::float3 color = math::float3::one);

	//math::float4x4 orthonormalMatrix = math::float4x4::identity;
	uint reference_vertex;

	ResourceShaderProgram* ui_shader = nullptr;
	ResourceShaderObject* ui_fragment = nullptr;
	ResourceShaderObject* ui_vertex = nullptr;

	TextureImportSettings* texture_test = nullptr;
	ResourceTexture* texture_loaded = nullptr;

	ComponentRect* rect_test;

	bool uiMode = false;

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