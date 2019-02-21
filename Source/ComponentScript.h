#include "Component.h"
#include "GameObject.h"

#include <mono/metadata/object.h>

class ResourceScript;

class ComponentScript : public Component
{
public:
	ComponentScript(std::string scriptName, GameObject* gameObject = nullptr);
	virtual ~ComponentScript();

	void Awake();
	void Start();
	void PreUpdate();
	void Update();
	void PostUpdate();
	void OnEnableMethod();
	void OnDisableMethod();
	void OnStop();

	void OnEnable() override;
	void OnDisable() override;

	void OnUniqueEditor() override;
	
	uint GetInternalSerializationBytes();
	uint GetPublicVarsSerializationBytes() const;

	void Serialize(char*& cursor) const;
	void deSerialize(char*& cursor, uint32_t& goUUID);

	void SavePublicVars(char*& cursor) const;
	void LoadPublicVars(char*& cursor);

	void OnInternalSave(char*& cursor);
	void OnInternalLoad(char*& cursor);

public:
	void InstanceClass();

public:
	bool awaked = false;
	std::string scriptName;

	ResourceScript* scriptRes = nullptr;

	uint32_t handleID = 0;
	MonoObject* classInstance = nullptr;
};