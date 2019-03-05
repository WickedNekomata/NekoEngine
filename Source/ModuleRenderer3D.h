#ifndef __MODULE_RENDERER_3D_H__
#define __MODULE_RENDERER_3D_H__

#include "Module.h"
#include "Globals.h"

#include "ComponentMesh.h"
#include "ComponentCamera.h"
#include "ComponentTransform.h"
#include "GameObject.h"

#include "MathGeoLib\include\Math\float4x4.h"

#include "glew\include\GL\glew.h"
#include "SDL\include\SDL_opengl.h"
#include "SDL\include\SDL_video.h"

#include <vector>

class GameObject;
class ComponentProjector;
class QuadtreeNode;
union Uniform;

class ModuleRenderer3D : public Module
{
public:

	ModuleRenderer3D(bool start_enabled = true);
	~ModuleRenderer3D();

	bool Init(JSON_Object* jObject);
	update_status PreUpdate();
	update_status PostUpdate();
	bool CleanUp();

	void OnSystemEvent(System_Event event);

	void SaveStatus(JSON_Object*) const;
	void LoadStatus(const JSON_Object*);

	void OnResize(int width, int height);
	void CalculateProjectionMatrix();

	uint GetMaxTextureUnits() const;

	bool SetVSync(bool vsync);
	bool GetVSync() const;

	void SetCapabilityState(GLenum capability, bool enable) const;
	bool GetCapabilityState(GLenum capability) const;

	void SetWireframeMode(bool enable) const;
	bool IsWireframeMode() const;

	bool AddMeshComponent(ComponentMesh* toAdd);
	bool EraseMeshComponent(ComponentMesh* toErase);

	bool AddProjectorComponent(ComponentProjector* toAdd);
	bool EraseProjectorComponent(ComponentProjector* toErase);

	bool AddCameraComponent(ComponentCamera* toAdd);
	bool EraseCameraComponent(ComponentCamera* toErase);

	bool RecalculateMainCamera();
	bool SetMainCamera(ComponentCamera* mainCamera);
	bool SetCurrentCamera();
	ComponentCamera* GetMainCamera() const;
	ComponentCamera* GetCurrentCamera() const;

	void SetMeshComponentsSeenLastFrame(bool seenLastFrame);
	void FrustumCulling() const;

	void DrawMesh(ComponentMesh* toDraw) const;
	void DrawProjectors(ComponentProjector* toDraw) const;

	void RecursiveDrawQuadtree(QuadtreeNode* node) const;

	void LoadSpecificUniforms(uint& textureUnit, const std::vector<Uniform>& uniforms, const std::vector<const char*>& ignore = std::vector<const char*>()) const;
	void LoadGenericUniforms(uint shaderProgram) const;

private:

	std::vector<ComponentMesh*> meshComponents;

	std::vector<ComponentProjector*> projectorComponents;

	std::vector<ComponentCamera*> cameraComponents;
	ComponentCamera* mainCamera = nullptr;
	ComponentCamera* currentCamera = nullptr;

	uint maxTextureUnits = 0;

public:

	SDL_GLContext context;
	math::float3x3 NormalMatrix;
	math::float4x4 ModelMatrix, ViewMatrix, ProjectionMatrix;

	bool vsync = false;

	bool debugDraw = false;
	bool drawBoundingBoxes = true;
	bool drawFrustums = true;
	bool drawColliders = true;
	bool drawRigidActors = true;
	bool drawQuadtree = false;
};

#endif