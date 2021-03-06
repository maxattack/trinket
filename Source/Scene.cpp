// Trinket Game Engine
// (C) 2020 Max Kaufmann <max.kaufmann@gmail.com>

#include "Scene.h"
#include <shared_mutex>

Scene::Scene() 
	: mgr(false)
{
	mgr.ReserveCompact(1024);
	CreateSublevel("Default Level");
}

Scene::~Scene() {
}

ObjectID Scene::CreateObject(Name name) {
	return mgr.CreateObject(name);
}

ObjectID Scene::CreateSublevel(Name name) {
	let id = CreateObject(name);
	let pHierarchy = NewObjectComponent<Hierarchy>(id);
	pHierarchy->AddListener(this);
	sublevels.TryAppendObject(id, pHierarchy);
	return id;
}

void Scene::TryReleaseObject(ObjectID id) {

	if (!mgr.GetPool().Contains(id))
		return;

	// TODO: make this suckless?
	// TODO: multithreaded locks?
	static eastl::vector<ObjectID> destroySet;
	destroySet.clear();
	destroySet.push_back(id);

	if (sublevels.Contains(id)) {
		// removing a whole sublevel
		// TODO
		return;
		//
	}

	let ownerID = GetSublevel(id);
	if (!ownerID.IsNil()) {
		sceneObjects.TryReleaseObject_Swap(id);
		let pHierarchy = GetSublevelHierarchyFor(ownerID);
		for (HierarchyDescendentIterator di(pHierarchy, id); di.MoveNext();)
			destroySet.push_back(di.GetObject());
		pHierarchy->TryRelease(id);
	}

	for(auto it : destroySet) {
		for(auto listener : listeners)
			listener->Scene_WillReleaseObject(this, it);
		mgr.ReleaseObject(id);
	}
}

void Scene::TryRename(ObjectID id, Name name) {
	if (let pName = mgr.TryGetComponent<C_NAME>(id))
		*pName = name;
}

Name Scene::GetName(ObjectID id) const {
	let pName = mgr.GetPool().TryGetComponent<C_NAME>(id);
	return pName ? *pName : Name(ForceInit::Default);
}

ObjectID Scene::FindObject(Name name) const {
	let pNames = mgr.GetPool().GetComponentData<C_NAME>();
	let n = mgr.GetPool().Count();
	for(int it=0; it<n; ++it) {
		if (name == pNames[it])
			return *mgr.GetPool().GetComponentByIndex<C_HANDLE>(it);
	}
	return OBJECT_NIL;
}

void Scene::Hierarchy_DidAddObject(Hierarchy* hierarchy, ObjectID id) {
	sceneObjects.TryAppendObject(id, hierarchy);
}

void Scene::Hierarchy_WillRemoveObject(Hierarchy* hierarchy, ObjectID id) {
	sceneObjects.TryReleaseObject_Swap(id);
}

void Scene::SanityCheck() {
	for (auto it = 0; it < GetSublevelCount(); ++it)
		GetHierarchyByIndex(it)->SanityCheck();
}
