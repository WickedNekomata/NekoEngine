#ifndef __EVENT_SYSTEM_H__
#define __EVENT_SYSTEM_H__

enum System_Event_Type 
{ 
	NoEvent,
	RefreshAssets, RefreshFiles, CopyShadersIntoLibrary,
	FileDropped, NewFile, FileRemoved, MetaRemoved, FileOverwritten, // FileEvent
	RecalculateBBoxes, // GameObjectEvent
	ShaderProgramChanged,
	RecreateQuadtree,
	Play,
	Pause,
	Stop,
	ResourceDestroyed,
	GameObjectDestroyed,
	ComponentDestroyed,

	// LayerEvent
	LayerNameReset,
	LayerChanged,
	LayerFilterMaskChanged,

	//Scripting Events
	Domain_Destroyed
};

class GameObject;
class Component;
class ComponentCollider;
class Resource;

struct FileEvent
{
	System_Event_Type type;
	const char* file;
	const char* metaFile;
};

struct GameObjectEvent
{
	System_Event_Type type;
	GameObject* gameObject;
};

struct ShaderEvent
{
	System_Event_Type type;
	uint shader;
};

struct ResourceEvent
{
	System_Event_Type type;
	Resource* resource;
};

struct ComponentEvent
{
	System_Event_Type type;
	Component* component;
};

struct LayerEvent
{
	System_Event_Type type;
	ComponentCollider* collider;
	uint layer;
};

union System_Event
{
	System_Event_Type type;      /**< Event type, shared with all events */
	FileEvent fileEvent;             /**< Drag and drop event data */
	GameObjectEvent goEvent;
	ShaderEvent shaderEvent;
	ResourceEvent resEvent;
	ComponentEvent compEvent;
	LayerEvent layerEvent;

	/* This is necessary for ABI compatibility between Visual C++ and GCC
	   Visual C++ will respect the push pack pragma and use 52 bytes for
	   this structure, and GCC will use the alignment of the largest datatype
	   within the union, which is 8 bytes.

	   So... we'll add padding to force the size to be 56 bytes for both.

	   padding?
	*/
};

#endif
