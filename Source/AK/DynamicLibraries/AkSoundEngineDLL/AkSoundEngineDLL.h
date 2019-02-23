/*******************************************************************************
The content of this file includes portions of the AUDIOKINETIC Wwise Technology
released in source code form as part of the SDK installer package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use this file in accordance with the end user license agreement provided 
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.

  Version: v2017.2.6  Build: 6636
  Copyright (c) 2006-2018 Audiokinetic Inc.
*******************************************************************************/

#ifndef AK_SOUND_ENGINE_DLL_H_
#define AK_SOUND_ENGINE_DLL_H_

#include <AK/SoundEngine/Common/AkSoundEngine.h>
#include <AK/MusicEngine/Common/AkMusicEngine.h>
#include <AK/SoundEngine/Common/AkModule.h>
#include <AK/SoundEngine/Common/AkStreamMgrModule.h>

namespace AK
{
	namespace SOUNDENGINE_DLL
    {
		AKSOUNDENGINE_API AKRESULT Init(
            AkMemSettings *     in_pMemSettings,
            AkStreamMgrSettings *  in_pStmSettings,
            AkDeviceSettings *  in_pDefaultDeviceSettings,
            AkInitSettings *    in_pSettings,
            AkPlatformInitSettings * in_pPlatformSettings,
			AkMusicSettings *	in_pMusicSettings
            );
		AKSOUNDENGINE_API void     Term();

		AKSOUNDENGINE_API void     Tick();

        // File system interface.
		AKSOUNDENGINE_API AKRESULT SetBasePath(
			const AkOSChar*   in_pszBasePath
			);
		AKSOUNDENGINE_API AKRESULT SetBankPath(
			const AkOSChar*   in_pszBankPath
			);
		AKSOUNDENGINE_API AKRESULT SetAudioSrcPath(
			const AkOSChar*   in_pszAudioSrcPath
			);
		AKSOUNDENGINE_API AKRESULT SetLangSpecificDirName(
			const AkOSChar*   in_pszDirName
			);

		AKSOUNDENGINE_API AKRESULT InitPlugins();
    }
}

#endif //AK_SOUND_ENGINE_DLL_H_
