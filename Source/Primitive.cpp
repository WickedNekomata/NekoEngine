#include "Globals.h"
#include "glew/include/GL/glew.h"
#include "Primitive.h"

#include "MathGeoLib/include/Math/TransformOps.h"
#include "MathGeoLib/include/Math/MathConstants.h"

// Primitive --------------------------------------------------
Primitive::Primitive() {}

Primitive::Primitive(math::float3 position, PrimitiveTypes type) : position(position), type(type) {}

Primitive::~Primitive()
{
	RELEASE_ARRAY(vertices);
}

void Primitive::Render() const
{
	InnerRender();

	if (axis)
		RenderAxis();
}

void Primitive::InnerRender() const {}

void Primitive::RenderAxis() const
{	
	math::float3 size = GetSize();

	glBegin(GL_LINES);

	glLineWidth(1.f);

	// Y axis
	glColor3f(0.f, 255.f, 0.f);
	glVertex3f(position.x, position.y, position.z);
	glVertex3f(position.x, position.y + size.y, position.z);

	// X axis
	glColor3f(255.f, 0.f, 0.f);
	glVertex3f(position.x, position.y, position.z);
	glVertex3f(position.x + size.x, position.y, position.z);
	
	// Z axis
	glColor3f(0.f, 0.f, 255.f);
	glVertex3f(position.x, position.y, position.z);
	glVertex3f(position.x, position.y, position.z + size.z);

	glColor3f(1.f, 1.f, 1.f);

	glEnd();
}

PrimitiveTypes Primitive::GetType() const
{
	return type;
}

// Primitive Index --------------------------------------------------
PrimitiveIndex::PrimitiveIndex() {}

PrimitiveIndex::PrimitiveIndex(math::float3 position, PrimitiveTypes type) : Primitive(position, type) {}

PrimitiveIndex::~PrimitiveIndex() 
{
	RELEASE_ARRAY(indices);
}

void PrimitiveIndex::InnerRender() const
{
	glEnableClientState(GL_VERTEX_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, verticesID);
	glVertexPointer(3, GL_FLOAT, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesID);
	glDrawElements(GL_TRIANGLES, indicesSize, GL_UNSIGNED_BYTE, NULL);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glDisableClientState(GL_VERTEX_ARRAY);
}

// Ray
PrimitiveRay::PrimitiveRay(math::float3 startPos, math::float3 endPos) : Primitive(startPos, PrimitiveTypes::PrimitiveTypeRay), endPos(endPos)
{
	// Vertices
	uint verticesSize = 6;
	vertices = new GLfloat[verticesSize]{

		startPos.x, startPos.y, startPos.z,
		endPos.x, endPos.y, endPos.z
	};

	glGenBuffers(1, &verticesID);
	glBindBuffer(GL_ARRAY_BUFFER, verticesID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLuint) * verticesSize, vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void PrimitiveRay::InnerRender() const
{
	glEnableClientState(GL_VERTEX_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, verticesID);
	glVertexPointer(3, GL_FLOAT, 0, NULL);
	glDrawArrays(GL_LINES, 0, 2);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glDisableClientState(GL_VERTEX_ARRAY);
}

// Circle
PrimitiveCircle::PrimitiveCircle(math::float3 position, float radius, uint sides) : PrimitiveIndex(position, PrimitiveTypes::PrimitiveTypeCircle), radius(radius), sides(sides)
{
	// Vertices
	uint verticesSize = 3 * (1 + sides);
	vertices = new GLfloat[verticesSize];

	uint i = 0;
	// A (0)
	vertices[i] = position.x;
	vertices[++i] = position.y;
	vertices[++i] = position.z;

	float deltaAngle = 360.0f / (float)sides;

	for (float angle = 0.0f; angle < 360.0f; angle += deltaAngle)
	{
		// B (1), C (2)...
		vertices[++i] = radius * cosf(DEGTORAD * angle);
		vertices[++i] = radius * sinf(DEGTORAD * angle);
		vertices[++i] = position.z;
	}

	glGenBuffers(1, &verticesID);
	glBindBuffer(GL_ARRAY_BUFFER, verticesID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * verticesSize, vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Indices
	indicesSize = 3 * sides;
	indices = new GLubyte[indicesSize];

	uint index = 1;
	for (uint j = 0; j < indicesSize - 2; ++j)
	{
		indices[j] = 0; // A
		indices[++j] = index; // B ... // C
		indices[++j] = ++index; // C ... // D
	}
	indices[indicesSize - 1] = 1; // B

	glGenBuffers(1, &indicesID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indicesSize, indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

// Plane
PrimitivePlane::PrimitivePlane(math::float3 position, float sizeX, float sizeZ) : PrimitiveIndex(position, PrimitiveTypes::PrimitiveTypePlane), sizeX(sizeX), sizeZ(sizeZ)
{
	// Vertices
	uint verticesSize = 12;
	vertices = new GLfloat[verticesSize]{

		 sizeX, position.y, -sizeZ, // A (0)
		 sizeX, position.y,  sizeZ, // B (1)
		-sizeX, position.y, -sizeZ, // C (2)
		-sizeX, position.y,  sizeZ  // D (3)
	};

	glGenBuffers(1, &verticesID);
	glBindBuffer(GL_ARRAY_BUFFER, verticesID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLuint) * verticesSize, vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Indices
	indicesSize = 6;
	indices = new GLubyte[indicesSize]{

		2, 1, 0, // CBA
		3, 1, 2  // DBC
	};

	glGenBuffers(1, &indicesID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indicesSize, indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

// Cube
PrimitiveCube::PrimitiveCube(math::float3 position, math::float3 size) : PrimitiveIndex(position, PrimitiveTypes::PrimitiveTypeCube), size(size)
{
	math::float3 radius = size / 2.0f;

	// Vertices
	uint verticesSize = 24;
	vertices = new GLfloat[verticesSize]{

		-radius.x, -radius.y,  radius.z, // A (0)
		 radius.x, -radius.y,  radius.z, // B (1)
		-radius.x,  radius.y,  radius.z, // C (2)
		 radius.x,  radius.y,  radius.z, // D (3)
		-radius.x, -radius.y, -radius.z, // E (4)
	   	 radius.x, -radius.y, -radius.z, // F (5)
		-radius.x,  radius.y, -radius.z, // G (6)
		 radius.x,  radius.y, -radius.z  // H (7)
	};

	glGenBuffers(1, &verticesID);
	glBindBuffer(GL_ARRAY_BUFFER, verticesID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLuint) * verticesSize, vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Indices
	indicesSize = 36;
	indices = new GLubyte[indicesSize]{

		// Front
		0, 1, 2, // ABC
		1, 3, 2, // BDC

		// Right
		1, 5, 3, // BFD
		5, 7, 3, // FHD

		// Back
		5, 4, 7, // FEH
		4, 6, 7, // EGH

		// Left
		4, 0, 6, // EAG
		0, 2, 6, // ACG

		// Top
		2, 3, 6, // CDG
		3, 7, 6, // DHG

		// Bottom
		0, 4, 1, // AEB
		1, 4, 5  // BEF
	};

	glGenBuffers(1, &indicesID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indicesSize, indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

// Sphere
PrimitiveSphere::PrimitiveSphere(math::float3 position, float radius) : PrimitiveIndex(position, PrimitiveTypes::PrimitiveTypeSphere), radius(radius)
{
	verticalCircle = new PrimitiveCircle(position, radius);
	horizontalCircle = new PrimitiveCircle(position, radius);
}

PrimitiveSphere::~PrimitiveSphere() 
{
	RELEASE(verticalCircle);
	RELEASE(horizontalCircle);
}

void PrimitiveSphere::InnerRender() const 
{
	verticalCircle->Render();
	glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	horizontalCircle->Render();
	glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
}

// Cylinder
PrimitiveCylinder::PrimitiveCylinder(math::float3 position, float height, float radius, uint sides)
	: PrimitiveIndex(position, PrimitiveTypes::PrimitiveTypeCylinder), height(height), radius(radius), sides(sides)
{
	topCap = new PrimitiveCircle(math::float3(position.x, position.y + height / 2.0f, position.z), radius);
	bottomCap = new PrimitiveCircle(math::float3(position.x, position.y - height / 2.0f, position.z), radius);

	// TODO: Draw sides
}

PrimitiveCylinder::~PrimitiveCylinder() 
{
	RELEASE(topCap);
	RELEASE(bottomCap);
}

void PrimitiveCylinder::InnerRender() const 
{
	glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	topCap->Render();
	glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);

	glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	bottomCap->Render();
	glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
}

// Frustum
PrimitiveFrustum::PrimitiveFrustum(math::float3 startPosition, float startSizeX, float startSizeY, math::float3 endPosition, float endSizeX, float endSizeY) 
	: PrimitiveIndex(startPosition, PrimitiveTypes::PrimitiveTypeFrustum), startSizeX(startSizeX), startSizeY(startSizeY), endPosition(endPosition), endSizeX(endSizeX), endSizeY(endSizeY)
{
	float startRadiusX = startSizeX / 2;
	float startRadiusY = startSizeY / 2;

	float endRadiusX = endSizeX / 2;
	float endRadiusY = endSizeY / 2;

	// Vertices
	uint verticesSize = 24;
	vertices = new GLfloat[verticesSize]{

		startPosition.x - startRadiusX, startPosition.y - startRadiusX, startPosition.z, // A (0)
		startPosition.x + startRadiusX, startPosition.y - startRadiusX, startPosition.z, // B (1)
		startPosition.x - startRadiusX, startPosition.y + startRadiusX, startPosition.z, // C (2)
		startPosition.x + startRadiusX, startPosition.y + startRadiusX, startPosition.z, // D (3)

		endPosition.x - endRadiusX, endPosition.y - endRadiusX, endPosition.z,			 // E (4)
		endPosition.x + endRadiusX, endPosition.y - endRadiusX, endPosition.z,			 // F (5)
		endPosition.x - endRadiusX, endPosition.y + endRadiusX, endPosition.z,			 // G (6)
		endPosition.x + endRadiusX, endPosition.y + endRadiusX, endPosition.z,			 // H (7)
	};

	glGenBuffers(1, &verticesID);
	glBindBuffer(GL_ARRAY_BUFFER, verticesID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLuint) * verticesSize, vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Indices
	indicesSize = 36;
	indices = new GLubyte[indicesSize]{

		// Front
		2, 1, 0, // ABC
		2, 3, 1, // BDC

		// Right
		3, 5, 1, // BFD
		3, 7, 5, // FHD

		// Back
		7, 4, 5, // FEH
		7, 6, 4, // EGH

		// Left
		6, 0, 4, // EAG
		6, 2, 0, // ACG

		// Top
		6, 3, 2, // CDG
		6, 7, 3, // DHG

		// Bottom
		1, 4, 0, // AEB
		5, 4, 1	 // BEF
	};

	glGenBuffers(1, &indicesID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indicesSize, indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}