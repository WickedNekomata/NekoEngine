#pragma once

#include "Color.h"

#include "MathGeoLib/include/Math/float3.h"
#include "MathGeoLib/include/Math/float4x4.h"

enum PrimitiveTypes
{
	Primitive_Point,
	Primitive_Line,
	Primitive_Plane,
	Primitive_Cube,
	Primitive_Sphere,
	Primitive_Cylinder
};

class Primitive
{
public:

	Primitive();

	virtual void	Render() const;
	virtual void	InnerRender() const;
	PrimitiveTypes	GetType() const;
	void			SetPos(float x, float y, float z);
	void			SetRotation(float angle, const math::float3 &u);
	void			Scale(float x, float y, float z);


public:
	
	Color color;
	math::float4x4 transform;
	bool axis,wire;

protected:

	PrimitiveTypes type;
};

// ============================================
class pCube : public Primitive
{
public:

	pCube();
	pCube(float sizeX, float sizeY, float sizeZ);
	void InnerRender() const;

public:

	math::float3 size;
};

// ============================================
class pSphere : public Primitive
{
public:

	pSphere();
	pSphere(float radius);
	void InnerRender() const;

public:

	float radius;
};

// ============================================
class pCylinder : public Primitive
{
public:

	pCylinder();
	pCylinder(float radius, float height);
	void InnerRender() const;

public:

	float radius;
	float height;
};

// ============================================
class pLine : public Primitive
{
public:

	pLine();
	pLine(float x, float y, float z);
	void InnerRender() const;

public:

	math::float3 origin;
	math::float3 destination;
};

// ============================================
class pPlane : public Primitive
{
public:

	pPlane();
	pPlane(float x, float y, float z, float d);
	void InnerRender() const;

public:

	math::float3 normal;
	float constant;
};