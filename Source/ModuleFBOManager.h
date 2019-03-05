#ifndef __FBOMANAGER
#define __FBOMANAGER

#include "Module.h"

class ModuleFBOManager : public Module
{
public:

	ModuleFBOManager();
	~ModuleFBOManager();
	bool Start();
	bool CleanUp();

	void LoadGBuffer();
	void BindGBuffer();
	void DrawGBufferToScreen() const;
	void MergeDepthBuffer();

public:

	uint gBuffer;
	uint gPosition;
	uint gNormal;
	uint gAlbedoSpec;
	uint rboDepth;
};

#endif
