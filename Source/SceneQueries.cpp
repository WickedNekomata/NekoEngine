#include "SceneQueries.h"

#include "physx\include\PxPhysicsAPI.h"

RaycastHit::RaycastHit() {}

RaycastHit::RaycastHit(GameObject* gameObject, ComponentCollider* collider, ComponentRigidActor* actor, math::float3 point, math::float3 normal, math::float2 texCoord, float distance, uint faceIndex) :
	gameObject(gameObject), collider(collider), actor(actor), point(point), normal(normal), texCoord(texCoord), distance(distance), faceIndex(faceIndex) {}

RaycastHit::~RaycastHit() {}

void RaycastHit::SetGameObject(GameObject* gameObject)
{
	this->gameObject = gameObject;
}

void RaycastHit::SetCollider(ComponentCollider* collider)
{
	this->collider = collider;
}

void RaycastHit::SetActor(ComponentRigidActor* actor)
{
	this->actor = actor;
}

void RaycastHit::SetPoint(math::float3& point)
{
	this->point = point;
}

void RaycastHit::SetNormal(math::float3& normal)
{
	this->normal = normal;
}

void RaycastHit::SetTexCoord(math::float2& texCoord)
{
	this->texCoord = texCoord;
}

void RaycastHit::SetDistance(float distance)
{
	this->distance = distance;
}

void RaycastHit::SetFaceIndex(uint faceIndex)
{
	this->faceIndex = faceIndex;
}

GameObject* RaycastHit::GetGameObject() const
{
	return gameObject;
}

ComponentCollider* RaycastHit::GetCollider() const
{
	return collider;
}

ComponentRigidActor* RaycastHit::GetActor() const
{
	return actor;
}

math::float3 RaycastHit::GetPoint() const
{
	return point;
}

math::float3 RaycastHit::GetNormal() const
{
	return normal;
}

math::float2 RaycastHit::GetTexCoord() const
{
	return texCoord;
}

float RaycastHit::GetDistance() const
{
	return distance;
}

uint RaycastHit::GetFaceIndex() const
{
	return faceIndex;
}

// ----------------------------------------------------------------------------------------------------

SweepHit::SweepHit() {}

SweepHit::SweepHit(GameObject* gameObject, ComponentCollider* collider, ComponentRigidActor* actor, math::float3 point, math::float3 normal, float distance, uint faceIndex) :
	gameObject(gameObject), collider(collider), actor(actor), point(point), normal(normal), distance(distance), faceIndex(faceIndex) {}

SweepHit::~SweepHit() {}

void SweepHit::SetGameObject(GameObject* gameObject)
{
	this->gameObject = gameObject;
}

void SweepHit::SetCollider(ComponentCollider* collider)
{
	this->collider = collider;
}

void SweepHit::SetActor(ComponentRigidActor* actor)
{
	this->actor = actor;
}

void SweepHit::SetPoint(math::float3& point)
{
	this->point = point;
}

void SweepHit::SetNormal(math::float3& normal)
{
	this->normal = normal;
}

void SweepHit::SetDistance(float distance)
{
	this->distance = distance;
}

void SweepHit::SetFaceIndex(uint faceIndex)
{
	this->faceIndex = faceIndex;
}

GameObject* SweepHit::GetGameObject() const
{
	return gameObject;
}

ComponentCollider* SweepHit::GetCollider() const
{
	return collider;
}

ComponentRigidActor* SweepHit::GetActor() const
{
	return actor;
}

math::float3 SweepHit::GetPoint() const
{
	return point;
}

math::float3 SweepHit::GetNormal() const
{
	return normal;
}

float SweepHit::GetDistance() const
{
	return distance;
}

uint SweepHit::GetFaceIndex() const
{
	return faceIndex;
}

// ----------------------------------------------------------------------------------------------------

OverlapHit::OverlapHit() {}

OverlapHit::OverlapHit(GameObject* gameObject, ComponentCollider* collider, ComponentRigidActor* actor, uint faceIndex) :
	gameObject(gameObject), collider(collider), actor(actor), faceIndex(faceIndex) {}

OverlapHit::~OverlapHit() {}

void OverlapHit::SetGameObject(GameObject* gameObject)
{
	this->gameObject = gameObject;
}

void OverlapHit::SetCollider(ComponentCollider* collider)
{
	this->collider = collider;
}

void OverlapHit::SetActor(ComponentRigidActor* actor)
{
	this->actor = actor;
}

void OverlapHit::SetFaceIndex(uint faceIndex)
{
	this->faceIndex = faceIndex;
}

GameObject* OverlapHit::GetGameObject() const
{
	return gameObject;
}

ComponentCollider* OverlapHit::GetCollider() const
{
	return collider;
}

ComponentRigidActor* OverlapHit::GetActor() const
{
	return actor;
}

uint OverlapHit::GetFaceIndex() const
{
	return faceIndex;
}
