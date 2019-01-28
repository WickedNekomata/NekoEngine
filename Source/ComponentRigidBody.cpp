#include "ComponentRigidBody.h"

#include "Application.h"
#include "ModulePhysics.h"

#include "imgui\imgui.h"

ComponentRigidBody::ComponentRigidBody(GameObject* parent, ComponentTypes componentType) : Component(parent, componentType) 
{
	geometry = new Geometry();
	geometry->geometryType = GeometryTypes::GeometryTypeSphere;
	ResetCurrentGeometry();
}

ComponentRigidBody::~ComponentRigidBody() 
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
}

void ComponentRigidBody::Update() {}

void ComponentRigidBody::OnUniqueEditor()
{
#ifndef GAMEMODE
	// Geometry type
	const char* geometryType[] = { "Sphere", "Capsule", "Box" };
	int currentGeometryType = geometry->geometryType;
	ImGui::PushItemWidth(100.0f);
	if (ImGui::Combo("Geometry Type", &currentGeometryType, geometryType, IM_ARRAYSIZE(geometryType)))
	{
		geometry->geometryType = (GeometryTypes)currentGeometryType;
		ResetCurrentGeometry();
		UpdateCurrentShape();
		return;
	}

	const double f64_lo_a = -1000000000000000.0, f64_hi_a = +1000000000000000.0;
	bool updateCurrentShape = false;
	switch (geometry->geometryType)
	{
	case GeometryTypes::GeometryTypeSphere:

		if (ImGui::InputFloat("Radius", &geometry->geometrySphere.radius))
			updateCurrentShape = true;

		break;

	case GeometryTypes::GeometryTypeCapsule:

		if (ImGui::InputFloat("Radius", &geometry->geometryCapsule.radius))
			updateCurrentShape = true;
		if (ImGui::InputFloat("Half height", &geometry->geometryCapsule.halfHeight))
			updateCurrentShape = true;

		break;

	case GeometryTypes::GeometryTypeBox:

		ImGui::Text("Half extents");
		ImGui::PushItemWidth(50.0f);
		if (ImGui::DragScalar("##CenterX", ImGuiDataType_Float, (void*)&geometry->geometryBox.halfExtents.x, 0.1f, &f64_lo_a, &f64_hi_a, "%f", 1.0f))
			updateCurrentShape = true;
		ImGui::SameLine();
		ImGui::PushItemWidth(50.0f);
		if (ImGui::DragScalar("##CenterY", ImGuiDataType_Float, (void*)&geometry->geometryBox.halfExtents.y, 0.1f, &f64_lo_a, &f64_hi_a, "%f", 1.0f))
			updateCurrentShape = true; 
		ImGui::SameLine();
		ImGui::PushItemWidth(50.0f);
		if (ImGui::DragScalar("##CenterZ", ImGuiDataType_Float, (void*)&geometry->geometryBox.halfExtents.z, 0.1f, &f64_lo_a, &f64_hi_a, "%f", 1.0f))
			updateCurrentShape = true;

		break;
	}
	if (updateCurrentShape)
		UpdateCurrentShape();
#endif
}

void ComponentRigidBody::ResetCurrentGeometry() const
{
	switch (geometry->geometryType)
	{
	case GeometryTypes::GeometryTypeSphere:
		geometry->geometrySphere.radius = 1.0f;
		break;

	case GeometryTypes::GeometryTypeCapsule:
		geometry->geometryCapsule.radius = 1.0f;
		geometry->geometryCapsule.halfHeight = 1.0f;
		break;

	case GeometryTypes::GeometryTypeBox:
		geometry->geometryBox.halfExtents = math::float3::one;
		break;
	}
}

void ComponentRigidBody::UpdateCurrentShape()
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
		gShape = App->physics->CreateShape(PxCapsuleGeometry(geometry->geometryCapsule.radius, geometry->geometryCapsule.halfHeight), *App->physics->GetDefaultMaterial());	
		break;

	case GeometryTypes::GeometryTypeBox:
		gShape = App->physics->CreateShape(PxBoxGeometry(geometry->geometryBox.halfExtents.x, geometry->geometryBox.halfExtents.y, geometry->geometryBox.halfExtents.z), *App->physics->GetDefaultMaterial());		
		break;
	}

	// Attach current shape
	if (gShape != nullptr)
		gActor->attachShape(*gShape);
}

void ComponentRigidBody::SetTransform(float* ptr) const
{
	gActor->setGlobalPose(PxTransform(PxMat44(ptr)));
}