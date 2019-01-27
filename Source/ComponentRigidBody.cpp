#include "ComponentRigidBody.h"

#include "imgui\imgui.h"

ComponentRigidBody::ComponentRigidBody(GameObject* parent, GeometryTypes geometryType) : Component(parent, ComponentType::RigidBodyComponent) {}

ComponentRigidBody::~ComponentRigidBody()
{
}

void ComponentRigidBody::Update() {}

void ComponentRigidBody::OnUniqueEditor()
{
#ifndef GAMEMODE
	ImGui::Text("RigidBody");
	ImGui::Spacing();
#endif
}

void ComponentRigidBody::CreateGeometry()
{
	switch (geometryType)
	{
	case GeometryTypes::Sphere:
		// Center, radius
		//PxRigidDynamic* aCapsuleActor = thePhysics->createRigidDynamic(PxTransform(position));
		//PxTransform relativePose(PxQuat(PxHalfPi, PxVec(0, 0, 1)));
		//PxShape* aCapsuleShape = PxRigidActorExt::createExclusiveShape(*aCapsuleActor,
		//PxCapsuleGeometry(radius, halfHeight), aMaterial);
		//aCapsuleShape->setLocalPose(relativePose);
		//PxRigidBodyExt::updateMassAndInertia(*aCapsuleActor, capsuleDensity);
		//aScene->addActor(aCapsuleActor);
		break;
	case GeometryTypes::Capsule:
		// Center, half height, radius

		break;
	case GeometryTypes::Box:

		break;
	}
}