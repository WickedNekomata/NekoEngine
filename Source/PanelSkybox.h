#ifndef __PANEL_SKYBOX_H__
#define __PANEL_SKYBOX_H__

#include "Panel.h"

#ifndef GAMEMODE

#include <vector>
#include "Globals.h"

class ResourceTexture;

class PanelSkybox : public Panel
{
public:

	PanelSkybox(char* name);
	~PanelSkybox();

	bool Draw();

	std::vector<ResourceTexture*> cubemapTextures;
};

#endif

#endif
