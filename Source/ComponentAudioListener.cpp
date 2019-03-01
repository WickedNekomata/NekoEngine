#include "ComponentAudioListener.h"

#include "Application.h"
#include "ModuleAudio.h"
#include "ComponentTransform.h"
#include "GameObject.h"
#include "MathGeoLib/include/Math/Quat.h"
#include "MathGeoLib/include/Math/float3.h"

ComponentAudioListener::ComponentAudioListener(GameObject* parent) : Component(parent, ComponentTypes::AudioListenerComponent) 
{
	listener = App->audio->CreateSoundEmitter("listener");
	App->audio->SetListener(listener);
}

ComponentAudioListener::ComponentAudioListener(const ComponentAudioListener& componentAudioListener) : Component(componentAudioListener.parent, ComponentTypes::AudioListenerComponent)
{
	listener = App->audio->CreateSoundEmitter("listener");
	App->audio->SetListener(listener);
}

ComponentAudioListener::~ComponentAudioListener()
{
	parent->cmp_audioListener = nullptr;
	RELEASE(listener);
}

void ComponentAudioListener::Update()
{
	UpdateListenerPos();
}

void ComponentAudioListener::UpdateListenerPos() {
	ComponentTransform* transformation = (ComponentTransform*)parent->GetComponent(ComponentTypes::TransformComponent);

	if (transformation != nullptr)
	{
		math::Quat rot = transformation->rotation;

		math::float3 vector_pos = transformation->position;
		math::float3 vector_front = rot.Transform(math::float3(0, 0, 1));
		math::float3 vector_up = rot.Transform(math::float3(0, 1, 0));

		listener->SetPos(vector_pos.x, vector_pos.y, vector_pos.z, vector_front.x, vector_front.y, vector_front.z, vector_up.x, vector_up.y, vector_up.z);
	}
}

uint ComponentAudioListener::GetInternalSerializationBytes()
{
	return sizeof(WwiseT::AudioSource*);
}

void ComponentAudioListener::OnInternalSave(char*& cursor)
{
	size_t bytes = sizeof(WwiseT::AudioSource*);
	memcpy(cursor, &listener, bytes);
	cursor += bytes;
}

void ComponentAudioListener::OnInternalLoad(char*& cursor)
{
	size_t bytes = sizeof(WwiseT::AudioSource*);
	memcpy(&listener, cursor, bytes);
	cursor += bytes;
}