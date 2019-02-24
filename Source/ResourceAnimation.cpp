#include "ResourceAnimation.h"
#include "imgui/imgui.h"
#include "ModuleScene.h"
#include "ModuleFileSystem.h"

ResourceAnimation::ResourceAnimation(ResourceTypes type, uint uuid, ResourceData data, ResourceAnimationData animationData) : Resource(type, uuid, data), animationData(animationData) {}

ResourceAnimation::~ResourceAnimation()
{
}

bool ResourceAnimation::LoadInMemory()
{
	return true;
}

bool ResourceAnimation::UnloadFromMemory()
{
	return true;
}

void ResourceAnimation::OnPanelAssets()
{
	ImGuiTreeNodeFlags flags = 0;
	flags |= ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_Leaf;

	if (App->scene->selectedObject == this)
		flags |= ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_Selected;

	char id[DEFAULT_BUF_SIZE];
	sprintf(id, "%s##%d", data.name.data(), uuid);

	if (ImGui::TreeNodeEx(id, flags))
		ImGui::TreePop();

	if (ImGui::IsMouseReleased(0) && ImGui::IsItemHovered() /*&& (mouseDelta.x == 0 && mouseDelta.y == 0)*/)
	{
		SELECT(this);
	}

	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
	{
		ImGui::SetDragDropPayload("ANIMATION_RESOURCE", &uuid, sizeof(uint));
		ImGui::EndDragDropSource();
	}
}

bool ResourceAnimation::ImportFile(const char* file, std::string& name, std::string& outputFile)
{

	assert(file != nullptr);

	// Search for the meta associated to the file
	char metaFile[DEFAULT_BUF_SIZE];
	strcpy_s(metaFile, strlen(file) + 1, file); // file
	strcat_s(metaFile, strlen(metaFile) + strlen(EXTENSION_META) + 1, EXTENSION_META); // extension

	if (App->fs->Exists(metaFile))
	{
		// Read the meta
		uint uuid = 0;
		int64_t lastModTime = 0;
		ResourceAnimation::ReadMeta(metaFile, lastModTime, uuid, name);
		assert(uuid > 0 && lastModTime > 0);

		// The uuid of the resource would be the entry
		char entry[DEFAULT_BUF_SIZE];
		sprintf_s(entry, "%u", uuid);
		outputFile = entry;
	}


	

	return true;
}

bool ResourceAnimation::ExportFile(ResourceData& data, ResourceAnimationData& prefabData, std::string& outputFile, bool overwrite)
{
	return true;
}

uint ResourceAnimation::CreateMeta(const char* file, uint animationUUID, std::string& name, std::string& outputMetaFile)
{
	assert(file != nullptr);

	uint uuidsSize = 1;
	uint nameSize = DEFAULT_BUF_SIZE;

	// Name
	char animationName[DEFAULT_BUF_SIZE];
	strcpy_s(animationName, DEFAULT_BUF_SIZE, name.data());

	// --------------------------------------------------

	uint size =
		sizeof(int64_t) +
		sizeof(uint) +
		sizeof(uint) * uuidsSize +

		sizeof(char) * nameSize;

	char* data = new char[size];
	char* cursor = data;

	// 1. Store last modification time
	int64_t lastModTime = App->fs->GetLastModificationTime(file);
	assert(lastModTime > 0);
	uint bytes = sizeof(int64_t);
	memcpy(cursor, &lastModTime, bytes);

	cursor += bytes;

	// 2. Store uuids size
	bytes = sizeof(uint);
	memcpy(cursor, &uuidsSize, bytes);

	cursor += bytes;

	// 3. Store animation uuid
	bytes = sizeof(uint) * uuidsSize;
	memcpy(cursor, &animationUUID, bytes);

	cursor += bytes;

	// 4. Store animation name size
	bytes = sizeof(uint);
	memcpy(cursor, &nameSize, bytes);

	cursor += bytes;

	// 5. Store animation name
	bytes = sizeof(char) * nameSize;
	memcpy(cursor, animationName, bytes);

	// --------------------------------------------------

	// Build the path of the meta file and save it
	outputMetaFile = file;
	outputMetaFile.append(EXTENSION_META);
	uint resultSize = App->fs->Save(outputMetaFile.data(), data, size);
	if (resultSize > 0)
	{
		CONSOLE_LOG(LogTypes::Normal, "Resource Animation: Successfully saved meta '%s'", outputMetaFile.data());
	}
	else
	{
		CONSOLE_LOG(LogTypes::Error, "Resource Animation: Could not save meta '%s'", outputMetaFile.data());
		return 0;
	}

	return lastModTime;
}

bool ResourceAnimation::ReadMeta(const char* metaFile, int64_t& lastModTime, uint& animationUUID, std::string& name)
{
	assert(metaFile != nullptr);

	char* buffer;
	uint size = App->fs->Load(metaFile, &buffer);

	if (size > 0)
	{
		char* cursor = (char*)buffer;

		// 1. Load last modification time
		uint bytes = sizeof(int64_t);
		memcpy(&lastModTime, cursor, bytes);

		cursor += bytes;

		// 2. Load uuids size
		uint uuidsSize = 0;
		bytes = sizeof(uint);
		memcpy(&uuidsSize, cursor, bytes);
		assert(uuidsSize > 0);

		cursor += bytes;

		// 3. Load animation uuid
		bytes = sizeof(uint) * uuidsSize;
		memcpy(&animationUUID, cursor, bytes);

		cursor += bytes;

		// 4. Load animation name size
		uint nameSize = 0;
		bytes = sizeof(uint);
		memcpy(&nameSize, cursor, bytes);
		assert(nameSize > 0);

		cursor += bytes;

		// 5. Load animation name
		name.resize(nameSize);
		bytes = sizeof(char) * nameSize;
		memcpy(&name[0], cursor, bytes);

		CONSOLE_LOG(LogTypes::Normal, "Resource Animation: Successfully loaded meta '%s'", metaFile);
		RELEASE_ARRAY(buffer);
	}
	else
	{
		CONSOLE_LOG(LogTypes::Error, "Resource Animation: Could not load meta '%s'", metaFile);
		return false;
	}

	return true;
}

bool ResourceAnimation::LoadFile(const char* file, ResourceAnimationData& outputAnimationData)
{
	ResourceAnimation* resource = (ResourceAnimation*)App->resources->CreateNewResource(Resource::Type::ANIMATION, uid_to_force);

	char* cursor = buffer;

	// Load anim name
	uint bytes = sizeof(char) * TITLE_MAX_LENGTH;
	char name[TITLE_MAX_LENGTH];
	memcpy(&name, cursor, bytes);
	resource->name = name;

	// Load duration
	cursor += bytes;
	bytes = sizeof(resource->duration);
	memcpy(&resource->duration, cursor, bytes);

	// Load ticks per second
	cursor += bytes;
	bytes = sizeof(resource->ticks_per_second);
	memcpy(&resource->ticks_per_second, cursor, bytes);

	// Load amount Bone transformations
	cursor += bytes;
	bytes = sizeof(resource->num_keys);
	memcpy(&resource->num_keys, cursor, bytes);

	resource->bone_keys = new ResourceAnimation::BoneTransformation[resource->num_keys];

	char buff[4096];
	for (uint i = 0; i < resource->num_keys; ++i)
	{
		ResourceAnimation::BoneTransformation* bone = &resource->bone_keys[i];
		uint count = 0;

		// load bone name size
		cursor += bytes;
		bytes = sizeof(count);
		memcpy(&count, cursor, bytes);

		// load bone name
		cursor += bytes;
		bytes = sizeof(char) * count + 1;
		memcpy(buff, cursor, bytes);
		bone->bone_name = buff;

		// load num_positions -------------------------------
		cursor += bytes;
		bytes = sizeof(count);
		memcpy(&count, cursor, bytes);
		bone->positions.Init(ResourceAnimation::BoneTransformation::Key::KeyType::POSITION, count);

		// load position times
		cursor += bytes;
		bytes = sizeof(double) * count;
		memcpy(bone->positions.time, cursor, bytes);

		// load position values
		cursor += bytes;
		bytes = sizeof(float) * 3 * count;
		memcpy(bone->positions.value, cursor, bytes);

		// load num rotations -------------------------------
		count = 0;
		cursor += bytes;
		bytes = sizeof(count);
		memcpy(&count, cursor, bytes);
		bone->rotations.Init(ResourceAnimation::BoneTransformation::Key::KeyType::ROTATION, count);

		// load rotation times
		cursor += bytes;
		bytes = sizeof(double) * count;
		memcpy(bone->rotations.time, cursor, bytes);

		// load rotation values
		cursor += bytes;
		bytes = sizeof(float) * 4 * count;
		memcpy(bone->rotations.value, cursor, bytes);

		// load num_scales -------------------------------
		count = 0;
		cursor += bytes;
		bytes = sizeof(count);
		memcpy(&count, cursor, bytes);
		bone->scalings.Init(ResourceAnimation::BoneTransformation::Key::KeyType::SCALE, count);

		// load position times
		cursor += bytes;
		bytes = sizeof(double) * count;
		memcpy(bone->scalings.time, cursor, bytes);

		// load position values
		cursor += bytes;
		bytes = sizeof(float) * 3 * count;
		memcpy(bone->scalings.value, cursor, bytes);
	}

	App->animation->SetAnimationGos(resource);

	RELEASE_ARRAY(buffer);

	return true;
}
