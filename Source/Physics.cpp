// Trinket Game Engine
// (C) 2020 Max Kaufmann <max.kaufmann@gmail.com>

#include "ObjectHandle.h"
#include "Physics.h"
#include "World.h"

#include <PxPhysicsAPI.h>

using namespace physx;

static PxDefaultErrorCallback gDefaultErrorCallback;
static PxDefaultAllocator gDefaultAllocatorCallback;
static PxDefaultCpuDispatcher* gDispatcher = nullptr;

PhysicsRuntime::PhysicsRuntime(World* aWorld) : pWorld(aWorld) {

	pFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gDefaultAllocatorCallback, gDefaultErrorCallback);
	CHECK_ASSERT(pFoundation != nullptr);

	PxTolerancesScale scale;
	pPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *pFoundation, scale);
	CHECK_ASSERT(pPhysics != nullptr);

	#if TRINKET_EDITOR
	pCooking = PxCreateCooking(PX_PHYSICS_VERSION, *pFoundation, PxCookingParams(scale));
	CHECK_ASSERT(pCooking != nullptr);
	#endif

	pDispatcher = PxDefaultCpuDispatcherCreate(2);
	CHECK_ASSERT(pDispatcher != nullptr);

	PxSceneDesc sceneDesc(scale);
	sceneDesc.gravity = PxVec3(0.f, -10.f, 0.f);
	sceneDesc.cpuDispatcher = pDispatcher;
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;
	pDefaultScene = pPhysics->createScene(sceneDesc);
	CHECK_ASSERT(pDefaultScene != nullptr);

	pDefaultMaterial = pPhysics->createMaterial(0.5f, 0.5f, 0.5f);
}

PhysicsRuntime::~PhysicsRuntime() {
	if (pDefaultScene)
		pDefaultScene->release();
	if (pDefaultMaterial)
		pDefaultMaterial->release();
	#if TRINKET_EDITOR
	if (pCooking)
		pCooking->release();
	#endif
	if (pDispatcher)
		pDispatcher->release();
	if (pPhysics)
		pPhysics->release();
	if (pFoundation)
		pFoundation->release();
}

void PhysicsRuntime::TryAddGroundPlane() {
	if (!pGroundPlane) {
		pGroundPlane = PxCreatePlane(*pPhysics, PxPlane(0, 1, 0, 0), *pDefaultMaterial);
		pDefaultScene->addActor(*pGroundPlane);
	}
}

bool PhysicsRuntime::TryAttachRigidbodyTo(ObjectID id) {
	let pScene = pWorld->GetScene();
	CHECK_ASSERT(pScene->IsValid(id));
	if (rigidBodies.Contains(id))
		return false;

	let pHierarchy = pScene->GetSublevelHierarchyFor(id);
	let worldPose = pHierarchy->GetScenePose(id);
	PxRigidDynamic* body = pPhysics->createRigidDynamic(worldPose->ToPhysX());
	body->userData = ObjectHandle(ObjectTag::SCENE_OBJECT, id).p;
	pDefaultScene->addActor(*body);

	
	rigidBodies.TryAppendObject(id, body, worldPose->rpose);
	return true;
}

bool PhysicsRuntime::TryAttachBoxTo(ObjectID id, float extent, float density) {
	let ppRigidbody = rigidBodies.TryGetComponent<C_BODY>(id);
	if (!ppRigidbody)
		return false;

	// TODO: figure out how to do a obj -> many relationship, unlike ObjectPool which 
	//       is 1-1.

	let shape = pPhysics->createShape(PxBoxGeometry(extent, extent, extent), *pDefaultMaterial);
	(*ppRigidbody)->attachShape(*shape);
	PxRigidBodyExt::updateMassAndInertia(**ppRigidbody, density);
	shape->release();
	return true;
}

void PhysicsRuntime::Tick(float dt) {
	let pScene = pWorld->GetScene();

	timeAccum += dt;

	int numFixedTicks = 0;
	while(timeAccum > fixedDeltaTime) {
		timeAccum -= fixedDeltaTime;
		++numFixedTicks;
	}

	let n = rigidBodies.Count();
	let pHandles = rigidBodies.GetComponentData<C_OBJECT_ID>();
	let pBodies = rigidBodies.GetComponentData<C_BODY>();
	let pPrevPoses = rigidBodies.GetComponentData<C_PREV_POSE>();

	if (n == 0)
		return;

	while(numFixedTicks > 1) {
		pDefaultScene->simulate(fixedDeltaTime);
		pDefaultScene->fetchResults(true);
		--numFixedTicks;
	}
	if (numFixedTicks == 1) {

		// save prev poses before last tick
		for(int it=0; it<n; ++it)
			pPrevPoses[it] = RPose(pBodies[it]->getGlobalPose());

		pDefaultScene->simulate(fixedDeltaTime);
		pDefaultScene->fetchResults(true);
	}


	// sample current poses
	let tickProgress = timeAccum / fixedDeltaTime;
	for(int it=0; it<n; ++it) {
		let id = pHandles[it];
		let pHierarchy = pScene->GetSublevelHierarchyFor(id);
		pHierarchy->SetSceneRigidPose(id, RPose::NLerp(
			pPrevPoses[it], 
			RPose(pBodies[it]->getGlobalPose()), 
			tickProgress)
		);
	}

}