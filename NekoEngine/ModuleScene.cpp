#include "Globals.h"
#include "Application.h"
#include "ModuleScene.h"
#include "ModuleWindow.h"
#include "ModuleRenderer3D.h"

#include "Primitive.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl2.h"
#include "imgui/imgui_impl_sdl.h"

#include "MathGeoLib/include/Math/float3.h"
#include "MathGeoLib/include/Geometry/Plane.h"
#include "MathGeoLib/include/Geometry/Sphere.h"
#include <gl/GL.h>
#include <stdio.h>
#include <time.h>

ModuleScene::ModuleScene(bool start_enabled) : Module(start_enabled)
{
	name = "Scene";
}

ModuleScene::~ModuleScene()
{}

bool ModuleScene::Init(JSON_Object * jObject)
{
	return true;
}

bool ModuleScene::Start()
{
	bool ret = true;

	float f;
	char* buf;

	App->camera->Move(math::float3(1.0f, 1.0f, 0.0f));
	App->camera->LookAt(math::float3(0, 0, 0));

	return ret;
}

update_status ModuleScene::Update(float dt)
{
	if (showIntersectionWin)
		ShowIntersectionWindow();
	else {
		RELEASE(currGeometryA);
		RELEASE(currGeometryB);
	}

	return UPDATE_CONTINUE;
}

bool ModuleScene::CleanUp()
{
	bool ret = true;

	return ret;
}

void ModuleScene::ShowIntersectionWindow() 
{
	ImGui::SetNextWindowSize({ 400,300 });

	ImGuiWindowFlags windowFlags = 0;
	windowFlags |= ImGuiWindowFlags_NoResize;
	windowFlags |= ImGuiWindowFlags_NoFocusOnAppearing;
	windowFlags |= ImGuiWindowFlags_NoScrollbar;
	windowFlags |= ImGuiWindowFlags_NoCollapse;
	
	ImGui::Begin("Test Intersections", &showIntersectionWin, windowFlags);

	const char* geometries[] = { "Sphere", "Capsule", "AABB", "OBB", "Plane", "Ray", "Triangle" };

	// Geometry A properties
	static math::float3 positionA(0.0f, 0.0f, 0.0f); // Sphere, AABB, Ray
	static float radiusA(1.0f); // Sphere, Capsule, AABB
	static math::LineSegment lineSegmentA(math::float3(0.0f, 0.0f, 0.0f), math::float3(0.0f, 0.0f, 0.0f)); // Capsule
	static math::float3 normalA(0.0f, 0.0f, 0.0f); // Plane
	static math::float3 directionA(0.0f, 0.0f, 0.0f); // Ray
	static float dA(0.0f); // Plane
	static math::float3 endpoint1A(0.0f, 0.0f, 0.0f); // Capsule, Triangle
	static math::float3 endpoint2A(0.0f, 0.0f, 0.0f); // Capsule, Triangle
	static math::float3 endpoint3A(0.0f, 0.0f, 0.0f); // Triangle

	ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_None;
	treeNodeFlags |= ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen;

	if (ImGui::TreeNodeEx("Geometry A", treeNodeFlags))
	{
		int type = currGeometryTypeA;
		ImGui::Combo("Type", &type, geometries, IM_ARRAYSIZE(geometries));
		currGeometryTypeA = (GeometryType)type;

		switch (currGeometryTypeA)
		{
		case GeometryType::Sphere:
		{
			// Position
			float pos[3] = { positionA.x, positionA.y, positionA.z };
			ImGui::InputFloat3("Position", pos);
			positionA = { pos[0], pos[1], pos[2] };

			// Radius
			ImGui::InputFloat("Radius", &radiusA);
		}
		break;

		case GeometryType::Capsule:
		{
			// Endpoints
			float ep1[3] = { endpoint1A.x, endpoint1A.y, endpoint1A.z };
			ImGui::InputFloat3("Endpoint A", ep1);
			endpoint1A = { ep1[0], ep1[1], ep1[2] };

			float ep2[3] = { endpoint2A.x, endpoint2A.y, endpoint2A.z };
			ImGui::InputFloat3("Endpoint B", ep2);
			endpoint2A = { ep2[0], ep2[1], ep2[2] };

			// Radius
			ImGui::InputFloat("Radius", &radiusA);

			lineSegmentA = { endpoint1A, endpoint2A };
		}
		break;

		case GeometryType::AABB:
		{
			// Position
			float pos[3] = { positionA.x, positionA.y, positionA.z };
			ImGui::InputFloat3("Position", pos);
			positionA = { pos[0], pos[1], pos[2] };

			// Radius
			ImGui::InputFloat("Radius", &radiusA);
		}
		break;

		case GeometryType::OBB:
		{
			// Position
			float pos[3] = { positionA.x, positionA.y, positionA.z };
			ImGui::InputFloat3("Position", pos);
			positionA = { pos[0], pos[1], pos[2] };

			// Radius
			ImGui::InputFloat("Radius", &radiusA);
		}
			break;

		case GeometryType::Plane:
		{
			// Normal
			float n[3] = { normalA.x, normalA.y, normalA.z };
			ImGui::InputFloat3("Normal", n);
			normalA = { n[0], n[1], n[2] };

			// d
			ImGui::InputFloat("d", &dA);
		}
		break;

		case GeometryType::Ray:
		{
			// Position
			float pos[3] = { positionA.x, positionA.y, positionA.z };
			ImGui::InputFloat3("Position", pos);
			positionA = { pos[0], pos[1], pos[2] };

			// Direction
			float dir[3] = { directionA.x, directionA.y, directionA.z };
			ImGui::InputFloat3("Direction", dir);
			directionA = { dir[0], dir[1], dir[2] };
		}
		break;

		case GeometryType::Triangle:
		{
			// Endpoints
			float ep1[3] = { endpoint1A.x, endpoint1A.y, endpoint1A.z };
			ImGui::InputFloat3("Endpoint A", ep1);
			endpoint1A = { ep1[0], ep1[1], ep1[2] };

			float ep2[3] = { endpoint2A.x, endpoint2A.y, endpoint2A.z };
			ImGui::InputFloat3("Endpoint B", ep2);
			endpoint2A = { ep2[0], ep2[1], ep2[2] };

			float ep3[3] = { endpoint3A.x, endpoint3A.y, endpoint3A.z };
			ImGui::InputFloat3("Endpoint C", ep3);
			endpoint3A = { ep3[0], ep3[1], ep3[2] };
		}
		break;

		default:
			break;
		};

		ImGui::TreePop();
	}

	// Geometry B properties
	static math::float3 positionB(0.0f, 0.0f, 0.0f); // Sphere, AABB, Ray
	static float radiusB(1.0f); // Sphere, Capsule, AABB
	static math::LineSegment lineSegmentB(math::float3(0.0f, 0.0f, 0.0f), math::float3(0.0f, 0.0f, 0.0f)); // Capsule
	static math::float3 normalB(0.0f, 0.0f, 0.0f); // Plane
	static math::float3 directionB(0.0f, 0.0f, 0.0f); // Ray
	static float dB(0.0f); // Plane
	static math::float3 endpoint1B(0.0f, 0.0f, 0.0f); // Capsule, Triangle
	static math::float3 endpoint2B(0.0f, 0.0f, 0.0f); // Capsule, Triangle
	static math::float3 endpoint3B(0.0f, 0.0f, 0.0f); // Triangle

	if (ImGui::TreeNodeEx("Geometry B", treeNodeFlags))
	{
		int type = currGeometryTypeB;
		ImGui::Combo("Type", &type, geometries, IM_ARRAYSIZE(geometries));
		currGeometryTypeB = (GeometryType)type;

		switch (currGeometryTypeB)
		{
		case GeometryType::Sphere:
		{
			// Position
			float pos[3] = { positionB.x, positionB.y, positionB.z };
			ImGui::InputFloat3("Position", pos);
			positionB = { pos[0], pos[1], pos[2] };

			// Radius
			ImGui::InputFloat("Radius", &radiusB);
		}
		break;

		case GeometryType::Capsule:
		{
			// Endpoints
			float ep1[3] = { endpoint1B.x, endpoint1B.y, endpoint1B.z };
			ImGui::InputFloat3("Endpoint A", ep1);
			endpoint1B = { ep1[0], ep1[1], ep1[2] };

			float ep2[3] = { endpoint2B.x, endpoint2B.y, endpoint2B.z };
			ImGui::InputFloat3("Endpoint B", ep2);
			endpoint2B = { ep2[0], ep2[1], ep2[2] };

			// Radius
			ImGui::InputFloat("Radius", &radiusB);

			lineSegmentB = { endpoint1B, endpoint2B };
		}
		break;

		case GeometryType::AABB:
		{
			// Position
			float pos[3] = { positionB.x, positionB.y, positionB.z };
			ImGui::InputFloat3("Position", pos);
			positionB = { pos[0], pos[1], pos[2] };

			// Radius
			ImGui::InputFloat("Radius", &radiusB);
		}
		break;

		case GeometryType::OBB:
		{
			// Position
			float pos[3] = { positionB.x, positionB.y, positionB.z };
			ImGui::InputFloat3("Position", pos);
			positionB = { pos[0], pos[1], pos[2] };

			// Radius
			ImGui::InputFloat("Radius", &radiusB);
		}
			break;

		case GeometryType::Plane:
		{
			// Normal
			float n[3] = { normalB.x, normalB.y, normalB.z };
			ImGui::InputFloat3("Normal", n);
			normalB = { n[0], n[1], n[2] };

			// d
			ImGui::InputFloat("d", &dB);
		}
		break;

		case GeometryType::Ray:
		{
			// Position
			float pos[3] = { positionB.x, positionB.y, positionB.z };
			ImGui::InputFloat3("Position", pos);
			positionB = { pos[0], pos[1], pos[2] };

			// Direction
			float dir[3] = { directionB.x, directionB.y, directionB.z };
			ImGui::InputFloat3("Direction", dir);
			directionB = { dir[0], dir[1], dir[2] };
		}
		break;

		case GeometryType::Triangle:
		{
			// Endpoints
			float ep1[3] = { endpoint1B.x, endpoint1B.y, endpoint1B.z };
			ImGui::InputFloat3("Endpoint A", ep1);
			endpoint1B = { ep1[0], ep1[1], ep1[2] };

			float ep2[3] = { endpoint2B.x, endpoint2B.y, endpoint2B.z };
			ImGui::InputFloat3("Endpoint B", ep2);
			endpoint2B = { ep2[0], ep2[1], ep2[2] };

			float ep3[3] = { endpoint3B.x, endpoint3B.y, endpoint3B.z };
			ImGui::InputFloat3("Endpoint C", ep3);
			endpoint3B = { ep3[0], ep3[1], ep3[2] };
		}
		break;

		default:
			break;
		};

		ImGui::TreePop();
	}

	ImGui::Spacing();

	static bool intersect = false;

	if (ImGui::Button("GENERATE"))
	{
		// Create geometry A
		RELEASE(currGeometryA);
		
		currGeometryA = new GeometryObject();

		switch (currGeometryTypeA)
		{
		case GeometryType::Sphere:
		{
			math::Sphere* sphere = new math::Sphere(positionA, radiusA);
			currGeometryA->geometry = (Geometry*)sphere;
			currGeometryA->geometryType = GeometryType::Sphere;
		}
		break;

		case GeometryType::Capsule:
		{
			math::Capsule* capsule = new math::Capsule(lineSegmentA, radiusA);
			currGeometryA->geometry = (Geometry*)capsule;
			currGeometryA->geometryType = GeometryType::Capsule;
		}
		break;

		case GeometryType::AABB:
		{
			math::AABB* aabb = new math::AABB(math::Sphere(positionA, radiusA));
			currGeometryA->geometry = (Geometry*)aabb;
			currGeometryA->geometryType = GeometryType::AABB;
		}
		break;

		case GeometryType::OBB:
		{
			math::OBB* obb = new math::OBB(math::AABB(math::Sphere(positionA, radiusA)));
			currGeometryA->geometry = (Geometry*)obb;
			currGeometryA->geometryType = GeometryType::OBB;
		}
		break;

		case GeometryType::Plane:
		{
			math::Plane* plane = new math::Plane(normalA, dA);
			currGeometryA->geometry = (Geometry*)plane;
			currGeometryA->geometryType = GeometryType::Plane;
		}
		break;

		case GeometryType::Ray:
		{
			math::Ray* ray = new math::Ray(positionA, directionA);
			currGeometryA->geometry = (Geometry*)ray;
			currGeometryA->geometryType = GeometryType::Ray;
		}
		break;

		case GeometryType::Triangle:
		{
			math::Triangle* triangle = new math::Triangle(endpoint1A, endpoint2A, endpoint3A);
			currGeometryA->geometry = (Geometry*)triangle;
			currGeometryA->geometryType = GeometryType::Triangle;
		}
		break;

		default:
			break;
		}

		// Create geometry B
		RELEASE(currGeometryB);

		currGeometryB = new GeometryObject();

		switch (currGeometryTypeB)
		{
		case GeometryType::Sphere:
		{
			math::Sphere* sphere = new math::Sphere(positionB, radiusB);
			currGeometryB->geometry = (Geometry*)sphere;
			currGeometryB->geometryType = GeometryType::Sphere;
		}
		break;

		case GeometryType::Capsule:
		{
			math::Capsule* capsule = new math::Capsule(lineSegmentB, radiusB);
			currGeometryB->geometry = (Geometry*)capsule;
			currGeometryB->geometryType = GeometryType::Capsule;
		}
		break;

		case GeometryType::AABB:
		{
			math::AABB* aabb = new math::AABB(math::Sphere(positionB, radiusB));
			currGeometryB->geometry = (Geometry*)aabb;
			currGeometryB->geometryType = GeometryType::AABB;
		}
		break;

		case GeometryType::OBB:
		{
			math::OBB* obb = new math::OBB(math::AABB(math::Sphere(positionB, radiusB)));
			currGeometryB->geometry = (Geometry*)obb;
			currGeometryB->geometryType = GeometryType::OBB;
		}
		break;

		case GeometryType::Plane:
		{
			math::Plane* plane = new math::Plane(normalB, dB);
			currGeometryB->geometry = (Geometry*)plane;
			currGeometryB->geometryType = GeometryType::Plane;
		}
		break;

		case GeometryType::Ray:
		{
			math::Ray* ray = new math::Ray(positionB, directionB);
			currGeometryB->geometry = (Geometry*)ray;
			currGeometryB->geometryType = GeometryType::Ray;
		}
		break;

		case GeometryType::Triangle:
		{
			math::Triangle* triangle = new math::Triangle(endpoint1B, endpoint2B, endpoint3B);
			currGeometryB->geometry = (Geometry*)triangle;
			currGeometryB->geometryType = GeometryType::Triangle;
		}
		break;

		default:
			break;
		}

		// Calculate intersection
		intersect = Intersect(currGeometryA, currGeometryB);
	}

	ImGui::SameLine();

	if (intersect)
		ImGui::Text("Intersection");
	else
		ImGui::Text("No intersection");

	ImGui::End();
}

#pragma region INTERSECT_METHODS
bool ModuleScene::Intersect(GeometryObject* geometryA, GeometryObject* geometryB)
{
	if (!geometryA || !geometryB)
		return false;

	switch (geometryA->geometryType)
	{
	case GeometryType::Sphere:
	{
		math::Sphere* sphereA = (math::Sphere*)geometryA->geometry;
		return SphereIntersect(sphereA, geometryB);		
	}
	break;

	case GeometryType::Capsule:
	{
		math::Capsule* capsuleA = (math::Capsule*)geometryA->geometry;
		return CapsuleIntersect(capsuleA, geometryB);
	}
	break;

	case GeometryType::AABB:
	{
		math::AABB* aabbA = (math::AABB*)geometryA->geometry;
		return AABBIntersect(aabbA, geometryB);
	}
	break;

	case GeometryType::OBB:
	{
		math::OBB* obbA = (math::OBB*)geometryA->geometry;
		return OBBIntersect(obbA, geometryB);
	}
	break;

	case GeometryType::Plane:
	{
		math::Plane* planeA = (math::Plane*)geometryA->geometry;
		return PlaneIntersect(planeA, geometryB);
	}
	break;

	case GeometryType::Ray:
	{
		math::Ray* rayA = (math::Ray*)geometryA->geometry;
		return RayIntersect(rayA, geometryB);
	}
	break;

	case GeometryType::Triangle:
	{
		math::Triangle* triangleA = (math::Triangle*)geometryA->geometry;
		return TriangleIntersect(triangleA, geometryB);
	}
	break;

	default:
		break;
	}

	return false;
}

bool ModuleScene::SphereIntersect(math::Sphere* sphereA, GeometryObject* geometryB)
{
	if (!sphereA || !geometryB)
		return false;

	switch (geometryB->geometryType)
	{
	case GeometryType::Sphere:
	{
		math::Sphere* sphereB = (math::Sphere*)geometryB->geometry;
		return sphereA->Intersects(*sphereB);
	}
	break;

	case GeometryType::Capsule:
	{
		math::Capsule* capsuleB = (math::Capsule*)geometryB->geometry;
		return sphereA->Intersects(*capsuleB);
	}
	break;

	case GeometryType::AABB:
	{
		math::AABB* aabbB = (math::AABB*)geometryB->geometry;
		return sphereA->Intersects(*aabbB);
	}
	break;

	case GeometryType::OBB:
	{
		math::OBB* obbB = (math::OBB*)geometryB->geometry;
		return sphereA->Intersects(*obbB);
	}
	break;

	case GeometryType::Plane:
	{
		math::Plane* planeB = (math::Plane*)geometryB->geometry;
		return sphereA->Intersects(*planeB);
	}
	break;

	case GeometryType::Ray:
	{
		math::Ray* rayB = (math::Ray*)geometryB->geometry;
		return sphereA->Intersects(*rayB);
	}
	break;

	case GeometryType::Triangle:
	{
		math::Triangle* triangleB = (math::Triangle*)geometryB->geometry;
		return sphereA->Intersects(*triangleB);
	}
	break;

	default:
		break;
	}

	return false;
}

bool ModuleScene::CapsuleIntersect(math::Capsule* capsuleA, GeometryObject* geometryB)
{
	if (!capsuleA || !geometryB)
		return false;

	switch (geometryB->geometryType)
	{
	case GeometryType::Sphere:
	{
		math::Sphere* sphereB = (math::Sphere*)geometryB->geometry;
		return capsuleA->Intersects(*sphereB);
	}
	break;

	case GeometryType::Capsule:
	{
		math::Capsule* capsuleB = (math::Capsule*)geometryB->geometry;
		return capsuleA->Intersects(*capsuleB);
	}
	break;

	case GeometryType::AABB:
	{
		math::AABB* aabbB = (math::AABB*)geometryB->geometry;
		return capsuleA->Intersects(*aabbB);
	}
	break;

	case GeometryType::OBB:
	{
		math::OBB* obbB = (math::OBB*)geometryB->geometry;
		return capsuleA->Intersects(*obbB);
	}
	break;

	case GeometryType::Plane:
	{
		math::Plane* planeB = (math::Plane*)geometryB->geometry;
		return capsuleA->Intersects(*planeB);
	}
	break;

	case GeometryType::Ray:
	{
		math::Ray* rayB = (math::Ray*)geometryB->geometry;
		return capsuleA->Intersects(*rayB);
	}
	break;

	case GeometryType::Triangle:
	{
		math::Triangle* triangleB = (math::Triangle*)geometryB->geometry;
		return capsuleA->Intersects(*triangleB);
	}
	break;

	default:
		break;
	}

	return false;
}

bool ModuleScene::AABBIntersect(math::AABB* aabbA, GeometryObject* geometryB)
{
	if (!aabbA || !geometryB)
		return false;

	switch (geometryB->geometryType)
	{
	case GeometryType::Sphere:
	{
		math::Sphere* sphereB = (math::Sphere*)geometryB->geometry;
		return aabbA->Intersects(*sphereB);
	}
	break;

	case GeometryType::Capsule:
	{
		math::Capsule* capsuleB = (math::Capsule*)geometryB->geometry;
		return aabbA->Intersects(*capsuleB);
	}
	break;

	case GeometryType::AABB:
	{
		math::AABB* aabbB = (math::AABB*)geometryB->geometry;
		return aabbA->Intersects(*aabbB);
	}
	break;

	case GeometryType::OBB:
	{
		math::OBB* obbB = (math::OBB*)geometryB->geometry;
		return aabbA->Intersects(*obbB);
	}
	break;

	case GeometryType::Plane:
	{
		math::Plane* planeB = (math::Plane*)geometryB->geometry;
		return aabbA->Intersects(*planeB);
	}
	break;

	case GeometryType::Ray:
	{
		math::Ray* rayB = (math::Ray*)geometryB->geometry;
		return aabbA->Intersects(*rayB);
	}
	break;

	case GeometryType::Triangle:
	{
		math::Triangle* triangleB = (math::Triangle*)geometryB->geometry;
		return aabbA->Intersects(*triangleB);
	}
	break;

	default:
		break;
	}

	return false;
}

bool ModuleScene::OBBIntersect(math::OBB* obbA, GeometryObject* geometryB)
{
	if (!obbA || !geometryB)
		return false;

	switch (geometryB->geometryType)
	{
	case GeometryType::Sphere:
	{
		math::Sphere* sphereB = (math::Sphere*)geometryB->geometry;
		return obbA->Intersects(*sphereB);
	}
	break;

	case GeometryType::Capsule:
	{
		math::Capsule* capsuleB = (math::Capsule*)geometryB->geometry;
		return obbA->Intersects(*capsuleB);
	}
	break;

	case GeometryType::AABB:
	{
		math::AABB* aabbB = (math::AABB*)geometryB->geometry;
		return obbA->Intersects(*aabbB);
	}
	break;

	case GeometryType::OBB:
	{
		math::OBB* obbB = (math::OBB*)geometryB->geometry;
		return obbA->Intersects(*obbB);
	}
	break;

	case GeometryType::Plane:
	{
		math::Plane* planeB = (math::Plane*)geometryB->geometry;
		return obbA->Intersects(*planeB);
	}
	break;

	case GeometryType::Ray:
	{
		math::Ray* rayB = (math::Ray*)geometryB->geometry;
		return obbA->Intersects(*rayB);
	}
	break;

	case GeometryType::Triangle:
	{
		math::Triangle* triangleB = (math::Triangle*)geometryB->geometry;
		return obbA->Intersects(*triangleB);
	}
	break;

	default:
		break;
	}

	return false;
}

bool ModuleScene::PlaneIntersect(math::Plane* planeA, GeometryObject* geometryB)
{
	if (!planeA || !geometryB)
		return false;

	switch (geometryB->geometryType)
	{
	case GeometryType::Sphere:
	{
		math::Sphere* sphereB = (math::Sphere*)geometryB->geometry;
		return planeA->Intersects(*sphereB);
	}
	break;

	case GeometryType::Capsule:
	{
		math::Capsule* capsuleB = (math::Capsule*)geometryB->geometry;
		return planeA->Intersects(*capsuleB);
	}
	break;

	case GeometryType::AABB:
	{
		math::AABB* aabbB = (math::AABB*)geometryB->geometry;
		return planeA->Intersects(*aabbB);
	}
	break;

	case GeometryType::OBB:
	{
		math::OBB* obbB = (math::OBB*)geometryB->geometry;
		return planeA->Intersects(*obbB);
	}
	break;

	case GeometryType::Plane:
	{
		math::Plane* planeB = (math::Plane*)geometryB->geometry;
		return planeA->Intersects(*planeB);
	}
	break;

	case GeometryType::Ray:
	{
		math::Ray* rayB = (math::Ray*)geometryB->geometry;
		return planeA->Intersects(*rayB);
	}
	break;

	case GeometryType::Triangle:
	{
		math::Triangle* triangleB = (math::Triangle*)geometryB->geometry;
		return planeA->Intersects(*triangleB);
	}
	break;

	default:
		break;
	}

	return false;
}

bool ModuleScene::RayIntersect(math::Ray* rayA, GeometryObject* geometryB)
{
	if (!rayA || !geometryB)
		return false;

	switch (geometryB->geometryType)
	{
	case GeometryType::Sphere:
	{
		math::Sphere* sphereB = (math::Sphere*)geometryB->geometry;
		return rayA->Intersects(*sphereB);
	}
	break;

	case GeometryType::Capsule:
	{
		math::Capsule* capsuleB = (math::Capsule*)geometryB->geometry;
		return rayA->Intersects(*capsuleB);
	}
	break;

	case GeometryType::AABB:
	{
		math::AABB* aabbB = (math::AABB*)geometryB->geometry;
		return rayA->Intersects(*aabbB);
	}
	break;

	case GeometryType::OBB:
	{
		math::OBB* obbB = (math::OBB*)geometryB->geometry;
		return rayA->Intersects(*obbB);
	}
	break;

	case GeometryType::Plane:
	{
		math::Plane* planeB = (math::Plane*)geometryB->geometry;
		return rayA->Intersects(*planeB);
	}
	break;

	case GeometryType::Ray:
	{
		_LOG("2 rays can't intersect each other");
		return false;
	}
	break;

	case GeometryType::Triangle:
	{
		math::Triangle* triangleB = (math::Triangle*)geometryB->geometry;
		return rayA->Intersects(*triangleB);
	}
	break;

	default:
		break;
	}

	return false;
}

bool ModuleScene::TriangleIntersect(math::Triangle* triangleA, GeometryObject* geometryB)
{
	if (!triangleA || !geometryB)
		return false;

	switch (geometryB->geometryType)
	{
	case GeometryType::Sphere:
	{
		math::Sphere* sphereB = (math::Sphere*)geometryB->geometry;
		return triangleA->Intersects(*sphereB);
	}
	break;

	case GeometryType::Capsule:
	{
		math::Capsule* capsuleB = (math::Capsule*)geometryB->geometry;
		return triangleA->Intersects(*capsuleB);
	}
	break;

	case GeometryType::AABB:
	{
		math::AABB* aabbB = (math::AABB*)geometryB->geometry;
		return triangleA->Intersects(*aabbB);
	}
	break;

	case GeometryType::OBB:
	{
		math::OBB* obbB = (math::OBB*)geometryB->geometry;
		return triangleA->Intersects(*obbB);
	}
	break;

	case GeometryType::Plane:
	{
		math::Plane* planeB = (math::Plane*)geometryB->geometry;
		return triangleA->Intersects(*planeB);
	}
	break;

	case GeometryType::Ray:
	{
		math::Ray* rayB = (math::Ray*)geometryB->geometry;
		return triangleA->Intersects(*rayB);
	}
	break;

	case GeometryType::Triangle:
	{
		math::Triangle* triangleB = (math::Triangle*)geometryB->geometry;
		return triangleA->Intersects(*triangleB);
	}
	break;

	default:
		break;
	}

	return false;
}

#pragma endregion
