#include "Component.h"
#include "GameObject.h"

struct _MonoObject;
class ResourceScript;

class ComponentScript : public Component
{
public:
	ComponentScript(std::string scriptName, GameObject* gameObject = nullptr) : scriptName(scriptName), Component(gameObject, ComponentType::ScriptComponent) { }
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

	void OnInspector();

	static uint bytesToSerialize() { return sizeof(uint32_t) * 3 + (sizeof(bool)); }
	uint bytesToSerializePublicVars() const;

	void Serialize(char*& cursor) const;
	void deSerialize(char*& cursor, uint32_t& goUUID);

	void SerializePublicVars(char*& cursor) const;
	void deSerializePublicVars(char*& cursor);

public:
	void InstanceClass();

public:
	bool awaked = false;
	std::string scriptName;

	ResourceScript* scriptRes = nullptr;

	uint32_t handleID = 0;
	_MonoObject* classInstance = nullptr;
};