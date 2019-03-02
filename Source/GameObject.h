#ifndef __GAMEOBJECT_H__
#define __GAMEOBJECT_H__

#include "Globals.h"

#include "ComponentTypes.h"
#include "EventSystem.h"

#include "MathGeoLib\include\Geometry\AABB.h"
#include <mono/metadata/object.h>

#include <vector>

class Component;

class GameObject
{
public:

	GameObject(const char* name, GameObject* parent, bool disableTransform = false);
	GameObject(GameObject& gameObject, GameObject* newRoot = 0);
	~GameObject();

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

	Component* AddComponent(ComponentTypes componentType, bool createDependencies = true);
	void AddComponent(Component* component);
	bool DestroyComponent(Component* destroyed);
	void EraseComponent(Component* erased);
	Component* GetComponent(int index) const;
	Component* GetComponent(ComponentTypes type) const;
	int GetComponentsLength();
	void ReorderComponents(Component* source, Component* target);

	void GetChildrenVector(std::vector<GameObject*>& go);
	uint GetSerializationBytes() const;
	void OnSave(char*& cursor) const;
	void OnLoad(char*& cursor);

	void RecursiveForceAllResources(uint forceRes) const;

	// scripting
	MonoObject* GetMonoObject();
	inline void SetMonoObject(uint32_t monoObjectHandle) { this->monoObjectHandle = monoObjectHandle; };

	void SetLayer(uint layerNumber);
	uint GetLayer() const;

public:
	
	class ComponentTransform*  transform = 0;
	class ComponentMesh*       cmp_mesh = 0;
	class ComponentMaterial*   cmp_material = 0;
	class ComponentCamera*     cmp_camera = 0;
	class ComponentNavAgent*   cmp_navAgent = 0;
	class ComponentEmitter*    cmp_emitter = 0;
	class ComponentRigidActor* cmp_rigidActor = 0;
	class ComponentCollider*   cmp_collider = 0;
	class ComponentBone*	   cmp_bone = 0;
	class ComponentLight*	   cmp_light = 0;
	class ComponentAudioListener* cmp_audioListener = 0;
	class ComponentAudioSource*   cmp_audioSource = 0;

	std::vector<Component*> components;

	math::AABB boundingBox;

	bool seenLastFrame = false;

private:

	char name[DEFAULT_BUF_SIZE];
	uint uuid;
	GameObject* parent = 0;
	uint parent_uuid = 0;

	std::vector<GameObject*> children;

	bool isActive = true;
	bool isStatic = false; // coordinate with statics and dynamics vector at go module

	// scripting
	uint32_t monoObjectHandle = 0;

	// Physics
	uint layer;

};

#endif
