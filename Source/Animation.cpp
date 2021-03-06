// Trinket Game Engine
// (C) 2020 Max Kaufmann <max.kaufmann@gmail.com>

#include "Animation.h"
#include "World.h"

AnimationRuntime::AnimationRuntime(World* aWorld)
	: pWorld(aWorld)
{
	pWorld->GetSkelRegistory()->AddListener(this);
}

AnimationRuntime::~AnimationRuntime() {
	pWorld->GetSkelRegistory()->RemoveListener(this);
}

CharacterRig* AnimationRuntime::CreateCharacterRig(SkelAsset* skel) {
	if (rigs.Contains(skel->ID()))
		return nullptr;

	let result = NewObjectComponent<CharacterRig>(skel);
	rigs.TryAppendObject(result->ID(), result);
	return result;
}

Animator* AnimationRuntime::AttachAnimatorTo(CharacterRig* rig, Skeleton* skeleton) {
	let earlyOut = animators.Contains(skeleton->ID()) || rig->GetSkelAsset() != skeleton->GetSkelAsset();
	if (earlyOut)
		return nullptr;
	
	let result = NewObjectComponent<Animator>(rig, skeleton);
	animators.TryAppendObject(result->ID(), result);
	return result;
}

void AnimationRuntime::Skeleton_WillReleaseSkeleton(class SkelRegistry* Caller, ObjectID id) {
	animators.TryReleaseObject_Swap(id);
}

void AnimationRuntime::Skeleton_WillReleaseSkelAsset(class SkelRegistry* Caller, ObjectID id) {
	rigs.TryReleaseObject_Swap(id);
}
