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
	
	/*Resource* res = (Resource*)GetResource();
	
	if (res)
		res->Release();*/ //CHECK THIS
}

bool ComponentBone::Save(JSON_Object* component_obj) const
{
	//todo: get resource path etc
	const Resource* res = (Resource*)App->res->GetResource(this->res);
	if (res)
		json_object_set_string(component_obj, "path", res->exportedFile.c_str());
	return true;
}

bool ComponentBone::Load(const JSON_Object * component_obj)
{
	bool ret = true;

	JSON_Value* value = json_object_get_value(component_obj, "path");
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
		uint uid = 0u;
		if (!uid_force.empty())
			uid = static_cast<unsigned int>(std::stoul(uid_force));

		/*if (uid > 0u)
			SetResource(App->res->bone_importer->GenerateResourceFromFile(file_path, uid));
		else
			SetResource(App->res->bone_importer->GenerateResourceFromFile(file_path));*/
	}

	return ret;
}

bool ComponentBone::SetResource(uint resource) //check all this
{

	if (Resource* res = (Resource*)App->res->GetResource(resource)) {
		//res->UnloadMemory();
	}
	
	res = resource;
	ResourceBone* bone_res = (ResourceBone*)(Resource*)App->res->GetResource(resource);

	if (bone_res)
		uint num_references = bone_res->LoadInMemory(); 

	return true;
}
