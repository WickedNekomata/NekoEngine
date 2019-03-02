#include "ComponentAudioSource.h"

#include "Application.h"
#include "ModuleAudio.h"
#include "ComponentTransform.h"
#include "GameObject.h"
#include "MathGeoLib/include/Math/Quat.h"
#include "MathGeoLib/include/Math/float3.h"

#include "imgui\imgui.h"
#include "imgui\imgui_internal.h"

ComponentAudioSource::ComponentAudioSource(GameObject* parent) : Component(parent, ComponentTypes::AudioSourceComponent)
{
	source = App->audio->CreateSoundEmitter("");
	App->audio->audio_sources.push_back(this);
	memset(audio_to_play, '\0', DEFAULT_BUF_SIZE);
}

ComponentAudioSource::ComponentAudioSource(const ComponentAudioSource& componentAudioSource) : Component(componentAudioSource.parent, ComponentTypes::AudioSourceComponent)
{
	source = App->audio->CreateSoundEmitter("");
	App->audio->audio_sources.push_back(this);
	memset(audio_to_play, '\0', DEFAULT_BUF_SIZE);
}

ComponentAudioSource::~ComponentAudioSource()
{
	App->audio->audio_sources.remove(this);
	parent->cmp_audioSource = nullptr;
	//RELEASE(source);
}

void ComponentAudioSource::Update()
{
	UpdateSourcePos();
}

void ComponentAudioSource::UpdateSourcePos()
{
	ComponentTransform* transformation = (ComponentTransform*)parent->GetComponent(ComponentTypes::TransformComponent);

	if (transformation != nullptr)
	{
		math::Quat rot = transformation->rotation;

		math::float3 vector_pos = transformation->position;
		math::float3 vector_front = rot.Transform(math::float3(0, 0, 1));
		math::float3 vector_up = rot.Transform(math::float3(0, 1, 0));

		source->SetPos(vector_pos.x, vector_pos.y, vector_pos.z, vector_front.x, vector_front.y, vector_front.z, vector_up.x, vector_up.y, vector_up.z);
	}
}

void ComponentAudioSource::OnUniqueEditor()
{
#ifndef GAMEMODE

	ImGui::Text("AudioClip");
	if (ImGui::InputText("", audio_to_play, DEFAULT_BUF_SIZE, ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue)) {
		SetAudio(audio_to_play);
	}
	
	if (ImGui::Button("Play")) {
		PlayAudio();
	}
	ImGui::SameLine();
	ImGui::SameLine();
	if (ImGui::Button("Stop")) {
		StopAudio();
	}

	bool muted = isMuted();
	if (ImGui::Checkbox("Mute", &muted)) {
		SetMuted(muted);
	}

	bool play_awake = GetPlayOnAwake();
	if (ImGui::Checkbox("Play On Awake", &play_awake)) {
		SetPlayOnAwake(play_awake);
	}

	int prior = GetPriority();
	if (ImGui::SliderInt("Priority", &prior, 1, 100)) {
		SetPriority(prior);
	}

	float vol = GetVolume();
	if (ImGui::SliderFloat("Volume", &vol, 0, 10)) {
		SetVolume(vol);
	}

	float audio_pitch = GetPitch();
	if (ImGui::SliderFloat("Pitch", &audio_pitch, 0, 15)) {
		SetPitch(audio_pitch);
	}

	ImGui::Separator();
	ImGui::Text("Only for 2D tracks:");
	ImGui::Separator();

	int pan_l = GetStereoPanLeft();
	if (ImGui::SliderInt("Pan Left difference", &pan_l, 0, 100)) {
		SetStereoPanLeft(pan_l);
	}
	int pan_r = GetStereoPanRight();
	if (ImGui::SliderInt("Pan Right difference", &pan_r, 0, 100)) {
		SetStereoPanRight(pan_r);
	}

#endif // !GAMEMODE

}

//Getters
std::string ComponentAudioSource::GetAudioToPlay() const
{
	return audio_to_play;
}
bool ComponentAudioSource::isMuted()const
{
	return mute;
}
bool ComponentAudioSource::GetBypassEffects()const
{
	return bypass_effects;
}
bool ComponentAudioSource::GetPlayOnAwake()const
{
	return play_on_awake;
}
bool ComponentAudioSource::isInLoop()const
{
	return loop;
}
int ComponentAudioSource::GetPriority()const
{
	return priority;
}
float ComponentAudioSource::GetVolume()const
{
	return volume;
}
bool ComponentAudioSource::isMono()const
{
	return mono;
}
float ComponentAudioSource::GetPitch()const
{
	return pitch;
}
int ComponentAudioSource::GetStereoPanLeft()const
{
	return stereo_pan_l;
}
int ComponentAudioSource::GetStereoPanRight()const
{
	return stereo_pan_r;
}
float ComponentAudioSource::GetMinDistance()const
{
	return min_distance;
}
float ComponentAudioSource::GetMaxDistance()const
{
	return max_distance;
}
int ComponentAudioSource::GetState()const
{
	return current_state;
}

//Setters
void ComponentAudioSource::SetAudio(const char* audio)
{
	strcpy(audio_to_play, audio);
}
void ComponentAudioSource::SetMuted(bool must_mute)
{
	mute = must_mute;
	if (mute == true)
	{
		source->SetVolume(0);
	}
	else {
		source->SetVolume(volume);
	}
}
void ComponentAudioSource::SetBypassEffects(bool must_bypass_effects)
{
	bypass_effects = must_bypass_effects;
}
void ComponentAudioSource::SetPlayOnAwake(bool must_play_on_awake)
{
	play_on_awake = must_play_on_awake;
}
void ComponentAudioSource::SetLoop(bool must_loop)
{
	loop = must_loop;
}
void ComponentAudioSource::SetPriority(int desired_priority)
{
	priority = desired_priority;
}
void ComponentAudioSource::SetVolume(float desired_volume)
{
	volume = desired_volume;
	if (mute == false) {
		source->SetVolume(desired_volume);
	}
}
void ComponentAudioSource::SetMono(bool must_mono)
{
	mono = must_mono;
	if (mono == true) {
		source->SetMono();
	}
	else {
		source->SetStereo();
	}
}
void ComponentAudioSource::SetPitch(float desired_pitch)
{
	pitch = desired_pitch;
	source->SetPitch(pitch);
}
void ComponentAudioSource::SetStereoPanLeft(float desired_stereo_pan)
{
	stereo_pan_l = desired_stereo_pan;
	source->SetPanLeft(desired_stereo_pan);
}

void ComponentAudioSource::SetStereoPanRight(float desired_stereo_pan)
{
	stereo_pan_r = desired_stereo_pan;
	source->SetPanRight(desired_stereo_pan);
}
void ComponentAudioSource::SetMinDistance(float desired_min_distance)
{
	min_distance = desired_min_distance;
}
void ComponentAudioSource::SetMaxDistance(float desired_max_distance)
{
	max_distance = desired_max_distance;
}

void ComponentAudioSource::SetState(int new_state)
{
	current_state = new_state;
}

//Audio
void ComponentAudioSource::PlayAudio()
{
	this->StopAudio();
	source->PlayEventByName(audio_to_play);
}

void ComponentAudioSource::PauseAudio()
{
	source->PauseEventByName(audio_to_play);
}

void ComponentAudioSource::ResumeAudio()
{
	source->ResumeEventByName(audio_to_play);
}

void ComponentAudioSource::StopAudio()
{
	source->StopEventByName(audio_to_play);
}

uint ComponentAudioSource::GetInternalSerializationBytes()
{
	return sizeof(WwiseT::AudioSource*) + (sizeof(char) * DEFAULT_BUF_SIZE) + (sizeof(bool) * 5) + (sizeof(int) * 2) + (sizeof(float) * 6);
}

void ComponentAudioSource::OnInternalSave(char*& cursor)
{
	size_t bytes = sizeof(WwiseT::AudioSource*);
	memcpy(cursor, &source, bytes);
	cursor += bytes;

	bytes = sizeof(char)*DEFAULT_BUF_SIZE;
	memcpy(cursor, &audio_to_play, bytes);
	cursor += bytes;

	bytes = sizeof(bool);
	memcpy(cursor, &mute, bytes);
	cursor += bytes;

	bytes = sizeof(bool);
	memcpy(cursor, &bypass_effects, bytes);
	cursor += bytes;

	bytes = sizeof(bool);
	memcpy(cursor, &play_on_awake, bytes);
	cursor += bytes;

	bytes = sizeof(bool);
	memcpy(cursor, &loop, bytes);
	cursor += bytes;

	bytes = sizeof(int);
	memcpy(cursor, &priority, bytes);
	cursor += bytes;

	bytes = sizeof(float);
	memcpy(cursor, &volume, bytes);
	cursor += bytes;

	bytes = sizeof(bool);
	memcpy(cursor, &mono, bytes);
	cursor += bytes;

	bytes = sizeof(float);
	memcpy(cursor, &pitch, bytes);
	cursor += bytes;

	bytes = sizeof(float);
	memcpy(cursor, &stereo_pan_l, bytes);
	cursor += bytes;

	bytes = sizeof(float);
	memcpy(cursor, &stereo_pan_r, bytes);
	cursor += bytes;

	bytes = sizeof(float);
	memcpy(cursor, &min_distance, bytes);
	cursor += bytes;

	bytes = sizeof(float);
	memcpy(cursor, &max_distance, bytes);
	cursor += bytes;

	bytes = sizeof(int);
	memcpy(cursor, &current_state, bytes);
	cursor += bytes;

}

void ComponentAudioSource::OnInternalLoad(char*& cursor)
{
	size_t bytes = sizeof(WwiseT::AudioSource*);
	memcpy(&source, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(char) * DEFAULT_BUF_SIZE;
	memcpy(&audio_to_play, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(bool);
	memcpy(&mute, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(bool);
	memcpy(&bypass_effects, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(bool);
	memcpy(&play_on_awake, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(bool);
	memcpy(&loop, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(int);
	memcpy(&priority, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(float);
	memcpy(&volume, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(bool);
	memcpy(&mono, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(float);
	memcpy(&pitch, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(float);
	memcpy(&stereo_pan_l, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(float);
	memcpy(&stereo_pan_r, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(float);
	memcpy(&min_distance, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(float);
	memcpy(&max_distance, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(int);
	memcpy(&current_state, cursor, bytes);
	cursor += bytes;
}