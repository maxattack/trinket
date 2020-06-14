// Trinket Game Engine
// (C) 2020 Max Kaufmann <max.kaufmann@gmail.com>

#pragma once
#include "Assets.h"
#include "World.h"
#include "Input.h"
#include "Graphics.h"
#include "Physics.h"

class ScriptVM {
private:
	AssetDatabase* pAssets;
	World* pWorld;
	Input* pInput;
	Graphics *pGraphics;
	Physics *pPhysics;
	struct lua_State* lua;

public:

	ScriptVM(Input* aInput, Graphics* aGraphics, Physics* aPhysics);
	~ScriptVM();

	AssetDatabase* GetAssets() const { return pAssets; }
	World* GetWorld() const { return pWorld; }
	Input* GetInput() const { return pInput; }
	Graphics* GetGraphics() const { return pGraphics; }
	Physics* GetPhysics() const { return pPhysics; }


	void RunScript(const char* path);
	void Tick();

#if TRINKET_TEST
	vec3 wireframePosition = vec3(0, 0, 0);
	vec4 wireframeColor = vec4(1, 1, 1, 1);
#endif

};
