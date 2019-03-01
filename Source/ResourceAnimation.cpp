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
		bool result = ResourceAnimation::ReadMeta(metaFile, lastModTime, uuid, name);
		assert(result);

		// The uuid of the resource would be the entry
		char entry[DEFAULT_BUF_SIZE];
		sprintf_s(entry, "%u", uuid);
		outputFile = entry;
	}
	
	return true;
}

bool ResourceAnimation::ExportFile(ResourceData& resourceData, ResourceAnimationData& animData, std::string& outputFile, bool overwrite)
{
	bool ret = false;

	// -------------- CALCULATING ANIMATION DATA SIZE --------------

	uint anim_name_size = sizeof(char)*MAX_BUF_SIZE;
	uint duration_size = sizeof(animData.duration);
	uint ticks_size = sizeof(animData.ticksPerSecond);
	uint num_keys_size = sizeof(animData.numKeys);

	uint final_size = anim_name_size + duration_size + ticks_size + num_keys_size;

	for (uint i = 0; i < animData.numKeys; i++)
	{
		uint id_size = sizeof(uint);
		uint bone_name_size = sizeof(char)*animData.boneKeys[i].bone_name.size() + 1; // TODO: check this to do it with a define

		final_size += id_size + bone_name_size;

		// Calculating positions data size
		uint pos_count_size = sizeof(animData.boneKeys[i].positions.count);
		uint pos_time_size = sizeof(double) * animData.boneKeys[i].positions.count;
		uint pos_value_size = sizeof(float) * 3 * animData.boneKeys[i].positions.count;

		final_size += pos_count_size + pos_time_size + pos_value_size;

		// Calculating rotations data size
		uint rot_count_size = sizeof(animData.boneKeys[i].rotations.count);
		uint rot_time_size = sizeof(double) * animData.boneKeys[i].rotations.count;
		uint rot_value_size = sizeof(float) * 4 * animData.boneKeys[i].rotations.count;

		final_size += rot_count_size + rot_time_size + rot_value_size;

		// Calculating scalings data size
		uint scale_count_size = sizeof(animData.boneKeys[i].scalings.count);
		uint scale_time_size = sizeof(double) * animData.boneKeys[i].scalings.count;
		uint scale_value_size = sizeof(float) * 3 * animData.boneKeys[i].scalings.count;

		final_size += scale_count_size + scale_time_size + scale_value_size;
	}

	// -------------- SAVING ANIMATION DATA --------------

	char* data = new char[final_size];
	char* cursor = data;

	// -------------- Saving animation generic data --------------

	// Saving anim name
	uint bytes = sizeof(char)*MAX_BUF_SIZE;

	char name[MAX_BUF_SIZE];
	memset(name, 0, sizeof(char) * MAX_BUF_SIZE);
	strcpy_s(name, MAX_BUF_SIZE, animData.name.c_str());

	memcpy(cursor, name, bytes);

	// Saving duration
	cursor += bytes;
	bytes = sizeof(animData.duration);
	memcpy(cursor, &animData.duration, bytes);

	// Saving ticks per second
	cursor += bytes;
	bytes = sizeof(animData.ticksPerSecond);
	memcpy(cursor, &animData.ticksPerSecond, bytes);

	// Saving num keys
	cursor += bytes;
	bytes = sizeof(animData.numKeys);
	memcpy(cursor, &animData.numKeys, bytes);

	// -------------- Saving animation bones data for each bone --------------

	for (uint i = 0; i < animData.numKeys; i++)
	{
		// name size
		cursor += bytes;
		bytes = sizeof(uint);
		uint name_size = animData.boneKeys[i].bone_name.size();
		memcpy(cursor, &name_size, bytes);

		// name
		cursor += bytes;
		bytes = sizeof(char) * name_size + 1;
		memcpy(cursor, animData.boneKeys[i].bone_name.c_str(), bytes);

		// Saving bone position data
		cursor += bytes;
		bytes = sizeof(animData.boneKeys[i].positions.count);
		memcpy(cursor, &animData.boneKeys[i].positions.count, bytes);

		cursor += bytes;
		bytes = sizeof(double) * animData.boneKeys[i].positions.count;
		memcpy(cursor, animData.boneKeys[i].positions.time, bytes);

		cursor += bytes;
		bytes = sizeof(float) * 3 * animData.boneKeys[i].positions.count;
		memcpy(cursor, animData.boneKeys[i].positions.value, bytes);

		// Saving bone rotation data
		cursor += bytes;
		bytes = sizeof(animData.boneKeys[i].rotations.count);
		memcpy(cursor, &animData.boneKeys[i].rotations.count, bytes);

		cursor += bytes;
		bytes = sizeof(double) * animData.boneKeys[i].rotations.count;
		memcpy(cursor, animData.boneKeys[i].rotations.time, bytes);

		cursor += bytes;
		bytes = sizeof(float) * 4 * animData.boneKeys[i].rotations.count;
		memcpy(cursor, animData.boneKeys[i].rotations.value, bytes);

		// Saving bone scaling data
		cursor += bytes;
		bytes = sizeof(animData.boneKeys[i].scalings.count);
		memcpy(cursor, &animData.boneKeys[i].scalings.count, bytes);

		cursor += bytes;
		bytes = sizeof(double) * animData.boneKeys[i].scalings.count;
		memcpy(cursor, animData.boneKeys[i].scalings.time, bytes);

		cursor += bytes;
		bytes = sizeof(float) * 3 * animData.boneKeys[i].scalings.count;
		memcpy(cursor, animData.boneKeys[i].scalings.value, bytes);
	}

	// -------------- Saving animation data in file --------------

	if (App->fs->SaveInGame((char*)data, final_size, FileTypes::AnimationFile, outputFile) > 0)
		ret = true;

	/*std::string tmp_str(L_ANIMATIONS_DIR);
	tmp_str.append("/");
	tmp_str.append(std::to_string(anim_data->GetUID()));
	tmp_str.append(".trAnimation"); // Adding our own format extension

	anim_data->SetExportedPath(tmp_str.c_str());

	ret = App->file_system->WriteInFile(tmp_str.c_str(), data, final_size);
	output = tmp_str;*/

	RELEASE_ARRAY(data);

	return ret;
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
	assert(file != nullptr);

	bool ret = false;

	char* buffer;
	uint size = App->fs->Load(file, &buffer);
	if (size > 0)
	{
		char* cursor = (char*)buffer;

		// Todo Title max length
		// Load anim name
		uint bytes = sizeof(char); //*TITLE_MAX_LENGTH;
	//char name[TITLE_MAX_LENGTH];
	//memcpy(&outputAnimationData.name, cursor, bytes);

	// Load duration
		cursor += bytes;
		bytes = sizeof(outputAnimationData.duration);
		memcpy(&outputAnimationData.duration, cursor, bytes);

		// Load ticks per second
		cursor += bytes;
		bytes = sizeof(outputAnimationData.ticksPerSecond);
		memcpy(&outputAnimationData.ticksPerSecond, cursor, bytes);

		// Load amount Bone transformations
		cursor += bytes;
		bytes = sizeof(outputAnimationData.numKeys);
		memcpy(&outputAnimationData.numKeys, cursor, bytes);

		outputAnimationData.boneKeys = new BoneTransformation[outputAnimationData.numKeys];

		char buff[4096];
		for (uint i = 0; i < outputAnimationData.numKeys; ++i)
		{
			BoneTransformation* bone = &outputAnimationData.boneKeys[i];
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
			bone->positions.Init(BoneTransformation::Key::KeyType::POSITION, count);

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
			bone->rotations.Init(BoneTransformation::Key::KeyType::ROTATION, count);

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
			bone->scalings.Init(BoneTransformation::Key::KeyType::SCALE, count);

			// load position times
			cursor += bytes;
			bytes = sizeof(double) * count;
			memcpy(bone->scalings.time, cursor, bytes);

			// load position values
			cursor += bytes;
			bytes = sizeof(float) * 3 * count;
			memcpy(bone->scalings.value, cursor, bytes);
		}

		// TODO: Module Animation
		//App->animation->SetAnimationGos(resource);

		RELEASE_ARRAY(buffer);
	}
	return true;
}
