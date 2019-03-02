#ifndef __MODULE_AUDIO_H__
#define __MODULE_AUDIO_H__

#include "Module.h"
#include "WwiseT.h"
#include <list>

struct ComponentAudioSource;

class ModuleAudio : public Module
{
public:
	ModuleAudio(bool start_enabled = true);
	~ModuleAudio();
	bool Start();
	update_status Update(/*float dt*/);
	update_status PostUpdate(/*float dt*/);
	bool CleanUp();

	// Set listener
	WwiseT::AudioSource* CreateSoundEmitter(const char * name);
	uint GetListenerID()const;

	// Utils
	void PlayOnAwake()const;
	void Stop()const;
	void Pause()const;
	void Resume()const;
	void SetListener(WwiseT::AudioSource* new_listener);

	void OnSystemEvent(System_Event event);

private:
	std::list<WwiseT::AudioSource*> event_list;
	WwiseT::AudioSource* listener;

public:
	std::list<ComponentAudioSource*> audio_sources;
};

#endif // !__MODULE_AUDIO_H__