#include "ModulePhysics.h"

#include "Application.h"

#define DLL_NAME_BITS "32"

#ifdef _DEBUG
	#define DLL_CONFIGURATION "debug"
	#pragma comment(lib, "physx/libx86/debugx86/PhysX_32_debug.lib")
	#pragma comment(lib, "physx/libx86/debugx86/PhysXCommon_32_debug.lib")
	#pragma comment(lib, "physx/libx86/debugx86/PhysXFoundation_32_debug.lib")
	#pragma comment(lib, "physx/libx86/debugx86/PhysXExtensions_static_32_debug.lib")
#else
	#define DLL_CONFIGURATION "release"
	#pragma comment(lib, "physx/libx86/releasex86/PhysX_32_release.lib")
	#pragma comment(lib, "physx/libx86/releasex86/PhysXCommon_32_release.lib")
	#pragma comment(lib, "physx/libx86/releasex86/PhysXFoundation_32_release.lib")
	#pragma comment(lib, "physx/libx86/releasex86/PhysXExtensions_static_32_release.lib")
#endif

const char* physxLibraryPath = "PhysX_" DLL_NAME_BITS "_" DLL_CONFIGURATION ".dll";
const char* commonLibraryPath = "PhysXCommon_" DLL_NAME_BITS "_" DLL_CONFIGURATION ".dll";
const char* cookingLibraryPath = "PhysXCookiung_" DLL_NAME_BITS "_" DLL_CONFIGURATION ".dll";
const char* deviceLibraryPath = "PhysXDevice_" DLL_CONFIGURATION ".dll";
const char* foundationLibraryPath = "PhysXFoundation_" DLL_NAME_BITS "_" DLL_CONFIGURATION ".dll";
const char* gpuLibraryPath = "PhysXGpu_" DLL_NAME_BITS "_" DLL_CONFIGURATION ".dll";

HMODULE foundationLibrary = NULL;
HMODULE commonLibrary = NULL;
HMODULE physxLibrary = NULL;

// Typedef the PhysX entry points
typedef PxFoundation*(PxCreateFoundation_FUNC)(PxU32, PxAllocatorCallback&, PxErrorCallback&);
typedef PxPhysics* (PxCreatePhysics_FUNC)(PxU32, PxFoundation&, const PxTolerancesScale& scale, bool, PxPvd*);
typedef void (PxSetPhysXDelayLoadHook_FUNC)(const PxDelayLoadHook* hook);
typedef void (PxSetPhysXCommonDelayLoadHook_FUNC)(const PxDelayLoadHook* hook);
typedef void (PxSetPhysXGpuLoadHook_FUNC)(const PxGpuLoadHook* hook);
typedef int (PxGetSuggestedCudaDeviceOrdinal_FUNC)(PxErrorCallback& errc);
typedef PxCudaContextManager* (PxCreateCudaContextManager_FUNC)(PxFoundation& foundation, const PxCudaContextManagerDesc& desc);

// Set the function pointers to NULL
PxCreateFoundation_FUNC* s_PxCreateFoundation_Func = NULL;
PxCreatePhysics_FUNC* s_PxCreatePhysics_Func = NULL;
PxSetPhysXDelayLoadHook_FUNC* s_PxSetPhysXDelayLoadHook_Func = NULL;
PxSetPhysXCommonDelayLoadHook_FUNC* s_PxSetPhysXCommonDelayLoadHook_Func = NULL;
PxSetPhysXGpuLoadHook_FUNC* s_PxSetPhysXGpuLoadHook_Func = NULL;
PxGetSuggestedCudaDeviceOrdinal_FUNC* s_PxGetSuggestedCudaDeviceOrdinal_Func = NULL;
PxCreateCudaContextManager_FUNC* s_PxCreateCudaContextManager_Func = NULL;

bool LoadPhysicsExplicitely()
{
	// Load the dlls
	foundationLibrary = LoadLibraryA(foundationLibraryPath);
	if (foundationLibrary == NULL)
		return false;

	commonLibrary = LoadLibraryA(commonLibraryPath);
	if (commonLibrary == NULL)
	{
		FreeLibrary(foundationLibrary);
		return false;
	}

	physxLibrary = LoadLibraryA(physxLibraryPath);
	if (physxLibrary == NULL)
	{
		FreeLibrary(foundationLibrary);
		FreeLibrary(commonLibrary);
		return false;
	}

	// Get the function pointers
	s_PxCreateFoundation_Func = (PxCreateFoundation_FUNC*)GetProcAddress(foundationLibrary, "PxCreateFoundation");
	s_PxCreatePhysics_Func = (PxCreatePhysics_FUNC*)GetProcAddress(physxLibrary, "PxCreateBasePhysics");
	s_PxSetPhysXDelayLoadHook_Func = (PxSetPhysXDelayLoadHook_FUNC*)GetProcAddress(physxLibrary, "PxSetPhysXDelayLoadHook");
	s_PxSetPhysXCommonDelayLoadHook_Func = (PxSetPhysXCommonDelayLoadHook_FUNC*)GetProcAddress(commonLibrary, "PxSetPhysXCommonDelayLoadHook");

	s_PxSetPhysXGpuLoadHook_Func = (PxSetPhysXGpuLoadHook_FUNC*)GetProcAddress(physxLibrary, "PxSetPhysXGpuLoadHook");
	s_PxGetSuggestedCudaDeviceOrdinal_Func = (PxGetSuggestedCudaDeviceOrdinal_FUNC*)GetProcAddress(physxLibrary, "PxGetSuggestedCudaDeviceOrdinal");
	s_PxCreateCudaContextManager_Func = (PxCreateCudaContextManager_FUNC*)GetProcAddress(physxLibrary, "PxCreateCudaContextManager");

	// Check if we have all required function pointers
	if (s_PxCreateFoundation_Func == NULL || s_PxCreatePhysics_Func == NULL || s_PxSetPhysXDelayLoadHook_Func == NULL || s_PxSetPhysXCommonDelayLoadHook_Func == NULL)
		return false;

	if (s_PxSetPhysXGpuLoadHook_Func == NULL || s_PxGetSuggestedCudaDeviceOrdinal_Func == NULL || s_PxCreateCudaContextManager_Func == NULL)
		return false;

	return true;
}

// Unload the dlls
void UnloadPhysicsExplicitely()
{
	FreeLibrary(physxLibrary);
	FreeLibrary(commonLibrary);
	FreeLibrary(foundationLibrary);
}

// Overriding the PxDelayLoadHook allows the load of a custom name dll inside PhysX, PhysXCommon and PhysXCooking dlls
struct SnippetDelayLoadHook : public PxDelayLoadHook
{
	virtual const char* getPhysXFoundationDllName() const
	{
		return foundationLibraryPath;
	}

	virtual const char* getPhysXCommonDllName() const
	{
		return commonLibraryPath;
	}
};

// Overriding the PxGpuLoadHook allows the load of a custom GPU name dll
struct SnippetGpuLoadHook : public PxGpuLoadHook
{
	virtual const char* getPhysXGpuDllName() const
	{
		return gpuLibraryPath;
	}
};

ModulePhysics::ModulePhysics(bool start_enabled) : Module(start_enabled)
{
	name = "Physics";
}

ModulePhysics::~ModulePhysics() {}

bool ModulePhysics::Init(JSON_Object* jObject)
{
	return true;
}

PxDefaultAllocator		gAllocator;
PxDefaultErrorCallback	gErrorCallback;

bool ModulePhysics::Start()
{
	// Load the explicitely named dlls
	bool isLoaded = LoadPhysicsExplicitely();
	if (!isLoaded)
		return false;

	// Foundation
	gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
	if (gFoundation == nullptr)
		CONSOLE_LOG("MODULE PHYSICS: PxCreateFoundation failed!");

	// Set PhysX and PhysXCommon delay load hook, this must be done before the create physics is called, before the PhysXFoundation, PhysXCommon delay load happens
	SnippetDelayLoadHook delayLoadHook;
	s_PxSetPhysXDelayLoadHook_Func(&delayLoadHook);
	s_PxSetPhysXCommonDelayLoadHook_Func(&delayLoadHook);

	// Set PhysXGpu load hook
	SnippetGpuLoadHook gpuLoadHook;
	s_PxSetPhysXGpuLoadHook_Func(&gpuLoadHook);

	// Physics
	bool recordMemoryAllocations = true; // whether to perform memory profiling
	PxTolerancesScale scale;
	scale.length = 100; // typical length of an object
	scale.speed = 981; // typical speed of an object
	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, scale, recordMemoryAllocations);
	if (gPhysics == nullptr)
		CONSOLE_LOG("MODULE PHYSICS: PxCreatePhysics failed!");

	return true;
}

update_status ModulePhysics::PreUpdate()
{
	return UPDATE_CONTINUE;
}

update_status ModulePhysics::Update()
{
	return UPDATE_CONTINUE;
}

update_status ModulePhysics::PostUpdate()
{
	return UPDATE_CONTINUE;
}

bool ModulePhysics::CleanUp()
{
	gPhysics->release();
	gFoundation->release();

	UnloadPhysicsExplicitely();

	return true;
}