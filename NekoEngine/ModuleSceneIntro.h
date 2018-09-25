#ifndef __MODULE_SCENE_INTRO_H__
#define __MODULE_SCENE_INTRO_H__

#include "Module.h"

#include "MathGeoLib/include/Geometry/GeometryAll.h"

enum GeometryType { Sphere, Capsule, AABB, OBB, Plane, Ray, Triangle };
struct Geometry; // Empty container

struct GeometryObject 
{
	~GeometryObject() 
	{
		if (geometry != nullptr)
			delete geometry;
		geometry = nullptr;
	}

	GeometryType geometryType = GeometryType::Sphere;
	Geometry* geometry = nullptr;
};

class ModuleSceneIntro : public Module
{
public:

	ModuleSceneIntro(Application* app, bool start_enabled = true);
	~ModuleSceneIntro();

	bool Start();
	update_status Update(float dt);
	bool CleanUp();

private:

	void ShowDemoWindow();
	void ShowPerformanceWindow();
	void ShowIntersectionWindow();

	bool Intersect(GeometryObject* geometryA, GeometryObject* geometryB);
	bool SphereIntersect(math::Sphere* sphereA, GeometryObject* geometryB);
	bool CapsuleIntersect(math::Capsule* capsuleA, GeometryObject* geometryB);
	bool AABBIntersect(math::AABB* aabbA, GeometryObject* geometryB);
	bool OBBIntersect(math::OBB* obbA, GeometryObject* geometryB);
	bool PlaneIntersect(math::Plane* planeA, GeometryObject* geometryB);
	bool RayIntersect(math::Ray* rayA, GeometryObject* geometryB);
	bool TriangleIntersect(math::Triangle* triangleA, GeometryObject* geometryB);

private:

	bool showDemoWin = false;
	bool showIntersectionWin = false;
	bool showPerformanceWin = false;

	// Geometry A
	GeometryType currGeometryTypeA = GeometryType::Sphere;
	GeometryObject* currGeometryA = nullptr;

	// Geometry B
	GeometryType currGeometryTypeB = GeometryType::Sphere;
	GeometryObject* currGeometryB = nullptr;
};

#endif
