#ifndef __ANIMATION_IMPORTER_H__
#define __ANIMATION_IMPORTER_H__

#include "Globals.h"
#include "ResourceAnimation.h"

struct aiAnimation;
struct aiNodeAnim;

class ResourceAnimation;

class AnimationImporter
{
public:

	AnimationImporter();
	~AnimationImporter();

	uint GenerateResourceFromFile(const char* file_path, uint uid_to_force = 0u);

	void Load(mutable const char* file_path, mutable ResourceData& data, mutable ResourceAnimationData& anim_data, mutable uint uid_to_force = 0u);

	uint Import(const aiAnimation* new_anim, std::string& output);

	bool SaveAnimation(ResourceAnimation* anim, std::string& output);

	void ImportBoneTransform(const aiNodeAnim * anim_node, BoneTransformation& bones_transform) const;

	bool EqualsWithEpsilon(float number_a, float number_b, float epsilon = 1e-3f) const;


};

#endif // __BONE_IMPORTER_H__