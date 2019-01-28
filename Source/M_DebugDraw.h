#ifndef __M_DEBUG_DRAW_H__
#define __M_DEBUG_DRAW_H__

#include "Recast&Detour/DebugUtils/Include/DebugDraw.h"

class M_DebugDraw : public duDebugDraw
{
public:
	virtual void depthMask(bool state);
	virtual void texture(bool state) {};
	virtual void begin(duDebugDrawPrimitives prim, float size = 1.0f);
	virtual void vertex(const float* pos, unsigned int color);
	virtual void vertex(const float x, const float y, const float z, unsigned int color);
	virtual void vertex(const float* pos, unsigned int color, const float* uv);
	virtual void vertex(const float x, const float y, const float z, unsigned int color, const float u, const float v);
	virtual void end();
};

#endif