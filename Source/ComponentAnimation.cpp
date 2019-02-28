#include "ComponentAnimation.h"

#include "ComponentMesh.h"
#include "Application.h"
#include "ModuleResourceManager.h"
#include "ModuleRenderer3D.h"
#include "ModuleFileSystem.h"
#include "SceneImporter.h"
#include "GameObject.h"
#include "Resource.h"
#include "ResourceMesh.h"
#include "ResourceAnimation.h"
#include "AnimationImporter.h"
#include "ModuleAnimation.h"

ComponentAnimation::ComponentAnimation(GameObject * embedded_game_object) :
	Component(embedded_game_object, ComponentTypes::AnimationComponent)
{
}

ComponentAnimation::ComponentAnimation(GameObject* embedded_game_object, uint resource) :
	Component(embedded_game_object, ComponentTypes::AnimationComponent)
{
	res = resource;
}

ComponentAnimation::ComponentAnimation(const ComponentAnimation & component_anim, GameObject * parent, bool include) : Component(parent, ComponentTypes::AnimationComponent)
{
	res = component_anim.res;
}

ComponentAnimation::~ComponentAnimation()
{
	
}

uint ComponentAnimation::GetInternalSerializationBytes()
{
	return sizeof(uint);
}

void ComponentAnimation::OnInternalSave(char*& cursor)
{
	size_t bytes = sizeof(uint);
	memcpy(cursor, &res, bytes);
	cursor += bytes;

}

void ComponentAnimation::OnInternalLoad(char*& cursor)
{
	uint loadedRes;
	size_t bytes = sizeof(uint);
	memcpy(&loadedRes, cursor, bytes);
	cursor += bytes;
	SetResource(loadedRes);

}

bool ComponentAnimation::SetResource(uint resource) //check all this
{
	res = resource;

	return true;
}
