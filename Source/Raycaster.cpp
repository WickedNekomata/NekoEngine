#include "Raycaster.h"

#include "Application.h"
#include "ModuleCameraEditor.h"
#include "ComponentCamera.h"
#include "ModuleWindow.h"
#include "GameObject.h"
#include "ComponentMesh.h"
#include "ComponentTransform.h"

#include "MathGeoLib/include/Geometry/Triangle.h"
#include "MathGeoLib/include/Geometry/LineSegment.h"

Raycaster::Raycaster()
{
}

Raycaster::~Raycaster()
{
}

void Raycaster::ScreenPointToRay(int posX, int posY, float& shortestDistance, math::float3& shortestHitPoint, GameObject** hitGo)
{
	*hitGo = nullptr;
	shortestHitPoint = math::float3::inf;
	shortestDistance = FLOAT_INF;

	int winWidth = App->window->GetWindowWidth();
	int winHeight = App->window->GetWindowHeight();
	float normalized_x = -(1.0f - (float(posX) * 2.0f) / winWidth);
	float normalized_y = 1.0f - (float(posY) * 2.0f) / winHeight;

	math::LineSegment raycast = App->camera->camera->cameraFrustum.UnProjectLineSegment(normalized_x, normalized_y);
	
	std::vector<GameObject*> hits;
	App->scene->quadtree.CollectIntersections(hits, raycast);

	Mesh* currentMesh = nullptr;

	for (int i = 0; i < hits.size(); ++i)
	{
		math::Triangle tri;
		math::LineSegment localSpaceSegment(raycast);
		localSpaceSegment.Transform(hits[i]->transform->GetGlobalMatrix().Inverted());

		currentMesh = hits[i]->meshRenderer->mesh;
		for (int j = 0; j < currentMesh->indicesSize;)
		{
			tri.a = math::float3(currentMesh->vertices[currentMesh->indices[j] * 3], currentMesh->vertices[currentMesh->indices[j] * 3 + 1], currentMesh->vertices[currentMesh->indices[j] * 3 + 2]);  j++;
			tri.b = math::float3(currentMesh->vertices[currentMesh->indices[j] * 3], currentMesh->vertices[currentMesh->indices[j] * 3 + 1], currentMesh->vertices[currentMesh->indices[j] * 3 + 2]);  j++;
			tri.c = math::float3(currentMesh->vertices[currentMesh->indices[j] * 3], currentMesh->vertices[currentMesh->indices[j] * 3 + 1], currentMesh->vertices[currentMesh->indices[j] * 3 + 2]);  j++;

			float distance;
			math::float3 hitPoint;
			if (localSpaceSegment.Intersects(tri, &distance, &hitPoint))
			{
				if (shortestDistance > distance)
				{
					shortestDistance = distance;
					shortestHitPoint = hitPoint;
					*hitGo = (GameObject*)hits[i];
				}
			}
		}
	}
}
