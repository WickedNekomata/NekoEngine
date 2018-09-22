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
	void			SetRotation(float angle, const float3 &u);
	void			Scale(float x, float y, float z);


public:
	
	Color color;
	float4x4 transform;
	bool axis,wire;

protected:

	PrimitiveTypes type;
};

// ============================================
class Cube : public Primitive
{
public:

	Cube();
	Cube(float sizeX, float sizeY, float sizeZ);
	void InnerRender() const;

public:

	float3 size;
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

	float3 origin;
	float3 destination;
};

// ============================================
class pPlane : public Primitive
{
public:

	pPlane();
	pPlane(float x, float y, float z, float d);
	void InnerRender() const;

public:

	float3 normal;
	float constant;
};