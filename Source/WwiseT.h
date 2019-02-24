#ifndef WWISET_H_
#define WWISET_H_

#include "Globals.h"
#include "AK\Common\AkTypes.h"
//#include <AK\SoundEngine\Platforms\Windows\AkTypes.h>

// Wwise libs
#ifndef _DEBUG  // Profile build configuration must be loaded instead of Debug
#define AK_OPTIMIZED
#pragma comment( lib, "AK/Release(StaticCRT)/lib/AkSoundEngine.lib")
#pragma comment( lib, "AK/Release(StaticCRT)/lib/AkMusicEngine.lib")
#pragma comment( lib, "AK/Release(StaticCRT)/lib/AkMemoryMgr.lib")
#pragma comment( lib, "AK/Release(StaticCRT)/lib/AkStreamMgr.lib")
#pragma comment( lib, "AK/Debug(StaticCRT)/lib/AkRoomVerbFX.lib")

#else
#include <AK/Comm/AkCommunication.h>
#pragma comment( lib, "AK/Debug(StaticCRT)/lib/CommunicationCentral.lib" )
#pragma comment( lib, "AK/ws2_32.lib") 
#pragma comment( lib, "AK/Debug(StaticCRT)/lib/AkSoundEngine.lib" )
#pragma comment( lib, "AK/Debug(StaticCRT)/lib/AkMusicEngine.lib" )
#pragma comment( lib, "AK/Debug(StaticCRT)/lib/AkMemoryMgr.lib" )
#pragma comment( lib, "AK/Debug(StaticCRT)/lib/AkStreamMgr.lib" )
#pragma comment( lib, "AK/Debug(StaticCRT)/lib/AkRoomVerbFX.lib")

#endif

#pragma comment( lib, "AK/dinput8.lib" )
#pragma comment( lib, "AK/dsound.lib" ) 
#pragma comment( lib, "AK/dxguid.lib" )

// Wwise memory hooks
namespace AK
{
#ifdef WIN32
	void * AllocHook(size_t in_size);
	void FreeHook(void * in_ptr);

	void * VirtualAllocHook(void * in_pMemAddress, size_t in_size, DWORD in_dwAllocationType, DWORD in_dwProtect);
	void VirtualFreeHook(void * in_pMemAddress, size_t in_size, DWORD in_dwFreeType);
#endif
}

// Wwise functions
namespace WwiseT
{
	class AudioSource
	{
	public:
		AudioSource(const char* name);
		AudioSource(uint id, const char* name);
		~AudioSource();

		// Events
		void PlayEventByName(const char* name);
		void StopEventByName(const char* name);
		void PauseEventByName(const char* name);
		void ResumeEventByName(const char* name);
		void ApplyEnvReverb(AkReal32 desired_level, const char * target);
		void ChangeState(const char* group_name, const char* new_state);
		// Seters
		void SetVolume(float new_volume);
		void SetPanLeft(float value);
		void SetPanRight(float value);
		void SetMono();
		void SetStereo();
		void SetPitch(float value);
		void SetListener(uint listener_id);
		void SetPos(float pos_x, float pos_y, float pos_z, float front_rot_x, float front_rot_y, float front_rot_z, float top_rot_x, float top_rot_y, float top_rot_z);
		// Geters
		uint GetID()const;
		const char* GetName()const;

	private:
		uint id;
		const char* name = nullptr; // Event name
		AkSoundPosition position;
	};

	bool InitSoundEngine();
	bool CloseSoundEngine(); // Close wwise
	void ProcessAudio();
	void LoadBank(const char* path);
	void SetDefaultListener(uint id);
	// Utils
	AudioSource* CreateAudSource(const char* name);
	AudioSource* CreateAudSource(uint id, const char* name);
	void StopAllEvents();
	void PauseAll();
	void ResumeAll();
}
// Make wwise methods here :)
// We will access all the functions in the engine from here

#endif