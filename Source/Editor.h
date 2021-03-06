// Trinket Game Engine
// (C) 2020 Max Kaufmann <max.kaufmann@gmail.com>

#pragma once

#include "Common.h"
#if TRINKET_EDITOR

#include <SDL.h>
#include "Display.h"
#include "Object.h"
#include "imgui.h"
#include "DiligentTools/ImGui/interface/ImGuiImplDiligent.hpp"
#include "DiligentTools/ImGui/interface/ImGuiUtils.hpp"
#include "imgui_impl_sdl.h"


class Display;
class World;

class Editor {
private:
	Display* pDisplay;
	World* pWorld;

	ImGuiImplDiligent impl;

	bool showDemoWindow = false;
	ObjectID selection = OBJECT_NIL;

	char renameBuf[1024];

public:

	Editor(Display *aDisplay, World* aWorld);
	~Editor();

	void HandleEvent(const SDL_Event& Event);
	void BeginUpdate();
	void EndUpdate();
	void Draw();

private:
	void ShowOutliner();
	void ShowInspector();

};

#endif