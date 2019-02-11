#include "ComponentSphereCollider.h"

#include "Application.h"
#include "ModulePhysics.h"
#include "GameObject.h"
#include "ComponentTransform.h"
#include "Layers.h"

#include "ComponentRigidActor.h"

#include "imgui\imgui.h"

#include "MathGeoLib\include\Math\float4x4.h"

ComponentSphereCollider::ComponentSphereCollider(GameObject* parent) : ComponentCollider(parent, ComponentTypes::SphereColliderComponent)
{
	if (parent->boundingBox.IsFinite()) // TODO: finish this together with the rest of the shapes
	{
		math::float4x4 globalMatrix = parent->transform->GetGlobalMatrix();
		math::float3 position = math::float3::zero;
		math::Quat rotation = math::Quat::identity;
		math::float3 scale = math::float3::one;
		globalMatrix.Decompose(position, rotation, scale);

		center = parent->boundingBox.CenterPoint() - position;
		radius = parent->boundingBox.HalfDiagonal().Length();
	}

	RecalculateShape();

	physx::PxShapeFlags shapeFlags = gShape->getFlags();
	isTrigger = shapeFlags & physx::PxShapeFlag::Enum::eTRIGGER_SHAPE && !(shapeFlags & physx::PxShapeFlag::eSIMULATION_SHAPE);
	participateInContactTests = shapeFlags & physx::PxShapeFlag::Enum::eSIMULATION_SHAPE;
	participateInSceneQueries = shapeFlags & physx::PxShapeFlag::Enum::eSCENE_QUERY_SHAPE;
}

ComponentSphereCollider::~ComponentSphereCollider() {}

// ----------------------------------------------------------------------------------------------------

void ComponentSphereCollider::OnUniqueEditor()
{
#ifndef GAMEMODE
	ImGui::Text("Sphere Collider");
	ImGui::Spacing();

	ComponentCollider::OnUniqueEditor();

	ImGui::AlignTextToFramePadding();
	ImGui::Text("Radius"); ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragFloat("##SphereRadius", &radius, 0.01f, 0.01f, FLT_MAX, "%.2f", 1.0f))
		SetRadius(radius);
	ImGui::PopItemWidth();
#endif
}

// ----------------------------------------------------------------------------------------------------

void ComponentSphereCollider::RecalculateShape()
{
	ClearShape();

	physx::PxSphereGeometry gSphereGeometry(radius);
	gShape = App->physics->CreateShape(gSphereGeometry, *gMaterial);
	assert(gShape != nullptr);

	physx::PxTransform relativePose(physx::PxVec3(center.x, center.y, center.z));
	gShape->setLocalPose(relativePose);

	Layer* layer = App->layers->GetLayer(parent->GetLayer());
	SetFiltering(layer->GetFilterGroup(), layer->GetFilterMask());

	// -----

	if (parent->rigidActor != nullptr)
		parent->rigidActor->UpdateShape(gShape);
}

// ----------------------------------------------------------------------------------------------------

void ComponentSphereCollider::SetRadius(float radius)
{
	this->radius = radius;
	gShape->setGeometry(physx::PxSphereGeometry(radius));
}

// ----------------------------------------------------------------------------------------------------

physx::PxSphereGeometry ComponentSphereCollider::GetSphereGeometry() const
{
	physx::PxSphereGeometry sphereGeometry;
	gShape->getSphereGeometry(sphereGeometry);
	return sphereGeometry;
}