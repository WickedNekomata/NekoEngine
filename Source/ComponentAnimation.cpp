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
	//this->resource = resource;
}

ComponentAnimation::~ComponentAnimation()
{
	//Resource* res = (Resource*)GetResource();
	//if (res)
		//res->Release();
}

uint ComponentAnimation::GetInternalSerializationBytes()
{
	return 0u;
}

bool ComponentAnimation::Save(JSON_Object* component_obj) const
{
	//todo: get resource path etc
	/*const Resource* res = this->GetResource();
	if (res)
		json_object_set_string(component_obj, "path", res->GetExportedFile());*/
	return true;
}

bool ComponentAnimation::Load(const JSON_Object * component_obj)
{
	bool ret = true;

	/*JSON_Value* value = json_object_get_value(component_obj, "path");
	const char* file_path = json_value_get_string(value);

	//todo clean
	if (file_path) {
		std::string uid_force = file_path;
		const size_t last_slash = uid_force.find_last_of("\\/");
		if (std::string::npos != last_slash)
			uid_force.erase(0, last_slash + 1);
		const size_t extension = uid_force.rfind('.');
		if (std::string::npos != extension)
			uid_force.erase(extension);
		UID uid = 0u;
		if (!uid_force.empty())
			uid = static_cast<unsigned int>(std::stoul(uid_force));

		if (uid > 0u)
			SetResource(App->resources->animation_importer->GenerateResourceFromFile(file_path, uid));
		else
			SetResource(App->resources->animation_importer->GenerateResourceFromFile(file_path));
	}*/

	return ret;
}

bool ComponentAnimation::SetResource(uint resource)
{
	/*if (Resource* res = (Resource*)GetResource()) {
		res->Release();
	}

	this->resource = resource;
	ResourceAnimation* bone_res = (ResourceAnimation*)this->GetResource();

	if (bone_res)
		uint num_references = bone_res->LoadToMemory();

	//if(bone_res)
		//App->animation->SetAnimationGos(bone_res);
		*/

	return true;
}