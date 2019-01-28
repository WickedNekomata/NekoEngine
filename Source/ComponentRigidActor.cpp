#include "ComponentRigidActor.h"

#include "Application.h"
#include "ModulePhysics.h"
#include "GameObject.h"
#include "ComponentTransform.h"

#include "imgui\imgui.h"

ComponentRigidActor::ComponentRigidActor(GameObject* parent, ComponentTypes componentType) : Component(parent, componentType)
{
	geometry = new Geometry();
	geometry->geometryType = GeometryTypes::GeometryTypeSphere;
	ResetGeometry();
}

ComponentRigidActor::~ComponentRigidActor()
{
	RELEASE(geometry);

	// Detach current shape
	/*
	uint nbShapes = gActor->getNbShapes();
	if (nbShapes > 0)
	{
		PxShape* gShape = nullptr;
		gActor->getShapes(&gShape, 1);

		if (gShape != nullptr)
		{
			gActor->detachShape(*gShape);
			gShape->release();

			gShape = nullptr;
		}
	}*/

	App->physics->RemoveActor(*gActor);
	gActor->release();
}

void ComponentRigidActor::Update() {}

void ComponentRigidActor::OnUniqueEditor()
{
#ifndef GAMEMODE
	// Geometry type
	const char* geometryType[] = { "Sphere", "Capsule", "Box" };
	int currentGeometryType = geometry->geometryType;
	ImGui::PushItemWidth(100.0f);
	if (ImGui::Combo("Geometry Type", &currentGeometryType, geometryType, IM_ARRAYSIZE(geometryType)))
	{
		geometry->geometryType = (GeometryTypes)currentGeometryType;
		ResetGeometry();
		UpdateShape();
		return;
	}

	const double f64_lo_a = -1000000000000000.0, f64_hi_a = +1000000000000000.0;
	bool updateShape = false;
	switch (geometry->geometryType)
	{
	case GeometryTypes::GeometryTypeSphere:

		ImGui::Text("Radius"); ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
		if (ImGui::DragScalar("##SphereRadius", ImGuiDataType_Float, (void*)&geometry->geometrySphere.radius, 0.01f, &f64_lo_a, &f64_hi_a, "%.2f", 1.0f))
		{
			if (geometry->geometrySphere.radius <= 0.0f)
				geometry->geometrySphere.radius = 0.5f;
			updateShape = true;
		}

		break;

	case GeometryTypes::GeometryTypeCapsule:
	{
		ImGui::Text("Radius"); ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
		if (ImGui::DragScalar("##CapsuleRadius", ImGuiDataType_Float, (void*)&geometry->geometryCapsule.radius, 0.01f, &f64_lo_a, &f64_hi_a, "%.2f", 1.0f))
		{
			if (geometry->geometryCapsule.radius <= 0.0f)
				geometry->geometryCapsule.radius = 0.5f;
			updateShape = true;
		}

		ImGui::Text("Half height"); ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
		if (ImGui::DragScalar("##CapsuleHalfHeight", ImGuiDataType_Float, (void*)&geometry->geometryCapsule.halfHeight, 0.01f, &f64_lo_a, &f64_hi_a, "%.2f", 1.0f))
		{
			if (geometry->geometryCapsule.halfHeight <= 0.0f)
				geometry->geometryCapsule.halfHeight = 0.5f;
			updateShape = true;
		}

		const char* capsuleDirection[] = { "X-Axis", "Y-Axis", "Z-Axis" };
		int currentCapsuleDirection = geometry->geometryCapsule.direction;
		ImGui::PushItemWidth(100.0f);
		if (ImGui::Combo("Direction", &currentCapsuleDirection, capsuleDirection, IM_ARRAYSIZE(capsuleDirection)))
		{
			geometry->geometryCapsule.direction = (CapsuleDirection)currentCapsuleDirection;
			updateShape = true;
		}
	}
		break;

	case GeometryTypes::GeometryTypeBox:

		ImGui::Text("Half extents"); ImGui::PushItemWidth(50.0f);
		if (ImGui::DragScalar("##BoxHalfExtentsX", ImGuiDataType_Float, (void*)&geometry->geometryBox.halfExtents.x, 0.01f, &f64_lo_a, &f64_hi_a, "%.2f", 1.0f))
		{
			if (geometry->geometryBox.halfExtents.x <= 0.0f)
				geometry->geometryBox.halfExtents.x = 0.5f;
			updateShape = true;
		}
		ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
		if (ImGui::DragScalar("##BoxHalfExtentsY", ImGuiDataType_Float, (void*)&geometry->geometryBox.halfExtents.y, 0.01f, &f64_lo_a, &f64_hi_a, "%.2f", 1.0f))
		{
			if (geometry->geometryBox.halfExtents.y <= 0.0f)
				geometry->geometryBox.halfExtents.y = 0.5f;
			updateShape = true;
		}
		ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
		if (ImGui::DragScalar("##BoxHalfExtentsZ", ImGuiDataType_Float, (void*)&geometry->geometryBox.halfExtents.z, 0.01f, &f64_lo_a, &f64_hi_a, "%.2f", 1.0f))
		{
			if (geometry->geometryBox.halfExtents.z <= 0.0f)
				geometry->geometryBox.halfExtents.z = 0.5f;
			updateShape = true;
		}

		break;
	}
	if (updateShape)
		UpdateShape();
#endif
}

void ComponentRigidActor::ResetGeometry() const
{
	switch (geometry->geometryType)
	{
	case GeometryTypes::GeometryTypeSphere:
		geometry->geometrySphere.radius = 0.5f;
		break;

	case GeometryTypes::GeometryTypeCapsule:
		geometry->geometryCapsule.radius = 0.5f;
		geometry->geometryCapsule.halfHeight = 0.5f;
		geometry->geometryCapsule.direction = CapsuleDirection::CapsuleDirectionYAxis;
		break;

	case GeometryTypes::GeometryTypeBox:
		geometry->geometryBox.halfExtents = math::float3(0.5f, 0.5f, 0.5f);
		break;
	}
}

void ComponentRigidActor::UpdateShape()
{
	PxShape* gShape = nullptr;

	// Detach current shape
	uint nbShapes = gActor->getNbShapes();
	if (nbShapes > 0)
	{
		gActor->getShapes(&gShape, 1);

		if (gShape != nullptr)
		{
			gActor->detachShape(*gShape);
			gShape->release();

			gShape = nullptr;
		}
	}

	// Update current shape
	switch (geometry->geometryType)
	{
	case GeometryTypes::GeometryTypeSphere:
		gShape = App->physics->CreateShape(PxSphereGeometry(geometry->geometrySphere.radius), *App->physics->GetDefaultMaterial());		
		break;

	case GeometryTypes::GeometryTypeCapsule:
	{
		gShape = App->physics->CreateShape(PxCapsuleGeometry(geometry->geometryCapsule.radius, geometry->geometryCapsule.halfHeight), *App->physics->GetDefaultMaterial());
		
		switch (geometry->geometryCapsule.direction)
		{
		case CapsuleDirection::CapsuleDirectionYAxis:
		{
			math::float3 direction = math::float3(0.0f, 0.0f, 1.0f);
			PxTransform relativePose(PxQuat(PxHalfPi, PxVec3(direction.x, direction.y, direction.z)));
			gShape->setLocalPose(relativePose);
		}
			break;
		case CapsuleDirection::CapsuleDirectionZAxis:
		{
			math::float3 direction = math::float3(0.0f, 1.0f, 0.0f);
			PxTransform relativePose(PxQuat(PxHalfPi, PxVec3(direction.x, direction.y, direction.z)));
			gShape->setLocalPose(relativePose);
		}
			break;
		}
	}
		break;

	case GeometryTypes::GeometryTypeBox:
		gShape = App->physics->CreateShape(PxBoxGeometry(geometry->geometryBox.halfExtents.x, geometry->geometryBox.halfExtents.y, geometry->geometryBox.halfExtents.z), *App->physics->GetDefaultMaterial());		
		break;
	}

	// Attach current shape
	if (gShape != nullptr)
		gActor->attachShape(*gShape);
}

void ComponentRigidActor::UpdateTransform() const
{
	math::float4x4 globalMatrix = parent->transform->GetGlobalMatrix();
	gActor->setGlobalPose(PxTransform(PxMat44(globalMatrix.Transposed().ptr())));
}