#include "Globals.h"
#include "glew/include/GL/glew.h"
#include "Primitive.h"
#include "Application.h"
#include "ModuleRenderer3D.h"

#include "MathGeoLib/include/Math/TransformOps.h"
#include "MathGeoLib/include/Math/MathConstants.h"

Primitive::Primitive(PrimitiveTypes type) : type(type) {}

Primitive::~Primitive()
{
	RELEASE_ARRAY(vertices);
	RELEASE_ARRAY(indices);

	RELEASE(axis);
}

void Primitive::Render() const
{
	InnerRender();

	if (showAxis && axis != nullptr)
		axis->Render();
}

void Primitive::InnerRender() const
{
	glEnableClientState(GL_VERTEX_ARRAY);

	// Array Buffer
	glBindBuffer(GL_ARRAY_BUFFER, verticesID);

	glColor3f(color.r, color.g, color.b);
	glTranslatef(transform.position.x, transform.position.y, transform.position.z);
	glRotatef(transform.angle, transform.u.x, transform.u.y, transform.u.z);

	glVertexPointer(3, GL_FLOAT, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//_Array_Buffer

	// Element Array Buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesID);
	glDrawElements(GL_TRIANGLES, indicesSize, GL_UNSIGNED_INT, NULL);

	glRotatef(-transform.angle, transform.u.x, transform.u.y, transform.u.z);
	glTranslatef(-transform.position.x, -transform.position.y, -transform.position.z);
	glColor3f(1.0f, 1.0f, 1.0f);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	//_Element_Array_buffer

	glDisableClientState(GL_VERTEX_ARRAY);
}

PrimitiveTypes Primitive::GetType() const 
{
	return type;
}

void Primitive::SetPosition(math::float3 position)
{
	transform.position = transform.startPosition + position;
}

void Primitive::SetRotation(float angle, math::float3 u)
{
	transform.angle = angle;
	transform.u = u;
}

void Primitive::SetColor(Color color)
{
	this->color = color;
}

void Primitive::ShowAxis(bool showAxis) 
{
	this->showAxis = showAxis;
}

// Ray --------------------------------------------------
PrimitiveRay::PrimitiveRay(math::float3 direction, float length, math::float3 position) : Primitive(PrimitiveTypes::PrimitiveTypeRay), direction(direction), length(length)
{
	transform.startPosition = position;

	direction.Normalize();
	math::float3 endPosition = position + (direction * length);
	
	// Vertices
	uint verticesSize = 3 * 2;
	vertices = new float[verticesSize]{

		position.x, position.y, position.z,
		endPosition.x, endPosition.y, endPosition.z
	};

	indicesSize = 2; // used as verticesSize

	glGenBuffers(1, (GLuint*)&verticesID);
	glBindBuffer(GL_ARRAY_BUFFER, verticesID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * verticesSize, vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void PrimitiveRay::InnerRender() const
{
	// Draw (Vertex Array)
	glEnableClientState(GL_VERTEX_ARRAY);

	// Array Buffer
	glBindBuffer(GL_ARRAY_BUFFER, verticesID);

	glColor3f(color.r, color.g, color.b);
	glTranslatef(transform.position.x, transform.position.y, transform.position.z);
	glRotatef(transform.angle, transform.u.x, transform.u.y, transform.u.z);

	glVertexPointer(3, GL_FLOAT, 0, NULL);
	glDrawArrays(GL_LINES, 0, indicesSize);

	glRotatef(-transform.angle, transform.u.x, transform.u.y, transform.u.z);
	glTranslatef(-transform.position.x, -transform.position.y, -transform.position.z);
	glColor3f(1.0f, 1.0f, 1.0f);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//_Array_Buffer

	glDisableClientState(GL_VERTEX_ARRAY);
}

// Axis --------------------------------------------------
PrimitiveAxis::PrimitiveAxis() : Primitive(PrimitiveTypes::PrimitiveTypeAxis)
{
	x = new PrimitiveRay(math::float3(1.0f, 0.0f, 0.0f), 1.0f);
	x->SetColor(Red);
	y = new PrimitiveRay(math::float3(0.0f, 1.0f, 0.0f), 1.0f);
	y->SetColor(Green);
	z = new PrimitiveRay(math::float3(0.0f, 0.0f, 1.0f), 1.0f);
	z->SetColor(Blue);
}

PrimitiveAxis::~PrimitiveAxis() 
{
	RELEASE_ARRAY(x);
	RELEASE_ARRAY(y);
	RELEASE_ARRAY(z);
}

void PrimitiveAxis::InnerRender() const 
{
	App->renderer3D->SetCapabilityState(GL_DEPTH_TEST, false);
	if (x != nullptr)
		x->Render();
	if (y != nullptr)
		y->Render();
	if (z != nullptr)
		z->Render();
	App->renderer3D->SetCapabilityState(GL_DEPTH_TEST, true);
}

// Circle --------------------------------------------------
PrimitiveCircle::PrimitiveCircle(float radius, uint sides) : Primitive(PrimitiveTypes::PrimitiveTypeCircle), radius(radius), sides(sides)
{
	axis = new PrimitiveAxis();

	// Vertices
	uint verticesSize = 3 * (1 + sides);
	vertices = new float[verticesSize];

	/// Center (0)
	int i = 0;
	vertices[i] = 0.0f;
	vertices[++i] = 0.0f;
	vertices[++i] = 0.0f;

	float deltaAngle = 360.0f / (float)sides;

	for (float angle = 0.0f; angle < 360.0f; angle += deltaAngle)
	{
		/// B (1), C (2)... (sides)
		vertices[++i] = radius * cosf(DEGTORAD * angle);
		vertices[++i] = radius * sinf(DEGTORAD * angle);
		vertices[++i] = 0.0f;
	}

	glGenBuffers(1, (GLuint*)&verticesID);
	glBindBuffer(GL_ARRAY_BUFFER, verticesID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * verticesSize, vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Indices
	indicesSize = 3 * sides;
	indices = new uint[indicesSize];

	uint index = 1; /// B
	i = -1;
	while (i < (int)indicesSize)
	{
		indices[++i] = 0; /// A
		indices[++i] = index; /// B ... // C
		indices[++i] = ++index; /// C ... // D
	}

	indices[indicesSize - 1] = 1; /// C = B

	glGenBuffers(1, (GLuint*)&indicesID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * indicesSize, indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

// Plane --------------------------------------------------
PrimitivePlane::PrimitivePlane(math::float2 size) : Primitive(PrimitiveTypes::PrimitiveTypePlane), size(size)
{
	axis = new PrimitiveAxis();

	// Vertices
	uint verticesSize = 3 * 4;
	vertices = new float[verticesSize]{

		 size.x, 0.0f, -size.y, /// A (0)
		 size.x, 0.0f,  size.y, /// B (1)
		-size.x, 0.0f, -size.y, /// C (2)
		-size.x, 0.0f,  size.y  /// D (3)
	};

	glGenBuffers(1, (GLuint*)&verticesID);
	glBindBuffer(GL_ARRAY_BUFFER, verticesID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * verticesSize, vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Indices
	indicesSize = 2 * 3;
	indices = new uint[indicesSize]{

		2, 1, 0, /// CBA
		3, 1, 2  /// DBC
	};

	glGenBuffers(1, (GLuint*)&indicesID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * indicesSize, indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

// Grid --------------------------------------------------
PrimitiveGrid::PrimitiveGrid(uint quadSize, uint quadsX, uint quadsZ) : Primitive(PrimitiveTypes::PrimitiveTypeGrid), quadSize(quadSize), quadsX(quadsX), quadsZ(quadsZ)
{
	axis = new PrimitiveAxis();

	// Vertices
	uint verticesSize = 3 * (2 * ((quadsX + 1) + (quadsZ - 1)));
	vertices = new float[verticesSize];

	uint sizeX = quadsX * quadSize;
	uint sizeZ = quadsZ * quadSize;
	float halfSizeX = (float)sizeX / 2.0f;
	float halfSizeZ = (float)sizeZ / 2.0f;

	// x
	int i = -1;
	for (uint index = 0; index < quadsX + 1; ++index)
	{
		vertices[++i] = -halfSizeX + (index * quadSize);
		vertices[++i] = 0.0f;
		vertices[++i] = -halfSizeZ;
	}
	for (uint index = 0; index < quadsX + 1; ++index)
	{
		vertices[++i] = -halfSizeX + (index * quadSize);
		vertices[++i] = 0.0f;
		vertices[++i] = halfSizeZ;
	}

	// z
	for (uint index = 1; index < quadsZ; ++index)
	{
		vertices[++i] = -halfSizeX;
		vertices[++i] = 0.0f;
		vertices[++i] = -halfSizeZ + (index * quadSize);
	}
	for (uint index = 1; index < quadsZ; ++index)
	{
		vertices[++i] = halfSizeX;
		vertices[++i] = 0.0f;
		vertices[++i] = -halfSizeZ + (index * quadSize);
	}

	glGenBuffers(1, (GLuint*)&verticesID);
	glBindBuffer(GL_ARRAY_BUFFER, verticesID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * verticesSize, vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Indices
	indicesSize = 2 * ((quadsX + 1) + (quadsZ + 1));
	indices = new uint[indicesSize];

	// z (with x)
	i = -1;
	for (uint index = 0; index < quadsX + 1; ++index)
	{
		indices[++i] = index;
		indices[++i] = (quadsX + 1) + index;
	}

	// x (with z)
	indices[++i] = 0;
	indices[++i] = quadsX;

	for (uint index = 0; index < quadsZ - 1; ++index)
	{
		indices[++i] = 2 * (quadsX + 1) + index;
		indices[++i] = 2 * (quadsX + 1) + ((quadsZ - 1) + index);
	}

	indices[++i] = quadsX + 1;
	indices[++i] = (2 * quadsX) + 1;

	glGenBuffers(1, (GLuint*)&indicesID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * indicesSize, indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void PrimitiveGrid::InnerRender() const
{
	glEnableClientState(GL_VERTEX_ARRAY);

	// Array Buffer
	glBindBuffer(GL_ARRAY_BUFFER, verticesID);

	glColor3f(color.r, color.g, color.b);
	glTranslatef(transform.position.x, transform.position.y, transform.position.z);
	glRotatef(transform.angle, transform.u.x, transform.u.y, transform.u.z);

	glVertexPointer(3, GL_FLOAT, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//_Array_Buffer

	// Element Array Buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesID);
	glDrawElements(GL_LINES, indicesSize, GL_UNSIGNED_INT, NULL);

	glRotatef(-transform.angle, transform.u.x, transform.u.y, transform.u.z);
	glTranslatef(-transform.position.x, -transform.position.y, -transform.position.z);
	glColor3f(1.0f, 1.0f, 1.0f);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	//_Element_Array_buffer

	glDisableClientState(GL_VERTEX_ARRAY);
}

// Cube --------------------------------------------------
PrimitiveCube::PrimitiveCube(math::float3 size) : Primitive(PrimitiveTypes::PrimitiveTypeCube), size(size)
{
	axis = new PrimitiveAxis();

	math::float3 radius = size / 2.0f;

	// Vertices
	uint verticesSize = 3 * 8;
	vertices = new float[verticesSize]{

		-radius.x, -radius.y,  radius.z, /// A (0)
		 radius.x, -radius.y,  radius.z, /// B (1)
		-radius.x,  radius.y,  radius.z, /// C (2)
		 radius.x,  radius.y,  radius.z, /// D (3)
		-radius.x, -radius.y, -radius.z, /// E (4)
	   	 radius.x, -radius.y, -radius.z, /// F (5)
		-radius.x,  radius.y, -radius.z, /// G (6)
		 radius.x,  radius.y, -radius.z  /// H (7)
	};

	glGenBuffers(1, (GLuint*)&verticesID);
	glBindBuffer(GL_ARRAY_BUFFER, verticesID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * verticesSize, vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Indices
	indicesSize = 3 * (2 * 6);
	indices = new uint[indicesSize]{

		/// Front
		0, 1, 2, /// ABC
		1, 3, 2, /// BDC

		/// Right
		1, 5, 3, /// BFD
		5, 7, 3, /// FHD

		/// Back
		5, 4, 7, /// FEH
		4, 6, 7, /// EGH

		/// Left
		4, 0, 6, /// EAG
		0, 2, 6, /// ACG

		/// Top
		2, 3, 6, /// CDG
		3, 7, 6, /// DHG

		/// Bottom
		0, 4, 1, /// AEB
		1, 4, 5  /// BEF
	};

	glGenBuffers(1, (GLuint*)&indicesID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * indicesSize, indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

// Sphere --------------------------------------------------
PrimitiveSphere::PrimitiveSphere(float radius) : Primitive(PrimitiveTypes::PrimitiveTypeSphere), radius(radius)
{
	axis = new PrimitiveAxis();

	verticalCircle = new PrimitiveCircle(radius);
	horizontalCircle = new PrimitiveCircle(radius);
	horizontalCircle->SetRotation(-90.0f, math::float3(1.0f, 0.0f, 0.0f));
}

PrimitiveSphere::~PrimitiveSphere()
{
	RELEASE(verticalCircle);
	RELEASE(horizontalCircle);
}

void PrimitiveSphere::InnerRender() const 
{
	if (verticalCircle != nullptr)
		verticalCircle->Render();
	if (horizontalCircle != nullptr)
		horizontalCircle->Render();
}

// Cylinder --------------------------------------------------
PrimitiveCylinder::PrimitiveCylinder(float height, float radius, uint sides) : Primitive(PrimitiveTypes::PrimitiveTypeCylinder), height(height), radius(radius), sides(sides)
{
	axis = new PrimitiveAxis();

	// Vertices
	uint verticesSize = 3 * (2 * (sides + 1));
	vertices = new float[verticesSize];

	float halfHeight = height / 2.0f;
	float deltaAngle = 360.0f / (float)sides;

	// 1. Top circle
	// circleSize = sides + 1
	/// Center (0)
	int i = 0;
	vertices[i] = 0.0f;
	vertices[++i] = halfHeight;
	vertices[++i] = 0.0f;

	/// Circle (normals up)
	for (float angle = 0.0f; angle < 360.0f; angle += deltaAngle)
	{
		/// (1), (2)... (sides)
		vertices[++i] = radius * sinf(DEGTORAD * angle);
		vertices[++i] = halfHeight;
		vertices[++i] = radius * cosf(DEGTORAD * angle);
	}

	// 2. Bottom circle
	// circleSize = sides + 1
	/// Center (sides + 1)
	vertices[++i] = 0.0f;
	vertices[++i] = -halfHeight;
	vertices[++i] = 0.0f;

	/// Circle (normals up)
	for (float angle = 0.0f; angle < 360.0f; angle += deltaAngle)
	{
		/// (sides + 2), (sides + 3)... ((2 * sides) + 1)
		vertices[++i] = radius * sinf(DEGTORAD * angle);
		vertices[++i] = -halfHeight;
		vertices[++i] = radius * cosf(DEGTORAD * angle);
	}

	glGenBuffers(1, (GLuint*)&verticesID);
	glBindBuffer(GL_ARRAY_BUFFER, verticesID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * verticesSize, vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Indices
	indicesSize = 3 * ((2 * sides) + (2 * sides));
	indices = new uint[indicesSize];

	// 1. Top cap (circle)
	// circleSize = sides
	uint topIndex = 1; /// B
	i = -1;
	while (topIndex < sides + 1)
	{
		indices[++i] = 0; /// A (center)
		indices[++i] = topIndex; /// B ... // C
		indices[++i] = ++topIndex;/// C ... // D
	}

	indices[i] = 1; /// C = B

	// 2. Bottom cap (inverted circle)
	// circleSize = sides
	uint bottomIndex = sides + 2; /// B
	while (bottomIndex < 2 * (sides + 1))
	{
		indices[++i] = sides + 1; /// A (center)
		indices[++i] = ++bottomIndex; /// B ... // C
		indices[++i] = bottomIndex - 1; /// C ... // D
	}

	indices[i - 1] = sides + 2; /// C = B

	// 3. Body 
	// circleSize = 2 * sides
	topIndex = 1;
	bottomIndex = sides + 2;
	while (i < (int)indicesSize - (3 * 2))
	{
		/// Triangle
		indices[++i] = bottomIndex;
		indices[++i] = ++bottomIndex;
		indices[++i] = topIndex;

		/// Triangle
		indices[++i] = bottomIndex;
		indices[++i] = ++topIndex;
		indices[++i] = topIndex - 1;
	}

	/// Triangle
	indices[indicesSize - 5] = sides + 2;

	/// Triangle
	indices[indicesSize - 3] = sides + 2;
	indices[indicesSize - 2] = 1;

	glGenBuffers(1, (GLuint*)&indicesID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * indicesSize, indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

// Cone 
PrimitiveCone::PrimitiveCone(float height, float radius, uint sides, math::float3 position) : Primitive(PrimitiveTypes::PrimitiveTypeCone), height(height), radius(radius), sides(sides)
{
	transform.startPosition = position;

	axis = new PrimitiveAxis();

	// Vertices
	uint verticesSize = 3 * ((sides + 1) + 1);
	vertices = new float[verticesSize];

	float halfHeight = height / 2.0f;

	// 1. Circle
	// circleSize = sides + 1

	float deltaAngle = 360.0f / (float)sides;

	/// Center (0)
	int i = 0;
	vertices[i] = position.x;
	vertices[++i] = position.y - halfHeight;
	vertices[++i] = position.z;

	/// Circle (normals down)
	for (float angle = 0.0f; angle < 360.0f; angle += deltaAngle)
	{
		/// (1), (2)... (sides)
		vertices[++i] = position.x + radius * cosf(DEGTORAD * angle);
		vertices[++i] = position.y - halfHeight;
		vertices[++i] = position.z + radius * sinf(DEGTORAD * angle);
	}

	// 2. Top
	// size = 1
	/// Center (sides + 1)
	vertices[++i] = position.x;
	vertices[++i] = position.y + halfHeight;
	vertices[++i] = position.z;

	glGenBuffers(1, (GLuint*)&verticesID);
	glBindBuffer(GL_ARRAY_BUFFER, verticesID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * verticesSize, vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Indices
	indicesSize = 3 * (2 * sides);
	indices = new uint[indicesSize];

	// 1. Cap
	// circleSize = sides 
	uint index = 1; /// B
	i = -1;
	while (i < 3 * ((int)sides - 1))
	{
		indices[++i] = 0; /// A
		indices[++i] = index; /// B ... // C
		indices[++i] = ++index; /// C ... // D
	}

	indices[i] = 1; /// C = B

	// 2. Body (inverted circle)
	// circleSize = sides
	index = 1; /// B
	while (i < (int)indicesSize - 1)
	{
		indices[++i] = sides + 1; /// A
		indices[++i] = ++index; /// C ... // D
		indices[++i] = index - 1; /// B ... // C
	}

	indices[i - 1] = 1; /// C = B
	
	glGenBuffers(1, (GLuint*)&indicesID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * indicesSize, indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

// Arrow
PrimitiveArrow::PrimitiveArrow(float lineLength, float coneHeight, float coneRadius, uint coneSides) : Primitive(PrimitiveTypes::PrimitiveTypeArrow)
{
	// Cone + Line

	float arrowLength = lineLength + coneHeight;
	float arrowHalfLength = arrowLength / 2.0f;

	float coneHalfHeight = coneHeight / 2.0f;

	cone = new PrimitiveCone(coneHeight, coneRadius, coneSides, math::float3(0.0f, arrowHalfLength - coneHalfHeight, 0.0f));
	line = new PrimitiveRay(math::float3(0.0f, 1.0f, 0.0f), lineLength, math::float3(0.0f, -arrowHalfLength, 0.0f));
}

PrimitiveArrow::~PrimitiveArrow() 
{
	RELEASE(cone);
	RELEASE(line);
}

void PrimitiveArrow::InnerRender() const 
{
	if (cone != nullptr)
		cone->Render();
	if (line != nullptr)
		line->Render();
}

// Frustum --------------------------------------------------
PrimitiveFrustum::PrimitiveFrustum(math::float2 startSize, math::float3 endPosition, math::float2 endSize) : Primitive(PrimitiveTypes::PrimitiveTypeFrustum), startSize(startSize), endPosition(endPosition), endSize(endSize)
{
	axis = new PrimitiveAxis();

	math::float2 startRadius = startSize / 2.0f;
	math::float2 endRadius = endSize / 2.0f;

	// Vertices
	uint verticesSize = 3 * 8;
	vertices = new float[verticesSize] {

		-startRadius.x, -startRadius.y, 0.0f, /// A (0)
		 startRadius.x, -startRadius.y, 0.0f, /// B (1)
		-startRadius.x, startRadius.y, 0.0f, /// C (2)
		 startRadius.x, startRadius.y, 0.0f, /// D (3)

		 endPosition.x - endRadius.x, endPosition.y - endRadius.y, endPosition.z, /// E (4)
		 endPosition.x + endRadius.x, endPosition.y - endRadius.y, endPosition.z, /// F (5)
		 endPosition.x - endRadius.x, endPosition.y + endRadius.y, endPosition.z, /// G (6)
		 endPosition.x + endRadius.x, endPosition.y + endRadius.y, endPosition.z, /// H (7)
	};

	glGenBuffers(1, (GLuint*)&verticesID);
	glBindBuffer(GL_ARRAY_BUFFER, verticesID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * verticesSize, vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Indices
	indicesSize = 3 * (2 * 6);
	indices = new uint[indicesSize]{

		/// Front
		2, 1, 0, /// ABC
		2, 3, 1, /// BDC

		/// Right
		3, 5, 1, /// BFD
		3, 7, 5, /// FHD

		/// Back
		7, 4, 5, /// FEH
		7, 6, 4, /// EGH

		/// Left
		6, 0, 4, /// EAG
		6, 2, 0, /// ACG

		/// Top
		6, 3, 2, /// CDG
		6, 7, 3, /// DHG

		/// Bottom
		1, 4, 0, /// AEB
		5, 4, 1	 /// BEF
	};

	glGenBuffers(1, (GLuint*)&indicesID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * indicesSize, indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}