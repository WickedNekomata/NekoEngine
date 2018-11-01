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
struct ComponentCamera;

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

	bool SetVSync(bool vsync);
	bool GetVSync() const;

	void SetCapabilityState(GLenum capability, bool enable) const;
	bool GetCapabilityState(GLenum capability) const;

	void SetWireframeMode(bool enable) const;
	bool IsWireframeMode() const;

	void SetDebugDraw(bool debugDraw);
	bool GetDebugDraw() const;

	void SetDrawBoundingBoxes(bool drawBoundingBoxes);
	bool GetDrawBoundingBoxes() const;

	void SetDrawMainCameraFrustum(bool drawMainCameraFrustum);
	bool GetDrawMainCameraFrustum() const;

	void SetDrawQuadtree(bool drawQuadtree);
	bool GetDrawQuadtree() const;

	ComponentMesh* CreateMeshComponent(GameObject* parent);
	void EraseMeshComponent(ComponentMesh* toErase);

	ComponentCamera* CreateCameraComponent(GameObject* parent);
	void EraseCameraComponent(ComponentCamera* toErase);

	bool RecalculateMainCamera();
	bool SetMainCamera(ComponentCamera* mainCamera);
	ComponentCamera* GetMainCamera() const;

	void SetMeshComponentsSeenLastFrame(bool seenLastFrame);
	void FrustumCulling() const;

	void DrawMesh(ComponentMesh* toDraw) const;
	void RecursiveDrawQuadtree(QuadtreeNode* node) const;

private:

	std::vector<ComponentMesh*> meshComponents;

	std::list<ComponentCamera*> cameraComponents;
	ComponentCamera* mainCamera = nullptr;

public:

	uint rendererTexture_id = 0;

	Light lights[MAX_LIGHTS];
	SDL_GLContext context;
	math::float3x3 NormalMatrix;
	math::float4x4 ModelMatrix, ViewMatrix, ProjectionMatrix;

	bool vsync = false;

	bool debugDraw = false;
	bool drawBoundingBoxes = true;
	bool drawMainCameraFrustum = true;
	bool drawQuadtree = false;
};

#endif