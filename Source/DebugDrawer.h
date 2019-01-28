#ifndef __DEBUG_DRAWER_H__
#define __DEBUG_DRAWER_H__

#include "Color.h"

#include "MathGeoLib\include\Geometry\AABB.h"
#include "MathGeoLib\include\Geometry\Frustum.h"
#include "MathGeoLib\include\Math\float4x4.h"

class DebugDrawer
{
public:

	DebugDrawer();
	~DebugDrawer();

	void StartDebugDraw();
	void EndDebugDraw();

	void DebugDraw(const math::AABB& aabb, const Color& color = White, const math::float4x4& globalTransform = math::float4x4::identity) const;
	void DebugDraw(const math::Frustum& frustum, const Color& color = White, const math::float4x4& globalTransform = math::float4x4::identity) const;

	void DebugDrawBox(const math::float3* vertices, const Color& color = White, const math::float4x4& globalTransform = math::float4x4::identity) const;
	void DebugDrawBox(const math::float3& halfExtents, const Color& color = White, const math::float4x4& globalTransform = math::float4x4::identity) const;
	void DebugDrawSphere(float radius, const Color& color = White, const math::float4x4& globalTransform = math::float4x4::identity) const;
	void DebugDrawCapsule(float radius, float halfHeight, const Color& color = White, const math::float4x4& globalTransform = math::float4x4::identity) const;

private:

	bool cullFace = true;
	bool lighting = true;
	bool texture2D = true;
	bool wireframeMode = false;
};

#endif