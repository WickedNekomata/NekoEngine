#ifndef __GAMEOBJECT_H__
#define __GAMEOBJECT_H__

#include "Globals.h"

#include "ComponentTypes.h"
#include "EventSystem.h"

#include "MathGeoLib\include\Geometry\AABB.h"

#include <mono/metadata/object.h>

#include <vector>

class Component;
class ResourcePrefab;

class GameObject
{
public:

	GameObject(const char* name, GameObject* parent, bool disableTransform = false);
	GameObject(GameObject& gameObject, bool includeComponents = true);
	~GameObject();

	void DestroyTemplate();

	void SetName(const char* name);
	const char* GetName() const;
	uint GetUUID() const;
	void ForceUUID(uint uuid);
	void SetParent(GameObject* parent);
	GameObject* GetParent() const;
	uint GetParentUUID() const;
	void ToggleIsActive();
	void ToggleIsStatic();
	bool IsActive() const;
	bool IsStatic() const;
	void OnEnable();
	void OnDisable();
	void RecursiveRecalculateBoundingBoxes();

	void CalculateBoundingBox();

	void OnSystemEvent(System_Event event);

	void Destroy();
	void GetChildrenAndThisVectorFromLeaf(std::vector<GameObject*>&);
	void AddChild(GameObject* target);
	bool EraseChild(const GameObject* toErase);
	bool IsChild(GameObject* target, bool untilTheEnd) const;
	bool HasChildren() const;
	uint GetChildrenLength() const;
	GameObject* GetChild(uint index) const;
	bool EqualsToChildrenOrThis(const void* isEqual) const;

	Component* AddComponent(ComponentTypes componentType, bool createDependencies = true, bool includeInModules = true);
	void AddComponent(Component* component);
	bool DestroyComponent(Component* destroyed);
	void EraseComponent(Component* erased);
	Component* GetComponent(int index) const;
	Component* GetComponent(ComponentTypes type) const;
	int GetComponentsLength();
	void ReorderComponents(Component* source, Component* target);

	void GetChildrenVector(std::vector<GameObject*>& go, bool thisGo = true);
	uint GetSerializationBytes() const;
	void OnSave(char*& cursor) const;
	void OnLoad(char*& cursor, bool includeInModules = true);

	void RecursiveForceAllResources(uint forceRes) const;

	// scripting
	MonoObject* GetMonoObject();
	inline void SetMonoObject(uint32_t monoObjectHandle) { this->monoObjectHandle = monoObjectHandle; };

	void SetLayer(uint layerNumber);
	uint GetLayer() const;

public:

	class ComponentTransform*  transform = nullptr; //Nullptr a puño y fuego D:<
	class ComponentMesh*       cmp_mesh = nullptr;
	class ComponentMaterial*   cmp_material = nullptr;
	class ComponentCamera*     cmp_camera = nullptr;
	class ComponentNavAgent*   cmp_navAgent = nullptr;
	class ComponentEmitter*    cmp_emitter = nullptr;
	class ComponentRigidActor* cmp_rigidActor = nullptr;
	class ComponentCollider*   cmp_collider = nullptr;
	class ComponentBone*	   cmp_bone = nullptr;
	class ComponentRectTransform* cmp_rectTransform = nullptr;
	class ComponentCanvasRenderer* cmp_canvasRenderer = nullptr;
	class ComponentImage* cmp_image = nullptr;
	class ComponentButton* cmp_button = nullptr;
	class ComponentLabel* cmp_label = nullptr;
	class ComponentAnimation*  cmp_animation = nullptr;
	class ComponentLight*	   cmp_light = nullptr;
	class ComponentProjector*  cmp_projector = nullptr;

	ResourcePrefab* prefab = nullptr;

	std::vector<Component*> components;
	std::vector<GameObject*> children;

	math::AABB originalBoundingBox;
	math::AABB boundingBox;

	bool seenLastFrame = false;

private:

	char name[DEFAULT_BUF_SIZE];
	uint uuid;
	GameObject* parent = 0;
	uint parent_uuid = 0;

	bool isActive = true;
	bool isStatic = false; // coordinate with statics and dynamics vector at go module

	// Scripting
	uint32_t monoObjectHandle = 0;

	// Physics
	uint layer = 0; // in the range [0...31]
};

#endif
