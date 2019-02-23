#include "DebugDrawer.h"

#include "Application.h"
#include "ModuleRenderer3D.h"

#include "glew\include\GL\glew.h"

#include <assert.h>

DebugDrawer::DebugDrawer() {}

DebugDrawer::~DebugDrawer() {}

void DebugDrawer::StartDebugDraw()
{
	cullFace = App->renderer3D->GetCapabilityState(GL_CULL_FACE);
	lighting = App->renderer3D->GetCapabilityState(GL_LIGHTING);
	texture2D = App->renderer3D->GetCapabilityState(GL_TEXTURE_2D);
	wireframeMode = App->renderer3D->IsWireframeMode();

	App->renderer3D->SetCapabilityState(GL_CULL_FACE, false);
	App->renderer3D->SetCapabilityState(GL_LIGHTING, false);
	App->renderer3D->SetCapabilityState(GL_TEXTURE_2D, false);
	App->renderer3D->SetWireframeMode(true);
}

void DebugDrawer::EndDebugDraw()
{
	App->renderer3D->SetCapabilityState(GL_CULL_FACE, cullFace);
	App->renderer3D->SetCapabilityState(GL_LIGHTING, lighting);
	App->renderer3D->SetCapabilityState(GL_TEXTURE_2D, texture2D);
	App->renderer3D->SetWireframeMode(wireframeMode);
}

void DebugDrawer::DebugDraw(const math::AABB& aabb, const Color& color, const math::float4x4& globalTransform) const
{
	assert(aabb.IsFinite() && globalTransform.IsFinite());

	math::float3 corners[8];
	aabb.GetCornerPoints(corners);

	DebugDrawBox(corners, color, globalTransform);
}

void DebugDrawer::DebugDraw(const math::Frustum& frustum, const Color& color, const math::float4x4& globalTransform) const
{
	assert(frustum.IsFinite() && globalTransform.IsFinite());

	math::float3 corners[8];
	frustum.GetCornerPoints(corners);

	DebugDrawBox(corners, color, globalTransform);
}

#define RAY_LENGTH 100.0f

void DebugDrawer::DebugDraw(const math::Ray& ray, const Color& color, const math::float4x4& globalTransform) const
{
	assert(ray.IsFinite() && globalTransform.IsFinite());

	math::float3 direction = ray.dir * RAY_LENGTH;
	DebugDrawLine(ray.pos, direction, color, globalTransform);
}

void DebugDrawer::DebugDrawBox(const math::float3* vertices, const Color& color, const math::float4x4& globalTransform) const
{
	assert(vertices->IsFinite() && globalTransform.IsFinite());

	glColor3f(color.r, color.g, color.b);
	glPushMatrix();
	glMultMatrixf(globalTransform.Transposed().ptr());

	glBegin(GL_QUADS);
	glVertex3fv((const GLfloat*)&vertices[1]);
	glVertex3fv((const GLfloat*)&vertices[5]);
	glVertex3fv((const GLfloat*)&vertices[7]);
	glVertex3fv((const GLfloat*)&vertices[3]);

	glVertex3fv((const GLfloat*)&vertices[4]);
	glVertex3fv((const GLfloat*)&vertices[0]);
	glVertex3fv((const GLfloat*)&vertices[2]);
	glVertex3fv((const GLfloat*)&vertices[6]);

	glVertex3fv((const GLfloat*)&vertices[5]);
	glVertex3fv((const GLfloat*)&vertices[4]);
	glVertex3fv((const GLfloat*)&vertices[6]);
	glVertex3fv((const GLfloat*)&vertices[7]);

	glVertex3fv((const GLfloat*)&vertices[0]);
	glVertex3fv((const GLfloat*)&vertices[1]);
	glVertex3fv((const GLfloat*)&vertices[3]);
	glVertex3fv((const GLfloat*)&vertices[2]);

	glVertex3fv((const GLfloat*)&vertices[3]);
	glVertex3fv((const GLfloat*)&vertices[7]);
	glVertex3fv((const GLfloat*)&vertices[6]);
	glVertex3fv((const GLfloat*)&vertices[2]);

	glVertex3fv((const GLfloat*)&vertices[0]);
	glVertex3fv((const GLfloat*)&vertices[4]);
	glVertex3fv((const GLfloat*)&vertices[5]);
	glVertex3fv((const GLfloat*)&vertices[1]);
	glEnd();

	glPopMatrix();
}

void DebugDrawer::DebugDrawBox(const math::float3& halfExtents, const Color& color, const math::float4x4& globalTransform) const
{
	assert(halfExtents.IsFinite() && globalTransform.IsFinite());

	glColor3f(color.r, color.g, color.b);
	glPushMatrix();
	glMultMatrixf(globalTransform.Transposed().ptr());

	glBegin(GL_QUADS);
	glVertex3f(-halfExtents.x, -halfExtents.y, halfExtents.z);
	glVertex3f(halfExtents.x, -halfExtents.y, halfExtents.z);
	glVertex3f(halfExtents.x, halfExtents.y, halfExtents.z);
	glVertex3f(-halfExtents.x, halfExtents.y, halfExtents.z);

	glVertex3f(halfExtents.x, -halfExtents.y, -halfExtents.z);
	glVertex3f(-halfExtents.x, -halfExtents.y, -halfExtents.z);
	glVertex3f(-halfExtents.x, halfExtents.y, -halfExtents.z);
	glVertex3f(halfExtents.x, halfExtents.y, -halfExtents.z);

	glVertex3f(halfExtents.x, -halfExtents.y, halfExtents.z);
	glVertex3f(halfExtents.x, -halfExtents.y, -halfExtents.z);
	glVertex3f(halfExtents.x, halfExtents.y, -halfExtents.z);
	glVertex3f(halfExtents.x, halfExtents.y, halfExtents.z);

	glVertex3f(-halfExtents.x, -halfExtents.y, -halfExtents.z);
	glVertex3f(-halfExtents.x, -halfExtents.y, halfExtents.z);
	glVertex3f(-halfExtents.x, halfExtents.y, halfExtents.z);
	glVertex3f(-halfExtents.x, halfExtents.y, -halfExtents.z);

	glVertex3f(-halfExtents.x, halfExtents.y, halfExtents.z);
	glVertex3f(halfExtents.x, halfExtents.y, halfExtents.z);
	glVertex3f(halfExtents.x, halfExtents.y, -halfExtents.z);
	glVertex3f(-halfExtents.x, halfExtents.y, -halfExtents.z);

	glVertex3f(-halfExtents.x, -halfExtents.y, -halfExtents.z);
	glVertex3f(halfExtents.x, -halfExtents.y, -halfExtents.z);
	glVertex3f(halfExtents.x, -halfExtents.y, halfExtents.z);
	glVertex3f(-halfExtents.x, -halfExtents.y, halfExtents.z);
	glEnd();

	glPopMatrix();
}

#define SPHERE_SIDES 30

void DebugDrawer::DebugDrawSphere(float radius, const Color& color, const math::float4x4& globalTransform) const
{
	assert(globalTransform.IsFinite());

	glColor3f(color.r, color.g, color.b);
	glPushMatrix();
	glMultMatrixf(globalTransform.Transposed().ptr());

	float deltaAngle = 360.0f / (float)SPHERE_SIDES;

	glBegin(GL_LINE_LOOP);
	for (float angle = 0.0f; angle <= 360.0f; angle += deltaAngle)
		glVertex3f(radius * cosf(DEGTORAD * angle), 0.0f, radius * sinf(DEGTORAD * angle));
	glEnd();

	glBegin(GL_LINE_LOOP);
	for (float angle = 0.0f; angle <= 360.0f; angle += deltaAngle)
		glVertex3f(radius * cosf(DEGTORAD * angle), radius * sinf(DEGTORAD * angle), 0.0f);
	glEnd();

	glBegin(GL_LINE_LOOP);
	for (float angle = 0.0f; angle <= 360.0f; angle += deltaAngle)
		glVertex3f(0.0f, radius * sinf(DEGTORAD * angle), radius * cosf(DEGTORAD * angle));
	glEnd();

	glPopMatrix();
}

#define CAPSULE_SIDES 30

void DebugDrawer::DebugDrawCapsule(float radius, float halfHeight, const Color& color, const math::float4x4& globalTransform) const
{
	assert(globalTransform.IsFinite());

	glColor3f(color.r, color.g, color.b);
	glPushMatrix();
	glMultMatrixf(globalTransform.Transposed().ptr());

	float deltaAngle = 360.0f / (float)SPHERE_SIDES;

	glBegin(GL_LINE_LOOP);
	for (float angle = 0.0f; angle <= 360.0f; angle += deltaAngle)
		glVertex3f(halfHeight, radius * sinf(DEGTORAD * angle), radius * cosf(DEGTORAD * angle));
	glEnd();

	glBegin(GL_LINE_LOOP);
	for (float angle = 0.0f; angle <= 360.0f; angle += deltaAngle)
		glVertex3f(-halfHeight, radius * sinf(DEGTORAD * angle), radius * cosf(DEGTORAD * angle));
	glEnd();

	glBegin(GL_LINE_STRIP);
	for (float angle = 270.0f; angle <= 450.0f; angle += deltaAngle)
		glVertex3f(halfHeight + radius * cosf(DEGTORAD * angle), radius * sinf(DEGTORAD * angle), 0.0f);
	glEnd();

	glBegin(GL_LINE_STRIP);
	for (float angle = 270.0f; angle <= 450.0f; angle += deltaAngle)
		glVertex3f(halfHeight + radius * cosf(DEGTORAD * angle), 0.0f, radius * sinf(DEGTORAD * angle));
	glEnd();

	glBegin(GL_LINE_STRIP);
	for (float angle = 90.0f; angle <= 270.0f; angle += deltaAngle)
		glVertex3f(-halfHeight + radius * cosf(DEGTORAD * angle), radius * sinf(DEGTORAD * angle), 0.0f);
	glEnd();

	glBegin(GL_LINE_STRIP);
	for (float angle = 90.0f; angle <= 270.0f; angle += deltaAngle)
		glVertex3f(-halfHeight + radius * cosf(DEGTORAD * angle), 0.0f, radius * sinf(DEGTORAD * angle));
	glEnd();

	glBegin(GL_LINE_STRIP);
	glVertex3f(halfHeight, radius, 0.0f);
	glVertex3f(-halfHeight, radius, 0.0f);
	glEnd();

	glBegin(GL_LINE_STRIP);
	glVertex3f(halfHeight, -radius, 0.0f);
	glVertex3f(-halfHeight, -radius, 0.0f);
	glEnd();

	glBegin(GL_LINE_STRIP);
	glVertex3f(halfHeight, 0.0f, radius);
	glVertex3f(-halfHeight, 0.0f, radius);
	glEnd();

	glBegin(GL_LINE_STRIP);
	glVertex3f(halfHeight, 0.0f, -radius);
	glVertex3f(-halfHeight, 0.0f, -radius);
	glEnd();

	glPopMatrix();
}

void DebugDrawer::DebugDrawLine(const math::float3& origin, const math::float3& destination, const Color& color, const math::float4x4& globalTransform) const
{
	assert(origin.IsFinite() && destination.IsFinite() && globalTransform.IsFinite());

	glColor3f(color.r, color.g, color.b);
	glPushMatrix();
	glMultMatrixf(globalTransform.Transposed().ptr());

	glBegin(GL_LINES);
	glVertex3f(origin.x, origin.y, origin.z);
	glVertex3f(destination.x, destination.y, destination.z);
	glEnd();

	glPopMatrix();
}

void DebugDrawer::DebugDrawCone(float radius, float height, const Color & color, const math::float4x4 & globalTransform) const
{
	glColor3f(color.r, color.g, color.b);
	glPushMatrix();
	glMultMatrixf(globalTransform.Transposed().ptr());

	float deltaAngle = 360.0f / (float)SPHERE_SIDES;

	glBegin(GL_LINE_LOOP);
	for (float angle = 0.0f; angle <= 360.0f; angle += deltaAngle)
		glVertex3f(radius * cosf(DEGTORAD * angle), height, radius * sinf(DEGTORAD * angle));
	glEnd();

	glPopMatrix();
}