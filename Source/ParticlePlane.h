#ifndef __ParticlePlane_H__
#define __ParticlePlane_H__

#include "Globals.h"

class ResourceTexture;

class ParticlePlane
{
public:
	ParticlePlane();
	~ParticlePlane();
	void LoadPlaneBuffers();
	void Render(math::float4x4 matrix, ResourceTexture* texture, uint textureUV, math::float4 color) const;

private:
	void DrawPlane(ResourceTexture* texture, uint textureUV, math::float4 &color) const;

public:
	uint indexID = 0u;
	uint vertexID = 0u;
};
#endif //!__ParticlePlane__