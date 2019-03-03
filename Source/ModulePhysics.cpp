#include "ModulePhysics.h"

#include "Application.h"
#include "ModuleTimeManager.h"
#include "ModuleRenderer3D.h"
#include "GameObject.h"

// *****Debug*****
#include "ModuleWindow.h"
#include "ModuleInput.h"
#include "ModuleCameraEditor.h"
#include "ModuleResourceManager.h"
#include "ModuleScene.h"
#include "ModuleGOs.h"
#include "ModuleGui.h"
#include "ComponentCamera.h"
#include "ComponentTransform.h"
#include "DebugDrawer.h"
#include "ResourcePrefab.h"

#include "MathGeoLib\include\Geometry\Frustum.h"
#include "MathGeoLib\include\Geometry\LineSegment.h"
//_*****Debug*****

#include "ComponentRigidActor.h"
#include "ComponentRigidStatic.h"
#include "ComponentRigidDynamic.h"
#include "ComponentCollider.h"
#include "ComponentBoxCollider.h"
#include "ComponentSphereCollider.h"
#include "ComponentCapsuleCollider.h"
#include "ComponentPlaneCollider.h"
#include "ComponentJoint.h"
#include "ComponentFixedJoint.h"
#include "ComponentDistanceJoint.h"
#include "ComponentSphericalJoint.h"
#include "ComponentRevoluteJoint.h"
#include "ComponentPrismaticJoint.h"
#include "ComponentD6Joint.h"

#include "PhysicsConstants.h"
#include "SimulationEvents.h"

#include <assert.h>

#include "MathGeoLib\include\Math\float2.h"

#ifdef _DEBUG
#pragma comment(lib, "physx\\libx86\\debugx86\\PhysX_32.lib")
#pragma comment(lib, "physx\\libx86\\debugx86\\PhysXCommon_32.lib")
#pragma comment(lib, "physx\\libx86\\debugx86\\PhysXFoundation_32.lib")
#pragma comment(lib, "physx\\libx86\\debugx86\\PhysXExtensions_static_32.lib")
#endif

#ifdef NDEBUG
#pragma comment(lib, "physx\\libx86\\releasex86\\PhysX_32.lib")
#pragma comment(lib, "physx\\libx86\\releasex86\\PhysXCommon_32.lib")
#pragma comment(lib, "physx\\libx86\\releasex86\\PhysXFoundation_32.lib")
#pragma comment(lib, "physx\\libx86\\releasex86\\PhysXExtensions_static_32.lib")
#endif

DefaultErrorCallback::DefaultErrorCallback() {}

DefaultErrorCallback::~DefaultErrorCallback() {}

void DefaultErrorCallback::reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line)
{
	switch (code)
	{
	case physx::PxErrorCode::Enum::eNO_ERROR:
		CONSOLE_LOG(LogTypes::Normal, "eNO_ERROR: %s", message);
		break;
	case physx::PxErrorCode::Enum::eDEBUG_INFO: //! \brief An informational message
		CONSOLE_LOG(LogTypes::Normal, "eDEBUG_INFO: %s", message);
		break;
	case physx::PxErrorCode::Enum::eDEBUG_WARNING: //! \brief a warning message for the user to help with debugging
		CONSOLE_LOG(LogTypes::Warning, "eDEBUG_WARNING: %s", message);
		break;
	case physx::PxErrorCode::Enum::eINVALID_PARAMETER: //! \brief method called with invalid parameter(s)
		CONSOLE_LOG(LogTypes::Error, "eINVALID_PARAMETER: %s", message);
		break;
	case physx::PxErrorCode::Enum::eINVALID_OPERATION: //! \brief method was called at a time when an operation is not possible
		CONSOLE_LOG(LogTypes::Error, "eINVALID_OPERATION: %s", message);
		break;
	case physx::PxErrorCode::Enum::eOUT_OF_MEMORY: //! \brief method failed to allocate some memory
		CONSOLE_LOG(LogTypes::Error, "eOUT_OF_MEMORY: %s", message);
		break;
	case physx::PxErrorCode::Enum::eINTERNAL_ERROR: //! \brief The library failed for some reason. Possibly you have passed invalid values like NaNs, which are not checked for
		CONSOLE_LOG(LogTypes::Error, "eINTERNAL_ERROR: %s", message);
		break;
	case physx::PxErrorCode::Enum::eABORT: //! \brief An unrecoverable error, execution should be halted and log output flushed
		CONSOLE_LOG(LogTypes::Error, "eABORT: %s", message);
		break;
	case physx::PxErrorCode::Enum::ePERF_WARNING: //! \brief The SDK has determined that an operation may result in poor performance
		CONSOLE_LOG(LogTypes::Warning, "ePERF_WARNING: %s", message);
		break;
	case physx::PxErrorCode::Enum::eMASK_ALL: //! \brief A bit mask for including all errors
		CONSOLE_LOG(LogTypes::Error, "eMASK_ALL: %s", message);
		break;
	}
}

// ----------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------

physx::PxFilterFlags FilterShader(
	physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0,
	physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1,
	physx::PxPairFlags& pairFlags, const void* constantBlock, physx::PxU32 constantBlockSize)
{
	if ((filterData0.word0 != 0 || filterData1.word0 != 0) &&
		!(filterData0.word0 & filterData1.word1 || filterData1.word0 & filterData0.word1))
		return physx::PxFilterFlag::eSUPPRESS;

	// Let triggers through
	if (physx::PxFilterObjectIsTrigger(attributes0) || physx::PxFilterObjectIsTrigger(attributes1))
		pairFlags = physx::PxPairFlag::eTRIGGER_DEFAULT;
	else
	{
		// Generate contacts for all that were not filtered above
		pairFlags = physx::PxPairFlag::eCONTACT_DEFAULT;
		pairFlags |= physx::PxPairFlag::eNOTIFY_CONTACT_POINTS;
		pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_FOUND;
		pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_PERSISTS;
		pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_LOST;
	}

	return physx::PxFilterFlags();
}

// ----------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------

ModulePhysics::ModulePhysics(bool start_enabled) : Module(start_enabled)
{
	name = "Physics";
}

ModulePhysics::~ModulePhysics() {}

bool ModulePhysics::Init(JSON_Object* jObject)
{
	gravity = math::float3(PhysicsConstants::GRAVITY_X, PhysicsConstants::GRAVITY_Y, PhysicsConstants::GRAVITY_Z);

	return true;
}

physx::PxDefaultAllocator	gAllocator;
DefaultErrorCallback		gErrorCallback;

bool ModulePhysics::Start()
{
	// Foundation
	gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
	assert(gFoundation != nullptr && "MODULE PHYSICS: PxCreateFoundation failed!");

	// Physics
	bool recordMemoryAllocations = true; // whether to perform memory profiling
	//physx::PxTolerancesScale scale;
	//scale.length = 100.0f; // typical length of an object
	//scale.speed = 981.0f; // typical speed of an object
	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, physx::PxTolerancesScale(), recordMemoryAllocations);
	assert(gPhysics != nullptr && "MODULE PHYSICS: PxCreatePhysics failed!");

	// Scene
	simulationEventCallback = new SimulationEventCallback(this);

	physx::PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
	sceneDesc.gravity = physx::PxVec3(gravity.x, gravity.y, gravity.z);
	gDispatcher = physx::PxDefaultCpuDispatcherCreate(1);
	sceneDesc.cpuDispatcher = gDispatcher;
	sceneDesc.filterShader = FilterShader;
	sceneDesc.simulationEventCallback = simulationEventCallback;
	gScene = gPhysics->createScene(sceneDesc);
	assert(gScene != nullptr && "MODULE PHYSICS: createScene failed!");

	// Default material
	defaultMaterial = gPhysics->createMaterial(PhysicsConstants::STATIC_FRICTION, PhysicsConstants::DYNAMIC_FRICTION, PhysicsConstants::RESTITUTION);

	// Ground
	physx::PxShape* planeShape = CreateShape(physx::PxPlaneGeometry(), *defaultMaterial);
	physx::PxFilterData filterData;
	filterData.word0 = App->layers->GetLayer(0)->GetFilterGroup();
	filterData.word1 = App->layers->GetLayer(0)->GetFilterMask();
	planeShape->setSimulationFilterData(filterData);
	physx::PxRigidStatic* groundPlane = CreateRigidStatic(physx::PxTransformFromPlaneEquation(physx::PxPlane(0.0f, 1.0f, 0.0f, 0.0f)), *planeShape);

	debugRay = math::Ray(math::float3::zero, math::float3::zero);

	return true;
}

update_status ModulePhysics::PreUpdate()
{
	return UPDATE_CONTINUE;
}

update_status ModulePhysics::Update()
{
	update_status updateStatus = update_status::UPDATE_CONTINUE;

	if (App->GetEngineState() == engine_states::ENGINE_PLAY
		|| App->GetEngineState() == engine_states::ENGINE_STEP)
	{
		// Step physics
		gAccumulator += App->timeManager->GetDt();
		//gAccumulator += App->timeManager->GetRealDt();
		if (gAccumulator >= PhysicsConstants::FIXED_DT)
		{
			gAccumulator = 0.0f;

			updateStatus = FixedUpdate();

			gScene->simulate(PhysicsConstants::FIXED_DT); // moves all objects in the scene forward by FIXED_DT
			gScene->fetchResults(true); // allows the simulation to finish and return the results

			// Update colliders
			for (std::vector<ComponentCollider*>::const_iterator it = colliderComponents.begin(); it != colliderComponents.end(); ++it)
				(*it)->Update();

			// Update rigid actors
			for (std::vector<ComponentRigidActor*>::const_iterator it = rigidActorComponents.begin(); it != rigidActorComponents.end(); ++it)
				(*it)->Update();
		}
	}

	// *****Debug*****
	//if (debugRay.IsFinite())
		//App->debugDrawer->DebugDraw(debugRay, Red);
	//if (debugTransform.IsFinite())
		//App->debugDrawer->DebugDrawSphere(debugRadius, Red, debugTransform);
	//_*****Debug*****

	return updateStatus;
}

update_status ModulePhysics::FixedUpdate()
{
#ifndef GAMEMODE
	if (App->gui->WantTextInput() || App->gui->IsMouseHoveringAnyWindow())
		return UPDATE_CONTINUE;
#endif

	//Debug();
	DestroyChest();

	return UPDATE_CONTINUE;
}

update_status ModulePhysics::PostUpdate()
{
	return UPDATE_CONTINUE;
}

bool ModulePhysics::CleanUp()
{
	colliderComponents.clear();
	rigidActorComponents.clear();

	defaultMaterial->release();
	defaultMaterial = nullptr;

	gScene->release();
	gScene = nullptr;

	gDispatcher->release();
	gDispatcher = nullptr;

	gPhysics->release();
	gPhysics = nullptr;

	gFoundation->release();
	gFoundation = nullptr;

	RELEASE(simulationEventCallback);

	return true;
}

void ModulePhysics::OnSystemEvent(System_Event event)
{
	switch (event.type)
	{
	case System_Event_Type::LayerChanged: // LayerEvent

		// Update filtering (specific collider)
		for (uint i = 0; i < colliderComponents.size(); ++i)
		{
			if (colliderComponents[i] == event.layerEvent.collider)
			{
				Layer* layer = App->layers->GetLayer(event.layerEvent.layer);
				colliderComponents[i]->SetFiltering(layer->GetFilterGroup(), layer->GetFilterMask());
			}
		}

		break;

	case System_Event_Type::LayerFilterMaskChanged: // LayerEvent

		// Update filtering (all colliders)
		for (uint i = 0; i < colliderComponents.size(); ++i)
		{
			Layer* layer = App->layers->GetLayer(colliderComponents[i]->GetParent()->GetLayer());
			if (layer->GetNumber() == event.layerEvent.layer)
				colliderComponents[i]->SetFiltering(layer->GetFilterGroup(), layer->GetFilterMask());
		}

		break;
	}
}

// *****Debug*****
void ModulePhysics::Debug()
{
	int winWidth = App->window->GetWindowWidth();
	int winHeight = App->window->GetWindowHeight();
	float normalizedX = -(1.0f - (float(App->input->GetMouseX()) * 2.0f) / winWidth);
	float normalizedY = 1.0f - (float(App->input->GetMouseY()) * 2.0f) / winHeight;
	math::Ray ray = App->renderer3D->GetCurrentCamera()->frustum.UnProjectLineSegment(normalizedX, normalizedY).ToRay();

	if (App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_DOWN)
	{
		// Raycast
		debugRay = ray;

		RaycastHit hitInfo;
		std::vector<RaycastHit> touchesInfo;
		if (Raycast(ray.pos, ray.dir, hitInfo, touchesInfo, FLT_MAX, BIT_SHIFT(0) | BIT_SHIFT(1) | BIT_SHIFT(2)))
		{
			// Hit
			if (hitInfo.GetGameObject() != nullptr)
			{
				CONSOLE_LOG(LogTypes::Normal, "The ray hit the game object '%s'", hitInfo.GetGameObject()->GetName());

				// Distance (and closest point) and AABB
				physx::PxShape* gShape = hitInfo.GetCollider()->GetShape();

				math::float4x4 gameObjectGlobalMatrix = hitInfo.GetGameObject()->transform->GetGlobalMatrix();
				math::float3 position = math::float3::zero;
				math::Quat rotation = math::Quat::identity;
				math::float3 scale = math::float3::one;
				gameObjectGlobalMatrix.Decompose(position, rotation, scale);
				physx::PxTransform gameObjectTransform = physx::PxTransform(physx::PxVec3(position.x, position.y, position.z),
					physx::PxQuat(rotation.x, rotation.y, rotation.z, rotation.w));

				physx::PxTransform transform = gameObjectTransform * gShape->getLocalPose();

				switch (gShape->getGeometryType())
				{
				case physx::PxGeometryType::Enum::eSPHERE:
				{
					physx::PxSphereGeometry gSphereGeometry;
					gShape->getSphereGeometry(gSphereGeometry);

					// Distance (and closest point)
					math::float3 closestPoint = math::float3::zero;
					float distance = ComponentCollider::GetPointToGeometryObjectDistance(ray.pos, gSphereGeometry, transform, closestPoint);
					CONSOLE_LOG(LogTypes::Normal, "The distance is %f and the closest point is (%f, %f, %f)", distance, closestPoint.x, closestPoint.y, closestPoint.z);

					// AABB
					physx::PxBounds3 bounds = ComponentCollider::GetGeometryObjectAABB(gSphereGeometry, transform);
					physx::PxVec3 dimensions = bounds.getDimensions();
					CONSOLE_LOG(LogTypes::Normal, "The bounds are (%f, %f, %f)", dimensions.x, dimensions.y, dimensions.z);
				}
				break;
				case physx::PxGeometryType::Enum::eCAPSULE:
				{
					physx::PxCapsuleGeometry gCapsuleGeometry;
					gShape->getCapsuleGeometry(gCapsuleGeometry);

					// Distance (and closest point)
					math::float3 closestPoint = math::float3::zero;
					float distance = ComponentCollider::GetPointToGeometryObjectDistance(ray.pos, gCapsuleGeometry, transform, closestPoint);
					CONSOLE_LOG(LogTypes::Normal, "The distance is %f and the closest point is (%f, %f, %f)", distance, closestPoint.x, closestPoint.y, closestPoint.z);

					// AABB
					physx::PxBounds3 bounds = ComponentCollider::GetGeometryObjectAABB(gCapsuleGeometry, transform);
					physx::PxVec3 dimensions = bounds.getDimensions();
					CONSOLE_LOG(LogTypes::Normal, "The bounds are (%f, %f, %f)", dimensions.x, dimensions.y, dimensions.z);
				}
				break;
				case physx::PxGeometryType::Enum::eBOX:
				{
					physx::PxBoxGeometry gBoxGeometry;
					gShape->getBoxGeometry(gBoxGeometry);

					// Distance (and closest point)
					math::float3 closestPoint = math::float3::zero;
					float distance = ComponentCollider::GetPointToGeometryObjectDistance(ray.pos, gBoxGeometry, transform, closestPoint);
					CONSOLE_LOG(LogTypes::Normal, "The distance is %f and the closest point is (%f, %f, %f)", distance, closestPoint.x, closestPoint.y, closestPoint.z);

					// AABB
					physx::PxBounds3 bounds = ComponentCollider::GetGeometryObjectAABB(gBoxGeometry, transform);
					physx::PxVec3 dimensions = bounds.getDimensions();
					CONSOLE_LOG(LogTypes::Normal, "The bounds are (%f, %f, %f)", dimensions.x, dimensions.y, dimensions.z);
				}
				break;
				case physx::PxGeometryType::Enum::ePLANE:
				{
					physx::PxPlaneGeometry gPlaneGeometry;
					gShape->getPlaneGeometry(gPlaneGeometry);

					// AABB
					physx::PxBounds3 bounds = ComponentCollider::GetGeometryObjectAABB(gPlaneGeometry, transform);
					physx::PxVec3 dimensions = bounds.getDimensions();
					CONSOLE_LOG(LogTypes::Normal, "The bounds are (%f, %f, %f)", dimensions.x, dimensions.y, dimensions.z);
				}
				break;
				}
			}

			// Touches
			for (uint i = 0; i < touchesInfo.size(); ++i)
			{
				if (touchesInfo[i].GetGameObject() != nullptr)
					CONSOLE_LOG(LogTypes::Normal, "The ray also touched the game object '%s'", touchesInfo[i].GetGameObject()->GetName());
			}
		}
	}
	else if (App->input->GetMouseButton(SDL_BUTTON_RIGHT) == KEY_DOWN)
	{
		// Sweep
		SweepHit hitInfo;
		physx::PxTransform transform(physx::PxVec3(ray.pos.x, ray.pos.y, ray.pos.z));
		if (Sweep(physx::PxBoxGeometry(PhysicsConstants::GEOMETRY_HALF_SIZE, PhysicsConstants::GEOMETRY_HALF_SIZE, PhysicsConstants::GEOMETRY_HALF_SIZE), transform, ray.dir, hitInfo))
		{
			// Hit
			if (hitInfo.GetGameObject() != nullptr)
				CONSOLE_LOG(LogTypes::Normal, "The sweep hit the game object '%s'", hitInfo.GetGameObject()->GetName());
		}
	}
	else if (App->input->GetMouseButton(SDL_BUTTON_MIDDLE) == KEY_DOWN)
	{
		// Overlap
		std::vector<OverlapHit> touchesInfo;
		if (OverlapSphere(5.0f, math::float3(0.0f, 0.0f, 0.0f), touchesInfo))
		{
			// Touches
			for (uint i = 0; i < touchesInfo.size(); ++i)
			{
				if (touchesInfo[i].GetGameObject() != nullptr)
					CONSOLE_LOG(LogTypes::Normal, "The overlap touched the game object '%s'", touchesInfo[i].GetGameObject()->GetName());
			}
		}
	}
}

void ModulePhysics::DestroyChest()
{
	if (App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_DOWN)
	{
		// Raycast
		RaycastHit hitInfo;
		std::vector<RaycastHit> touchesInfo;
		math::Ray ray = App->renderer3D->GetCurrentCamera()->ScreenToRay(math::float2(App->input->GetMouseX(), App->input->GetMouseY()));

		// Layer
		Layer* chestLayer = App->layers->GetLayer(App->layers->NameToNumber("Destructibles"));
		uint filterGroup = chestLayer->GetFilterGroup();

		if (Raycast(ray.pos, ray.dir, hitInfo, FLT_MAX, filterGroup))
		{
			GameObject* gameObject = hitInfo.GetGameObject();

			// Hit
			if (gameObject != nullptr)
			{
				// 1. Destroy game object (original mesh)
				gameObject->Destroy();

				// 2. Instantiate game object (broken mesh)
				ResourcePrefab* prefab = nullptr;

				std::vector<Resource*> prefabs = App->res->GetResourcesByType(ResourceTypes::PrefabResource);
				for (uint i = 0; i < prefabs.size(); ++i)
				{
					if (strcmp(prefabs[i]->GetName(), "BrokenChestFinal.pfb") == 0)
					{
						prefab = (ResourcePrefab*)prefabs[i];
						break;
					}
				}

				if (prefab != nullptr)
				{
					App->res->SetAsUsed(prefab->GetUuid());

					GameObject* brokenChest = prefab->GetRoot();

					if (brokenChest != nullptr)
					{
						GameObject* brokenChestInstance = App->GOs->Instanciate(brokenChest, App->scene->root);

						if (brokenChestInstance != nullptr)
						{
							App->res->SetAsUnused(prefab->GetUuid());

							// a) Move broken mesh to original mesh
							math::float4x4 globalMatrix = gameObject->transform->GetGlobalMatrix();
							brokenChestInstance->transform->SetMatrixFromGlobal(globalMatrix);

							// b) Apply forces to broken mesh
							std::vector<GameObject*> go;
							brokenChestInstance->GetChildrenVector(go, false);

							for (uint i = 0; i < go.size(); ++i)
							{
								ComponentRigidDynamic* rigidDynamic = (ComponentRigidDynamic*)go[i]->cmp_rigidActor;
								
								if (rigidDynamic != nullptr)
								{
									float force = 10.0f;
									int max = 1;
									int min = -1;

									float forceX = force * ((rand() % (max - min)) + min);
									float forceY = force * ((rand() % (max - min)) + min);
									float forceZ = force * ((rand() % (max - min)) + min);

									rigidDynamic->AddForce(math::float3(forceX, forceY, forceZ), physx::PxForceMode::eIMPULSE);
								}
							}
						}					
					}
				}
			}
		}
	}
}
//_*****Debug*****

// ----------------------------------------------------------------------------------------------------

physx::PxRigidStatic* ModulePhysics::CreateRigidStatic(const physx::PxTransform& transform, physx::PxShape& shape) const
{
	assert(transform.isFinite());
	physx::PxRigidStatic* rigidStatic = physx::PxCreateStatic(*gPhysics, transform, shape);

	AddActor(*rigidStatic);

	return rigidStatic;
}

physx::PxRigidDynamic* ModulePhysics::CreateRigidDynamic(const physx::PxTransform& transform, physx::PxShape& shape, float density, bool isKinematic) const
{
	assert(transform.isFinite());
	physx::PxRigidDynamic* rigidDynamic = nullptr;
	if (isKinematic)
		rigidDynamic = physx::PxCreateKinematic(*gPhysics, transform, shape, density);
	else
		rigidDynamic = physx::PxCreateDynamic(*gPhysics, transform, shape, density);

	AddActor(*rigidDynamic);

	return rigidDynamic;
}

void ModulePhysics::AddActor(physx::PxActor& actor) const
{
	gScene->addActor(actor);
	CONSOLE_LOG(LogTypes::Normal, "gScene actors after adding an actor: %i", gScene->getNbActors(physx::PxActorTypeFlag::Enum::eRIGID_STATIC | physx::PxActorTypeFlag::Enum::eRIGID_DYNAMIC));
}

void ModulePhysics::RemoveActor(physx::PxActor& actor) const
{
	gScene->removeActor(actor);
	CONSOLE_LOG(LogTypes::Normal, "gScene actors after removing an actor: %i", gScene->getNbActors(physx::PxActorTypeFlag::Enum::eRIGID_STATIC | physx::PxActorTypeFlag::Enum::eRIGID_DYNAMIC));
}

physx::PxShape* ModulePhysics::CreateShape(const physx::PxGeometry& geometry, const physx::PxMaterial& material, bool isExclusive) const
{
	return gPhysics->createShape(geometry, material, isExclusive);
}

physx::PxJoint* ModulePhysics::CreateJoint(JointTypes jointType, physx::PxRigidActor* actor0, const physx::PxTransform& localFrame0, physx::PxRigidActor* actor1, physx::PxTransform& localFrame1) const
{
	assert(actor0 != nullptr && actor1 != nullptr && localFrame0.isFinite() && localFrame1.isFinite());

	switch (jointType)
	{
	case JointTypes::FixedJoint:
		return physx::PxFixedJointCreate(*gPhysics, actor0, localFrame0, actor1, localFrame1);
		break;
	case JointTypes::DistanceJoint:
		return physx::PxDistanceJointCreate(*gPhysics, actor0, localFrame0, actor1, localFrame1);
		break;
	case JointTypes::SphericalJoint:
		return physx::PxSphericalJointCreate(*gPhysics, actor0, localFrame0, actor1, localFrame1);
		break;
	case JointTypes::RevoluteJoint:
		return physx::PxRevoluteJointCreate(*gPhysics, actor0, localFrame0, actor1, localFrame1);
		break;
	case JointTypes::PrismaticJoint:
		return physx::PxPrismaticJointCreate(*gPhysics, actor0, localFrame0, actor1, localFrame1);
		break;
	case JointTypes::D6Joint:
		return physx::PxD6JointCreate(*gPhysics, actor0, localFrame0, actor1, localFrame1);
		break;
	}

	return nullptr;
}

// ----------------------------------------------------------------------------------------------------

ComponentRigidActor* ModulePhysics::CreateRigidActorComponent(GameObject* parent, ComponentTypes componentRigidActorType)
{
	ComponentRigidActor* newComponent = nullptr;
	switch (componentRigidActorType)
	{
	case ComponentTypes::RigidStaticComponent:
		newComponent = new ComponentRigidStatic(parent);
		break;
	case ComponentTypes::RigidDynamicComponent:
		newComponent = new ComponentRigidDynamic(parent);
		break;
	}
	assert(newComponent != nullptr);

	std::vector<ComponentRigidActor*>::const_iterator it = std::find(rigidActorComponents.begin(), rigidActorComponents.end(), newComponent);
	assert(it == rigidActorComponents.end());
	rigidActorComponents.push_back(newComponent);

	return newComponent;
}

bool ModulePhysics::AddRigidActorComponent(ComponentRigidActor* toAdd)
{
	assert(toAdd != nullptr);
	bool ret = true;

	std::vector<ComponentRigidActor*>::const_iterator it = std::find(rigidActorComponents.begin(), rigidActorComponents.end(), toAdd);
	ret = it == rigidActorComponents.end();

	if (ret)
		rigidActorComponents.push_back(toAdd);

	return ret;
}

bool ModulePhysics::EraseRigidActorComponent(ComponentRigidActor* toErase)
{
	assert(toErase != nullptr);
	bool ret = false;

	std::vector<ComponentRigidActor*>::const_iterator it = std::find(rigidActorComponents.begin(), rigidActorComponents.end(), toErase);
	ret = it != rigidActorComponents.end();

	if (ret)
		rigidActorComponents.erase(it);

	return ret;
}

std::vector<ComponentRigidActor*> ModulePhysics::GetRigidActorComponents() const
{
	return rigidActorComponents;
}

ComponentRigidActor* ModulePhysics::FindRigidActorComponentByActor(physx::PxActor* actor) const
{
	assert(actor != nullptr);
	for (std::vector<ComponentRigidActor*>::const_iterator it = rigidActorComponents.begin(); it != rigidActorComponents.end(); ++it)
	{
		if ((*it)->GetActor() == actor)
			return *it;
	}

	return nullptr;
}

ComponentCollider* ModulePhysics::CreateColliderComponent(GameObject* parent, ComponentTypes componentColliderType)
{
	ComponentCollider* newComponent = nullptr;
	switch (componentColliderType)
	{
	case ComponentTypes::BoxColliderComponent:
		newComponent = new ComponentBoxCollider(parent);
		break;
	case ComponentTypes::SphereColliderComponent:
		newComponent = new ComponentSphereCollider(parent);
		break;
	case ComponentTypes::CapsuleColliderComponent:
		newComponent = new ComponentCapsuleCollider(parent);
		break;
	case ComponentTypes::PlaneColliderComponent:
		newComponent = new ComponentPlaneCollider(parent);
		break;
	}
	assert(newComponent != nullptr);

	std::vector<ComponentCollider*>::const_iterator it = std::find(colliderComponents.begin(), colliderComponents.end(), newComponent);
	assert(it == colliderComponents.end());
	colliderComponents.push_back(newComponent);

	return newComponent;
}

bool ModulePhysics::AddColliderComponent(ComponentCollider* toAdd)
{
	assert(toAdd != nullptr);
	bool ret = true;

	std::vector<ComponentCollider*>::const_iterator it = std::find(colliderComponents.begin(), colliderComponents.end(), toAdd);
	ret = it == colliderComponents.end();

	if (ret)
		colliderComponents.push_back(toAdd);

	return ret;
}

bool ModulePhysics::EraseColliderComponent(ComponentCollider* toErase)
{
	assert(toErase != nullptr);
	bool ret = false;

	std::vector<ComponentCollider*>::const_iterator it = std::find(colliderComponents.begin(), colliderComponents.end(), toErase);
	ret = it != colliderComponents.end();

	if (ret)
		colliderComponents.erase(it);

	return ret;
}

std::vector<ComponentCollider*> ModulePhysics::GetColliderComponents() const
{
	return colliderComponents;
}

ComponentCollider* ModulePhysics::FindColliderComponentByShape(physx::PxShape* shape) const
{
	assert(shape != nullptr);
	for (std::vector<ComponentCollider*>::const_iterator it = colliderComponents.begin(); it != colliderComponents.end(); ++it)
	{
		if ((*it)->GetShape() == shape)
			return *it;
	}

	return nullptr;
}

ComponentJoint* ModulePhysics::CreateJointComponent(GameObject* parent, ComponentTypes componentJointType)
{
	ComponentJoint* newComponent = nullptr;
	switch (componentJointType)
	{
	case ComponentTypes::FixedJointComponent:
		newComponent = new ComponentFixedJoint(parent, componentJointType);
		break;
	case ComponentTypes::DistanceJointComponent:
		newComponent = new ComponentDistanceJoint(parent, componentJointType);
		break;
	case ComponentTypes::SphericalJointComponent:
		newComponent = new ComponentSphericalJoint(parent, componentJointType);
		break;
	case ComponentTypes::RevoluteJointComponent:
		newComponent = new ComponentRevoluteJoint(parent, componentJointType);
		break;
	case ComponentTypes::PrismaticJointComponent:
		newComponent = new ComponentPrismaticJoint(parent, componentJointType);
		break;
	case ComponentTypes::D6JointComponent:
		newComponent = new ComponentD6Joint(parent, componentJointType);
		break;
	}
	assert(newComponent != nullptr);

	std::vector<ComponentJoint*>::const_iterator it = std::find(jointComponents.begin(), jointComponents.end(), newComponent);
	assert(it == jointComponents.end());
	jointComponents.push_back(newComponent);

	return newComponent;
}

bool ModulePhysics::AddJointComponent(ComponentJoint* toAdd)
{
	assert(toAdd != nullptr);
	bool ret = true;

	std::vector<ComponentJoint*>::const_iterator it = std::find(jointComponents.begin(), jointComponents.end(), toAdd);
	ret = it == jointComponents.end();

	if (ret)
		jointComponents.push_back(toAdd);

	return ret;
}

bool ModulePhysics::EraseJointComponent(ComponentJoint* toErase)
{
	assert(toErase != nullptr);
	bool ret = false;

	std::vector<ComponentJoint*>::const_iterator it = std::find(jointComponents.begin(), jointComponents.end(), toErase);
	ret = it != jointComponents.end();

	if (ret)
		jointComponents.erase(it);

	return ret;
}

std::vector<ComponentJoint*> ModulePhysics::GetJointComponents() const
{
	return jointComponents;
}

ComponentJoint* ModulePhysics::FindJointComponentByJoint(physx::PxJoint* joint) const
{
	assert(joint != nullptr);
	for (std::vector<ComponentJoint*>::const_iterator it = jointComponents.begin(); it != jointComponents.end(); ++it)
	{
		if ((*it)->GetJoint() == joint)
			return *it;
	}

	return nullptr;
}

// ----------------------------------------------------------------------------------------------------

void ModulePhysics::OnSimulationEvent(ComponentRigidActor* actor, SimulationEventTypes simulationEventType) const
{
	if (actor == nullptr)
		return;

	switch (simulationEventType)
	{
	case SimulationEventTypes::OnWake:
		actor->OnWake();
		break;
	case SimulationEventTypes::OnSleep:
		actor->OnSleep();
		break;
	}
}

void ModulePhysics::OnCollision(ComponentCollider* collider, Collision& collision, CollisionTypes collisionType) const
{
	if (collider == nullptr)
		return;

	switch (collisionType)
	{
	case OnCollisionEnter:
		collider->OnCollisionEnter(collision);
		break;
	case OnCollisionStay:
		collider->OnCollisionStay(collision);
		break;
	case OnCollisionExit:
		collider->OnCollisionExit(collision);
		break;
	case OnTriggerEnter:
		collider->OnTriggerEnter(collision);
		break;
	case OnTriggerExit:
		collider->OnTriggerExit(collision);
		break;
	}
}

// ----------------------------------------------------------------------------------------------------

// Raycast: traces a point along a line segment until it hits a geometry object
// Raycast with multiple touches and a blocking hit
bool ModulePhysics::Raycast(math::float3& origin, math::float3& direction, RaycastHit& hitInfo, std::vector<RaycastHit>& touchesInfo, float maxDistance, uint filterMask, SceneQueryFlags sceneQueryFlags) const
{
	assert(origin.IsFinite() && direction.IsFinite() && maxDistance >= 0.0f);
	direction.Normalize();

	physx::PxHitFlags hitFlags;
	hitFlags |= physx::PxHitFlag::eDEFAULT;
	hitFlags |= physx::PxHitFlag::eUV;

	physx::PxQueryFilterData filterData;
	filterData.data.word0 = filterMask; // raycast against this filter mask
	if (!(sceneQueryFlags & physx::PxQueryFlag::eSTATIC))
		filterData.flags &= ~physx::PxQueryFlag::eSTATIC;
	if (!(sceneQueryFlags & physx::PxQueryFlag::eDYNAMIC))
		filterData.flags &= ~physx::PxQueryFlag::eDYNAMIC;

	physx::PxRaycastHit hitBuffer[PhysicsConstants::MAX_HITS];
	physx::PxRaycastBuffer hitsBuffer(hitBuffer, PhysicsConstants::MAX_HITS);

	bool status = gScene->raycast(physx::PxVec3(origin.x, origin.y, origin.z), physx::PxVec3(direction.x, direction.y, direction.z),
		maxDistance, hitsBuffer, hitFlags, filterData);

	if (status)
	{
		// Touches
		std::vector<RaycastHit> touchesInfoDummy;
		for (physx::PxU32 i = 0; i < hitsBuffer.nbTouches; ++i)
		{
			assert(hitsBuffer.touches[i].distance <= hitsBuffer.block.distance);

			ComponentCollider* collider = FindColliderComponentByShape(hitsBuffer.touches[i].shape);
			ComponentRigidActor* actor = FindRigidActorComponentByActor(hitsBuffer.touches[i].actor);
			GameObject* gameObject = actor->GetParent();

			math::float3 point = math::float3::zero;
			if (hitsBuffer.touches[i].flags & physx::PxHitFlag::ePOSITION)
				point = math::float3(hitsBuffer.touches[i].position.x, hitsBuffer.touches[i].position.y, hitsBuffer.touches[i].position.z);
			math::float3 normal = math::float3::zero;
			if (hitsBuffer.touches[i].flags & physx::PxHitFlag::eNORMAL)
				normal = math::float3(hitsBuffer.touches[i].normal.x, hitsBuffer.touches[i].normal.y, hitsBuffer.touches[i].normal.z);
			math::float2 texCoord = math::float2::zero;
			if (hitsBuffer.touches[i].flags & physx::PxHitFlag::eUV)
				texCoord = math::float2(hitsBuffer.touches[i].u, hitsBuffer.touches[i].v);

			RaycastHit touchInfo(gameObject, collider, actor, point, normal, texCoord, hitsBuffer.touches[i].distance, hitsBuffer.touches[i].faceIndex);
			touchesInfoDummy.push_back(touchInfo);
		}

		// Hit
		if (hitsBuffer.hasBlock)
		{
			ComponentCollider* collider = FindColliderComponentByShape(hitsBuffer.block.shape);
			ComponentRigidActor* actor = FindRigidActorComponentByActor(hitsBuffer.block.actor);
			GameObject* gameObject = actor != nullptr ? actor->GetParent() : nullptr;

			hitInfo.SetCollider(collider);
			hitInfo.SetActor(actor);
			hitInfo.SetGameObject(gameObject);
			if (hitsBuffer.block.flags & physx::PxHitFlag::ePOSITION)
				hitInfo.SetPoint(math::float3(hitsBuffer.block.position.x, hitsBuffer.block.position.y, hitsBuffer.block.position.z));
			if (hitsBuffer.block.flags & physx::PxHitFlag::eNORMAL)
				hitInfo.SetNormal(math::float3(hitsBuffer.block.normal.x, hitsBuffer.block.normal.y, hitsBuffer.block.normal.z));
			if (hitsBuffer.block.flags & physx::PxHitFlag::eUV)
				hitInfo.SetTexCoord(math::float2(hitsBuffer.block.u, hitsBuffer.block.v));
			hitInfo.SetDistance(hitsBuffer.block.distance);
			hitInfo.SetFaceIndex(hitsBuffer.block.faceIndex);
		}
		else if (!touchesInfoDummy.empty())
		{
			std::sort(touchesInfoDummy.begin(), touchesInfoDummy.end(), RaycastHitComparator());

			std::vector<RaycastHit>::const_iterator it = touchesInfoDummy.begin();
			hitInfo = *it;
			it = touchesInfoDummy.erase(it);

			for (it; it != touchesInfoDummy.end(); ++it)
				touchesInfo.push_back(*it);
		}
	}

	hitsBuffer.finalizeQuery();

	return status;
}

// Raycast with a blocking hit (first encountered hit)
bool ModulePhysics::Raycast(math::float3& origin, math::float3& direction, RaycastHit& hitInfo, float maxDistance, uint filterMask, SceneQueryFlags sceneQueryFlags) const
{
	assert(origin.IsFinite() && direction.IsFinite() && maxDistance >= 0.0f);
	direction.Normalize();

	physx::PxHitFlags hitFlags;
	hitFlags |= physx::PxHitFlag::eDEFAULT;
	hitFlags |= physx::PxHitFlag::eUV;

	physx::PxQueryFilterData filterData;
	filterData.data.word0 = filterMask; // raycast against this filter mask
	filterData.flags |= physx::PxQueryFlag::eANY_HIT;
	if (!(sceneQueryFlags & physx::PxQueryFlag::eSTATIC))
		filterData.flags &= ~physx::PxQueryFlag::eSTATIC;
	if (!(sceneQueryFlags & physx::PxQueryFlag::eDYNAMIC))
		filterData.flags &= ~physx::PxQueryFlag::eDYNAMIC;

	physx::PxRaycastBuffer hitsBuffer;

	bool status = gScene->raycast(physx::PxVec3(origin.x, origin.y, origin.z), physx::PxVec3(direction.x, direction.y, direction.z),
		maxDistance, hitsBuffer, hitFlags, filterData);

	// Hit
	if (hitsBuffer.hasBlock)
	{
		ComponentCollider* collider = FindColliderComponentByShape(hitsBuffer.block.shape);
		ComponentRigidActor* actor = FindRigidActorComponentByActor(hitsBuffer.block.actor);
		GameObject* gameObject = actor != nullptr ? actor->GetParent() : nullptr;

		hitInfo.SetCollider(collider);
		hitInfo.SetActor(actor);
		hitInfo.SetGameObject(gameObject);
		if (hitsBuffer.block.flags & physx::PxHitFlag::ePOSITION)
			hitInfo.SetPoint(math::float3(hitsBuffer.block.position.x, hitsBuffer.block.position.y, hitsBuffer.block.position.z));
		if (hitsBuffer.block.flags & physx::PxHitFlag::eNORMAL)
			hitInfo.SetNormal(math::float3(hitsBuffer.block.normal.x, hitsBuffer.block.normal.y, hitsBuffer.block.normal.z));
		if (hitsBuffer.block.flags & physx::PxHitFlag::eUV)
			hitInfo.SetTexCoord(math::float2(hitsBuffer.block.u, hitsBuffer.block.v));
		hitInfo.SetDistance(hitsBuffer.block.distance);
		hitInfo.SetFaceIndex(hitsBuffer.block.faceIndex);
	}

	hitsBuffer.finalizeQuery();

	return status;
}

// Raycast with multiple touches (no blocking hits)
bool ModulePhysics::Raycast(math::float3& origin, math::float3& direction, std::vector<RaycastHit>& touchesInfo, float maxDistance, uint filterMask, SceneQueryFlags sceneQueryFlags) const
{
	assert(origin.IsFinite() && direction.IsFinite() && maxDistance >= 0.0f);
	direction.Normalize();

	physx::PxHitFlags hitFlags;
	hitFlags |= physx::PxHitFlag::eDEFAULT;
	hitFlags |= physx::PxHitFlag::eUV;

	physx::PxQueryFilterData filterData;
	filterData.data.word0 = filterMask; // raycast against this filter mask
	filterData.flags |= physx::PxQueryFlag::eNO_BLOCK;
	if (!(sceneQueryFlags & physx::PxQueryFlag::eSTATIC))
		filterData.flags &= ~physx::PxQueryFlag::eSTATIC;
	if (!(sceneQueryFlags & physx::PxQueryFlag::eDYNAMIC))
		filterData.flags &= ~physx::PxQueryFlag::eDYNAMIC;

	physx::PxRaycastHit hitBuffer[PhysicsConstants::MAX_HITS];
	physx::PxRaycastBuffer hitsBuffer(hitBuffer, PhysicsConstants::MAX_HITS);

	bool status = gScene->raycast(physx::PxVec3(origin.x, origin.y, origin.z), physx::PxVec3(direction.x, direction.y, direction.z),
		maxDistance, hitsBuffer, hitFlags, filterData);

	// Touches
	for (physx::PxU32 i = 0; i < hitsBuffer.nbTouches; ++i)
	{
		assert(hitsBuffer.touches[i].distance <= hitsBuffer.block.distance);

		ComponentCollider* collider = FindColliderComponentByShape(hitsBuffer.touches[i].shape);
		ComponentRigidActor* actor = FindRigidActorComponentByActor(hitsBuffer.touches[i].actor);
		GameObject* gameObject = actor != nullptr ? actor->GetParent() : nullptr;

		math::float3 point = math::float3::zero;
		if (hitsBuffer.touches[i].flags & physx::PxHitFlag::ePOSITION)
			point = math::float3(hitsBuffer.touches[i].position.x, hitsBuffer.touches[i].position.y, hitsBuffer.touches[i].position.z);
		math::float3 normal = math::float3::zero;
		if (hitsBuffer.touches[i].flags & physx::PxHitFlag::eNORMAL)
			normal = math::float3(hitsBuffer.touches[i].normal.x, hitsBuffer.touches[i].normal.y, hitsBuffer.touches[i].normal.z);
		math::float2 texCoord = math::float2::zero;
		if (hitsBuffer.touches[i].flags & physx::PxHitFlag::eUV)
			texCoord = math::float2(hitsBuffer.touches[i].u, hitsBuffer.touches[i].v);

		RaycastHit touchInfo(gameObject, collider, actor, point, normal, texCoord, hitsBuffer.touches[i].distance, hitsBuffer.touches[i].faceIndex);
		touchesInfo.push_back(touchInfo);
	}

	hitsBuffer.finalizeQuery();

	return status;
}

// Sweep: traces one geometry object through space to find the impact point on a second geometry object
/// Transformed box, sphere, capsule or convex geometry
bool ModulePhysics::Sweep(physx::PxGeometry& geometry, physx::PxTransform& transform, math::float3& direction, SweepHit& hitInfo, float maxDistance, float inflation, uint filterMask, SceneQueryFlags sceneQueryFlags) const
{
	assert(transform.isFinite() && direction.IsFinite());
	direction.Normalize();

	physx::PxQueryFilterData filterData;
	filterData.data.word0 = filterMask; // sweep against this filter mask
	if (!(sceneQueryFlags & physx::PxQueryFlag::eSTATIC))
		filterData.flags &= ~physx::PxQueryFlag::eSTATIC;
	if (!(sceneQueryFlags & physx::PxQueryFlag::eDYNAMIC))
		filterData.flags &= ~physx::PxQueryFlag::eDYNAMIC;

	physx::PxSweepBuffer hitsBuffer;

	bool status = gScene->sweep(geometry, transform, physx::PxVec3(direction.x, direction.y, direction.z),
		maxDistance, hitsBuffer, physx::PxHitFlag::eDEFAULT, filterData, (physx::PxQueryFilterCallback*)0, (physx::PxQueryCache*)0, inflation);

	// Hit
	if (hitsBuffer.hasBlock)
	{
		ComponentCollider* collider = FindColliderComponentByShape(hitsBuffer.block.shape);
		ComponentRigidActor* actor = FindRigidActorComponentByActor(hitsBuffer.block.actor);
		GameObject* gameObject = actor->GetParent();

		hitInfo.SetCollider(collider);
		hitInfo.SetActor(actor);
		hitInfo.SetGameObject(gameObject);
		if (hitsBuffer.block.flags & physx::PxHitFlag::ePOSITION)
			hitInfo.SetPoint(math::float3(hitsBuffer.block.position.x, hitsBuffer.block.position.y, hitsBuffer.block.position.z));
		if (hitsBuffer.block.flags & physx::PxHitFlag::eNORMAL)
			hitInfo.SetNormal(math::float3(hitsBuffer.block.normal.x, hitsBuffer.block.normal.y, hitsBuffer.block.normal.z));
		hitInfo.SetDistance(hitsBuffer.block.distance);
		hitInfo.SetFaceIndex(hitsBuffer.block.faceIndex);
	}

	hitsBuffer.finalizeQuery();

	return status;
}

// Overlap: checks whether two geometry objects overlap
/// Transformed box, sphere, capsule or convex geometry
bool ModulePhysics::Overlap(physx::PxGeometry& geometry, physx::PxTransform& transform, std::vector<OverlapHit>& touchesInfo, uint filterMask, SceneQueryFlags sceneQueryFlags) const
{
	assert(transform.isFinite());

	physx::PxQueryFilterData filterData;
	filterData.data.word0 = filterMask; // overlap against this filter mask
	filterData.flags |= physx::PxQueryFlag::eNO_BLOCK;
	if (!(sceneQueryFlags & physx::PxQueryFlag::eSTATIC))
		filterData.flags &= ~physx::PxQueryFlag::eSTATIC;
	if (!(sceneQueryFlags & physx::PxQueryFlag::eDYNAMIC))
		filterData.flags &= ~physx::PxQueryFlag::eDYNAMIC;

	physx::PxOverlapHit hitBuffer[PhysicsConstants::MAX_HITS];
	physx::PxOverlapBuffer hitsBuffer(hitBuffer, PhysicsConstants::MAX_HITS);

	bool status = gScene->overlap(geometry, transform, hitsBuffer, filterData);

	// Touches
	for (physx::PxU32 i = 0; i < hitsBuffer.nbTouches; ++i)
	{
		ComponentCollider* collider = FindColliderComponentByShape(hitsBuffer.touches[i].shape);
		ComponentRigidActor* actor = FindRigidActorComponentByActor(hitsBuffer.touches[i].actor);
		GameObject* gameObject = actor->GetParent();

		OverlapHit touchInfo(gameObject, collider, actor, hitsBuffer.touches[i].faceIndex);
		touchesInfo.push_back(touchInfo);
	}

	return status;
}

bool ModulePhysics::OverlapSphere(float radius, math::float3 center, std::vector<OverlapHit>& touchesInfo, uint filterMask, SceneQueryFlags sceneQueryFlags) const
{
	physx::PxSphereGeometry gSphereGeometry(radius);
	physx::PxTransform gTransform(physx::PxVec3(center.x, center.y, center.z));
	
	return Overlap(gSphereGeometry, gTransform, touchesInfo, filterMask, sceneQueryFlags);
}

// ----------------------------------------------------------------------------------------------------

void ModulePhysics::SetGravity(math::float3& gravity)
{
	assert(gravity.IsFinite());
	this->gravity = gravity;
	gScene->setGravity(physx::PxVec3(gravity.x, gravity.y, gravity.z));
}

math::float3 ModulePhysics::GetGravity() const
{
	return gravity;
}

void ModulePhysics::SetDefaultMaterial(physx::PxMaterial* material)
{
	assert(material != nullptr);
	defaultMaterial = material;
}

physx::PxMaterial* ModulePhysics::GetDefaultMaterial() const
{
	return defaultMaterial;
}

physx::PxTolerancesScale ModulePhysics::GetTolerancesScale() const
{
	return gPhysics->getTolerancesScale();
}