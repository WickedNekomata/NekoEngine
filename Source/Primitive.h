#ifndef __PRIMITIVE_H__
#define __PRIMITIVE_H__

#include "Color.h"
#include "Globals.h"

#include "MathGeoLib\include\Math\float2.h"
#include "MathGeoLib\include\Math\float3.h"
#include "MathGeoLib\include\Math\float4x4.h"

enum PrimitiveTypes
{
	PrimitiveNoType,
	PrimitiveTypeRay,
	PrimitiveTypeAxis,
	PrimitiveTypePlane,
	PrimitiveTypeGrid,
	PrimitiveTypeCircle,
	PrimitiveTypeCube,
	PrimitiveTypeSphere,
	PrimitiveTypeCylinder,
	PrimitiveTypeCone,
	PrimitiveTypeArrow,
	PrimitiveTypeFrustum,
};

struct Color;
class PrimitiveAxis;

class Primitive
{
public:

	Primitive(PrimitiveTypes type = PrimitiveTypes::PrimitiveNoType);
	virtual ~Primitive();

	void Render(const math::float4x4 globalTransform = math::float4x4::identity) const;

	// -----

	PrimitiveTypes GetType() const;

	void SetLocalTransform(const math::float4x4 localTransform);
	void SetColor(Color color);
	void SetWireframeMode(bool wireframeMode);
	void ShowAxis(bool showAxis);

protected:

	virtual void InnerRender(const math::float4x4 globalTransform) const;

protected:

	PrimitiveTypes type = PrimitiveTypes::PrimitiveNoType;

	math::float4x4 localTransform = math::float4x4::identity;
	Color color = White;
	bool wireframeMode = false;
	bool showAxis = false;
	PrimitiveAxis* axis = nullptr;

	float* vertices = nullptr;
	uint verticesID = 0;
	uint verticesSize = 0;

	uint* indices = nullptr;
	uint indicesID = 0;
	uint indicesSize = 0;
};

// Ray --------------------------------------------------
class PrimitiveRay : public Primitive
{
public:

	PrimitiveRay(math::float3 direction = math::float3(0.0f, 0.0f, -1.0f), float length = 100.0f);

private:
	
	void InnerRender(const math::float4x4 transform) const;

private:

	math::float3 direction = { 0.0f, 0.0f, 0.0f };
	float length = 0.0f;
};

// Axis --------------------------------------------------
class PrimitiveAxis : public Primitive
{
public:

	PrimitiveAxis();
	~PrimitiveAxis();

private:

	void InnerRender(const math::float4x4 transform) const;

private:

	PrimitiveRay* x = nullptr;
	PrimitiveRay* y = nullptr;
	PrimitiveRay* z = nullptr;
};

// Circle --------------------------------------------------
class PrimitiveCircle : public Primitive
{
public:

	PrimitiveCircle(float radius = 1.0f, uint sides = 8);

private:

	float radius = 0.0f;
	uint sides = 0;
};

// Plane --------------------------------------------------
class PrimitivePlane : public Primitive
{
public:

	PrimitivePlane(math::float2 size = math::float2(100.0f, 100.0f));

private:

	math::float2 size = { 0.0f,0.0f }; // x and z size
};

// Grid --------------------------------------------------
class PrimitiveGrid : public Primitive
{
public:

	PrimitiveGrid(uint quadSize = 1, uint quadsX = 100, uint quadsZ = 100);

private:

	void InnerRender(const math::float4x4 transform) const;

private:

	uint quadSize = 0;
	uint quadsX = 0;
	uint quadsZ = 0;
};

// Cube --------------------------------------------------
class PrimitiveCube : public Primitive
{
public:

	PrimitiveCube(math::float3 size = math::float3(1.0f, 1.0f, 1.0f));

private:

	math::float3 size = { 0.0f, 0.0f, 0.0f };
};

// Sphere --------------------------------------------------
class PrimitiveSphere : public Primitive
{
public:

	PrimitiveSphere(float radius = 1.0f, uint segments = 8);

private:

	float radius = 0.0f;
	uint segments = 0;
};

// Cylinder --------------------------------------------------
class PrimitiveCylinder : public Primitive
{
public:

	PrimitiveCylinder(float height = 5.0f, float radius = 1.0f, uint sides = 8);

private:

	float height = 0.0f;
	float radius = 0.0f;
	uint sides = 0;
};

// Cone --------------------------------------------------
class PrimitiveCone : public Primitive
{
public:

	PrimitiveCone(float height = 5.0f, float radius = 1.0f, uint sides = 5);

private:

	float height = 0.0f;
	float radius = 0.0f;
	uint sides = 0;
};

// Arrow --------------------------------------------------
class PrimitiveArrow : public Primitive
{
public:

	PrimitiveArrow(float lineLength = 5.0f, float coneHeight = 1.0f, float coneRadius = 1.0f, uint coneSides = 8);
	~PrimitiveArrow();

private:

	void InnerRender(const math::float4x4 transform) const;

private:

	PrimitiveCone* cone = nullptr;
	PrimitiveRay* line = nullptr;
};

// Frustum --------------------------------------------------
class PrimitiveFrustum : public Primitive
{
public:

	PrimitiveFrustum(math::float2 startSize = math::float2(0.5f, 0.5f), math::float2 endSize = math::float2(1.0f, 1.0f), math::float3 endPosition = math::float3(0.0f, 0.0f, -1.0f));

private:

	math::float2 startSize = { 0.0f, 0.0f };
	math::float2 endSize = { 0.0f, 0.0f };

	math::float3 endPosition = { 0.0f, 0.0f, 0.0f };
};

#endif