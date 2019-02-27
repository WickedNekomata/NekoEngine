#include "ComponentBone.h"

#include "ComponentMesh.h"
#include "Application.h"
#include "Resource.h"
#include "ModuleResourceManager.h"
#include "ModuleRenderer3D.h"
#include "SceneImporter.h"
#include "GameObject.h"
#include "ResourceMesh.h"
#include "ResourceBone.h"
//#include "BoneImporter.h"

ComponentBone::ComponentBone(GameObject * embedded_game_object) :
	Component(embedded_game_object, ComponentTypes::BoneComponent)
{
}

ComponentBone::ComponentBone(GameObject * embedded_game_object, uint resource) :
	Component(embedded_game_object, ComponentTypes::BoneComponent)
{
	res = resource;
}

ComponentBone::~ComponentBone()
{

}

void ComponentBone::OnEditor()
{
}

uint ComponentBone::GetInternalSerializationBytes()
{
	return sizeof(uint) + sizeof(uint);
}

void ComponentBone::OnInternalSave(char*& cursor)
{
	size_t bytes = sizeof(uint);
	memcpy(cursor, &res, bytes);
	cursor += bytes;

	bytes = sizeof(uint);
	memcpy(cursor, &attachedMesh, bytes);
	cursor += bytes;
}

void ComponentBone::OnInternalLoad(char*& cursor)
{
	uint loadedRes;
	size_t bytes = sizeof(uint);
	memcpy(&loadedRes, cursor, bytes);
	cursor += bytes;
	SetResource(loadedRes);

	bytes = sizeof(uint);
	memcpy(&attachedMesh, cursor, bytes);
	cursor += bytes;
}

bool ComponentBone::SetResource(uint resource) //check all this
{
	res = resource;

	return true;
}
