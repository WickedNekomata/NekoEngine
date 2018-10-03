#pragma once

#include "Color.h"

#include "MathGeoLib/include/Math/float2.h"
#include "MathGeoLib/include/Math/float3.h"

enum PrimitiveTypes
{
	PrimitiveNoType,
	PrimitiveTypeRay,
	PrimitiveTypeAxis,
	PrimitiveTypePlane,
	PrimitiveTypeCircle,
	PrimitiveTypeCube,
	PrimitiveTypeSphere,
	PrimitiveTypeCylinder,
	PrimitiveTypeFrustum,
};

struct Transform 
{
	// Position
	math::float3 position = { 0.0f, 0.0f, 0.0f };

	// Rotation
	float angle = 0.0f;
	math::float3 u = { 0.0f, 0.0f, 0.0f };

	// TODO: Scale
};

struct Color;
class PrimitiveAxis;

class Primitive
{
public:

	Primitive(PrimitiveTypes type = PrimitiveTypes::PrimitiveNoType);
	virtual ~Primitive();

	virtual void Render() const;

	PrimitiveTypes GetType() const { return type; }

	void SetPosition(math::float3 position);
	void SetRotation(float angle, math::float3 u); // angle in degrees
	void SetColor(Color color) { this->color = color; }

	void ShowAxis(bool showAxis) { this->showAxis = showAxis; };

protected:

	virtual void InnerRender() const;

protected:

	PrimitiveTypes type = PrimitiveTypes::PrimitiveNoType;
	Transform transform;
	Color color = White;

	bool showAxis = false;
	PrimitiveAxis* axis = nullptr;

	// Draw (Vertex Array with indices)
	float* vertices = nullptr;
	uint verticesID = 0;

	uint* indices = nullptr;
	uint indicesID = 0;
	uint indicesSize = 0;
};

// Ray --------------------------------------------------
class PrimitiveRay : public Primitive
{
public:

	PrimitiveRay(math::float3 direction = math::float3(0.0f, 0.0f, -1.0f), float length = 100.0f);

	void InnerRender() const;

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

	void InnerRender() const;

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

	math::float2 size; // x and z size
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

	PrimitiveSphere(float radius = 1.0f);
	~PrimitiveSphere();

private:

	void InnerRender() const;

private:

	float radius = 0.0f;

	PrimitiveCircle* verticalCircle = nullptr;
	PrimitiveCircle* horizontalCircle = nullptr;
};

// Cylinder --------------------------------------------------
class PrimitiveCylinder : public Primitive
{
public:

	PrimitiveCylinder(float height = 1.0f, float radius = 1.0f, uint sides = 8);
	~PrimitiveCylinder();

private:

	void InnerRender() const;

private:

	float height = 0.0f;
	float radius = 0.0f;
	uint sides = 0;

	PrimitiveCircle* topCap = nullptr;
	PrimitiveCircle* bottomCap = nullptr;
};

// Frustum --------------------------------------------------
class PrimitiveFrustum : public Primitive
{
public:

	PrimitiveFrustum(math::float2 startSize = math::float2(0.5f, 0.5f), math::float3 endPosition = math::float3(0.0f, 0.0f, -1.0f), math::float2 endSize = math::float2(1.0f, 1.0f));

private:

	math::float2 startSize = { 0.0f, 0.0f };
	math::float2 endSize = { 0.0f, 0.0f };

	math::float3 endPosition = { 0.0f, 0.0f, 0.0f };
};