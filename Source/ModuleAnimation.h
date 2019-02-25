#ifndef __MODULE_ANIMATION_H__
#define __MODULE_ANIMATION_H__

#include "Module.h"
#include "Globals.h"
#include "MathGeoLib/include/MathGeoLib.h"
#include "ResourceAnimation.h"

#include <vector>
#include <map>
#include <list>

class GameObject;
class ComponentBone;

enum AnimationState
{
	NOT_DEF_STATE = -1,
	PLAYING,
	PAUSED,
	STOPPED,
	BLENDING
};

class ModuleAnimation : public Module
{
public:

	struct Animation {
		std::string name;
		std::vector<GameObject*> animable_gos;
		std::map<GameObject*, BoneTransformation*> animable_data_map;

		bool loop = false;
		bool interpolate = false;
		float anim_speed = 1.0f;

		float anim_timer = 0.0f;
		float duration = 0.0f;
	};

	std::vector<Animation*> animations;

public:

	ModuleAnimation();
	~ModuleAnimation();

	// Called before render is available
	bool Awake(JSON_Object* config = nullptr);

	// Called before the first frame
	bool Start();
	bool CleanUp();
	bool Update(float dt);

	void SetAnimationGos(ResourceAnimation* res);
	void DeformMesh(ComponentBone* component_bone);
	void ResetMesh(ComponentBone* component_bone);

	float GetCurrentAnimationTime() const;
	const char* GetAnimationName(int index) const;
	uint GetAnimationsNumber() const;
	Animation* GetCurrentAnimation() const;

	void SetCurrentAnimationTime(float time);
	void SetCurrentAnimation(int i);

	void CleanAnimableGOS();

	void PlayAnimation();
	void PauseAnimation();
	void StopAnimation();
	void StepBackwards();
	void StepForward();

private:

	void RecursiveGetAnimableGO(GameObject* go, BoneTransformation* bone_transformation, Animation* animation);
	void MoveAnimationForward(float t, Animation* current_animation, float blend = 1.0f);

private:

	Animation* current_anim = nullptr;
	Animation* last_anim = nullptr;

	float blend_timer = 0.0f;
	std::vector<ResourceAnimation*> available_animations;

public:
	AnimationState anim_state = AnimationState::NOT_DEF_STATE;

};

#endif // __ANIMATION_H__
