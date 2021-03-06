// Trinket Game Engine
// (C) 2020 Max Kaufmann <max.kaufmann@gmail.com>

#pragma once
#include "Math.h"
#include "Mesh.h"

class World; 

class ScriptVM {
private:
	World* pWorld;
	struct lua_State* lua;
	MeshPlotter plotter;

public:

	ScriptVM(World* pWorld);
	~ScriptVM();

	World* GetWorld() { return pWorld; }
	MeshPlotter* GetPlotter() { return &plotter; }

	void RunScript(const char* path);
	void Update();

#if TRINKET_TEST
	vec3 wireframePosition = vec3(0, 0, 0);
	vec4 wireframeColor = vec4(1, 1, 1, 1);
	quat wireframeRotation = quat(1,0,0,0);
#endif

};
