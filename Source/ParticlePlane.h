#ifndef __ParticlePlane_H__
#define __ParticlePlane_H__

#include "Globals.h"
#include "MathGeoLib/Math/float3.h"

class ResourceTexture;

class ParticlePlane
{
public:
	ParticlePlane();
	~ParticlePlane();
	void LoadPlaneBuffers();
	void Render(float4x4 matrix, ResourceTexture* texture, uint textureUV, float4 color) const;

private:
	void DrawPlane(ResourceTexture* texture, uint textureUV, float4 &color) const;

public:
	uint indexID = 0u;
	uint vertexID = 0u;
};
#endif //!__ParticlePlane__