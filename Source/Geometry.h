#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

#include "MathGeoLib/include/Math/float3.h"

enum GeometryTypes
{
	GeometryTypeSphere,
	GeometryTypeCapsule,
	GeometryTypeBox
};

struct GeometrySphere
{
	GeometryTypes geometryType;

	float radius;
};

struct GeometryCapsule
{
	GeometryTypes geometryType;

	float radius;
	float halfHeight;
};

struct GeometryBox
{
	GeometryTypes geometryType;

	math::float3 halfExtents;
};

union Geometry
{
	Geometry() {}

	GeometryTypes geometryType;

	GeometrySphere geometrySphere;
	GeometryCapsule geometryCapsule;
	GeometryBox geometryBox;
};

#endif