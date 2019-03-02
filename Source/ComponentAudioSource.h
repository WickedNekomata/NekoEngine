#ifndef __COMPONENT_AUDIO_SOURCE_H__
#define __COMPONENT_AUDIO_SOURCE_H__

#include "Component.h"
#include "Globals.h"
#include "WwiseT.h"
#include <string>

class ComponentAudioSource : public Component
{

public:
	ComponentAudioSource(GameObject* parent);
	ComponentAudioSource(const ComponentAudioSource& componentAudioSource);
	~ComponentAudioSource();
	void Update();

	void UpdateSourcePos();

	//Getters
	std::string GetAudioToPlay() const;
	bool isMuted()const;
	bool GetBypassEffects()const;
	bool GetPlayOnAwake()const;
	bool isInLoop()const;
	int GetPriority()const;
	float GetVolume()const;
	bool isMono()const;
	float GetPitch()const;
	int GetStereoPanLeft()const;
	int GetStereoPanRight()const;
	float GetMinDistance()const;
	float GetMaxDistance()const;
	int GetState()const;

	//Setters
	void SetAudio(const char* audio);
	void SetMuted(bool must_mute);
	void SetBypassEffects(bool must_bypass_effects);
	void SetPlayOnAwake(bool must_play_on_awake);
	void SetLoop(bool must_loop);
	void SetPriority(int desired_priority);
	void SetVolume(float desired_volume);
	void SetMono(bool must_mono);
	void SetPitch(float desired_pitch);
	void SetStereoPanLeft(float desired_stereo_pan);
	void SetStereoPanRight(float desired_stereo_pan);
	void SetMinDistance(float desired_min_distance);
	void SetMaxDistance(float desired_max_distance);
	void SetState(int new_state);

	//Audio
	void PlayAudio();
	void PauseAudio();
	void ResumeAudio();
	void StopAudio();

	//Serialization
	uint GetInternalSerializationBytes();
	virtual void OnInternalSave(char*& cursor);
	virtual void OnInternalLoad(char*& cursor);

public:
	WwiseT::AudioSource* source;

private:
	bool mute = false;
	bool bypass_effects = true;
	bool play_on_awake = true;
	bool loop = false;
	int priority = 50;
	float volume = 1;
	bool mono = false;
	float pitch = 0.0f;
	float stereo_pan_l = 0.0f;
	float stereo_pan_r = 0.0f;
	float min_distance = 1.0f;
	float max_distance = 500.0f;
	std::string audio_to_play;
	int current_state = 1;

};


#endif // !__COMPONENT_AUDIO_SOURCE_H__#pragma once