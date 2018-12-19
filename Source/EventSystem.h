#ifndef __EVENT_SYSTEM_H__
#define __EVENT_SYSTEM_H__

enum System_Event_Type 
{ 
	NoEvent,
	RefreshAssets, RefreshFiles,
	FileDropped, NewFile, FileRemoved, MetaRemoved, FileOverwritten, // FileEvent
	RecalculateBBoxes, // GameObjectEvent
	ShaderProgramChanged,
	RecreateQuadtree
};

class GameObject;

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

union System_Event
{
	System_Event_Type type;      /**< Event type, shared with all events */
	FileEvent fileEvent;             /**< Drag and drop event data */
	GameObjectEvent goEvent;
	ShaderEvent shaderEvent;

	/* This is necessary for ABI compatibility between Visual C++ and GCC
	   Visual C++ will respect the push pack pragma and use 52 bytes for
	   this structure, and GCC will use the alignment of the largest datatype
	   within the union, which is 8 bytes.

	   So... we'll add padding to force the size to be 56 bytes for both.

	   padding?
	*/
};

#endif
