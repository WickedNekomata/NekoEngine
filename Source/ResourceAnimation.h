#ifndef __RESOURCE_ANIMATION_H__
#define __RESOURCE_ANIMATION_H__

#include "Resource.h"

#include <vector>

struct ResourceAnimationData
{
	// TODO
};

class ResourceAnimation : public Resource
{

public:
	struct BoneTransformation
	{
		std::string bone_name;

		struct Key
		{
			enum KeyType {
				POSITION = 0,
				SCALE,
				ROTATION,

				UNKNOWN
			};

			void Init(KeyType type, uint count) {
				this->count = count;
				time = new double[count];

				switch (type)
				{
				case ResourceAnimation::BoneTransformation::Key::POSITION:
				case ResourceAnimation::BoneTransformation::Key::SCALE:
					value = new float[3 * count];
					break;
				case ResourceAnimation::BoneTransformation::Key::ROTATION:
					value = new float[4 * count];
					break;
				}
			}

			~Key()
			{
				RELEASE_ARRAY(time);
				RELEASE_ARRAY(value);
			}

			uint count = 0;
			double* time = nullptr;
			float* value = nullptr;
		};

		Key positions;
		Key scalings;
		Key rotations;
	};

public:

	ResourceAnimation(ResourceTypes type, uint uuid, ResourceData data, ResourceAnimationData animationData);
	~ResourceAnimation();

	bool LoadInMemory();
	bool UnloadFromMemory();

	void OnPanelAssets();

	static bool ImportFile(const char* file, std::string& name, std::string& outputFile);
	static bool ExportFile(ResourceData& data, ResourceAnimationData& prefabData, std::string& outputFile, bool overwrite = false);
	static uint CreateMeta(const char* file, uint prefab_uuid, std::string& name, std::string& outputMetaFile);
	static bool ReadMeta(const char* metaFile, int64_t& lastModTime, uint& prefab_uuid, std::string& name);
	static bool LoadFile(const char* file, ResourceAnimationData& prefab_data_output);

public:
	std::string name;
	double duration;
	double ticks_per_second;

	uint num_keys = 0;
	BoneTransformation* bone_keys;

	ResourceAnimationData animationData;
};

#endif // __RESOURCE_ANIMATION_H__