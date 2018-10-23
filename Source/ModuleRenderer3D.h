#ifndef __MODULE_RENDERER_3D_H__
#define __MODULE_RENDERER_3D_H__

#include "Module.h"
#include "Globals.h"
#include "Light.h"

#include "MathGeoLib/include/Math/float4x4.h"

#include "glew\include\GL\glew.h"
#include "SDL\include\SDL_opengl.h"
#include "SDL/include/SDL_video.h"

#include <map>

#define MAX_LIGHTS 8

struct GameObject;
struct ComponentMesh;

class ModuleRenderer3D : public Module
{
public:

	ModuleRenderer3D(bool start_enabled = true);
	~ModuleRenderer3D();

	bool Init(JSON_Object* jObject);
	update_status PreUpdate(float dt);
	update_status PostUpdate(float dt);
	bool CleanUp();

	void SaveStatus(JSON_Object*) const;
	void LoadStatus(const JSON_Object*);

	void OnResize(int width, int height);
	void CalculateProjectionMatrix();

	float GetFOV() const;
	void SetFOV(float fov);

	bool SetVSync(bool vsync);
	bool GetVSync() const;

	void SetCapabilityState(GLenum capability, bool enable) const;
	bool GetCapabilityState(GLenum capability) const;

	void SetWireframeMode(bool enable) const;
	bool IsWireframeMode() const;

	void SetDebugDraw(bool debugDraw);
	bool GetDebugDraw() const;

	math::float4x4 Perspective(float fovy, float aspect, float n, float f) const;

	// GO' COMPONENTS
	ComponentMesh* CreateMeshComponent(GameObject* parent);
	void EraseComponent(ComponentMesh* toErase);

private:
	std::vector<ComponentMesh*> meshComponenets;

public:

	uint rendererTexture_id = 0;

	Light lights[MAX_LIGHTS];
	SDL_GLContext context;
	math::float3x3 NormalMatrix;
	math::float4x4 ModelMatrix, ViewMatrix, ProjectionMatrix;

	bool vsync = false;
	bool debugDraw = false;

	float fov = 0.0f;
};

#endif