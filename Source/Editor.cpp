// Trinket Game Engine
// (C) 2020 Max Kaufmann <max.kaufmann@gmail.com>

#include "Editor.h"

#if TRINKET_EDITOR

#include "Display.h"
#include "World.h"
#include <iostream>


Editor::Editor(Display* aDisplay, World* aWorld) 
	: pDisplay(aDisplay)
	, pWorld(aWorld)
	, impl(aDisplay->GetDevice(), aDisplay->GetSwapChain()->GetDesc().ColorBufferFormat, aDisplay->GetSwapChain()->GetDesc().DepthBufferFormat)
{
	ImGuiIO& io = ImGui::GetIO();
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.IniFilename = nullptr;
	ImGui::StyleColorsDark();
	ImGui_ImplSDL2_InitForD3D(pDisplay->GetWindow());
	renameBuf[0] = 0;
}

Editor::~Editor() {
}

void Editor::HandleEvent(const SDL_Event& Event) {
	ImGui_ImplSDL2_ProcessEvent(&Event);
}

void Editor::BeginUpdate() {
	ImGui_ImplSDL2_NewFrame(pDisplay->GetWindow());
	auto& SCDesc = pDisplay->GetSwapChain()->GetDesc();
	impl.NewFrame(SCDesc.Width, SCDesc.Height, SCDesc.PreTransform);
	if (showDemoWindow)
		ImGui::ShowDemoWindow(&showDemoWindow);
	ShowOutliner();
	ShowInspector();
}

void Editor::EndUpdate() {
	impl.EndFrame();
}

void Editor::ShowOutliner() {
	let pScene = &(pWorld->scene);

	using namespace std;

	let ss = pDisplay->GetScreenSize();

	ImGui::SetNextWindowPos(ImVec2(25, 25), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(float(ss.x/5), ss.y-50.f), ImGuiCond_FirstUseEver);
	ImGuiWindowFlags outlinerFlags(ImGuiWindowFlags_MenuBar);
	ImGui::Begin("Outliner", nullptr, outlinerFlags);

	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("Add")) {
			if (ImGui::MenuItem("Add New Sublevel")) {
				cout << "New Sublevel" << endl;
			}

			if (ImGui::MenuItem("Add Empty Object")) {
				cout << "New Object" << endl;
			}
			if (ImGui::MenuItem("Add Cube Primitive")) {
				cout << "Add Cube" << endl;
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Show")) {
			if (ImGui::MenuItem("Show IMGUI Demo")) {
				showDemoWindow = true;
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	for (int sublevelIdx = 0; sublevelIdx < pScene->GetSublevelCount(); ++sublevelIdx) {
		let pHierarchy = pScene->GetHierarchyByIndex(sublevelIdx);
		let hierarchyName = pScene->GetName(pScene->GetSublevelByIndex(sublevelIdx)).GetString();
		if (ImGui::CollapsingHeader(hierarchyName.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {

			let n = pHierarchy->Count();
			int32 depth = 0;
			auto itemClicked = OBJECT_NIL;
			auto reparentItem = OBJECT_NIL;
			auto reparentTarget = OBJECT_NIL;

			for (int it = 0; it < n;) {
				let id = pHierarchy->GetObjectByIndex(it);
				let hasChildren = pHierarchy->HasChildrenByIndex(it);
				let name = pScene->GetName(id).GetString();
				let selected = id == selection; //outlineSelection.Contains(id);

				let newDepth = pHierarchy->GetDepthByIndex(it);
				while (depth > newDepth) {
					ImGui::TreePop();
					--depth;
				}

				ImGuiTreeNodeFlags nodeFlags(
					ImGuiTreeNodeFlags_OpenOnArrow |
					//ImGuiTreeNodeFlags_OpenOnDoubleClick | 
					ImGuiTreeNodeFlags_SpanAvailWidth |
					ImGuiTreeNodeFlags_DefaultOpen
				);
				if (selected)
					nodeFlags |= ImGuiTreeNodeFlags_Selected;
				if (!hasChildren)
					nodeFlags |= ImGuiTreeNodeFlags_Leaf;

				let open = ImGui::TreeNodeEx(name.c_str(), nodeFlags);

				if (ImGui::IsItemClicked())
					itemClicked = id;

				ImGuiDragDropFlags dragFlags(0);
				if (ImGui::BeginDragDropSource(dragFlags)) {
					ImGui::SetDragDropPayload("OUTLINE_ITEM", &id, sizeof(ObjectID));
					ImGui::EndDragDropSource();
				}

				if (ImGui::BeginDragDropTarget()) {
					ImGuiDragDropFlags dropFlags(0);
					if (let payload = ImGui::AcceptDragDropPayload("OUTLINE_ITEM", dropFlags)) {
						reparentItem = *static_cast<ObjectID*>(payload->Data);
						reparentTarget = id;
					}
					ImGui::EndDragDropTarget();
				}

				if (open) {
					++it;
					++depth;
				} else {
					it = pHierarchy->GetDescendentRangeByIndex(it);
				}
			}
			while (depth > 0) {
				--depth;
				ImGui::TreePop();
			}
			if (!itemClicked.IsNil()) {

				if (pScene->IsValid(selection))
					if (let hierarchy = pScene->GetSublevelHierarchyFor(selection))
						hierarchy->SetRelativeScale(selection, vec3(1.f, 1.f, 1.f));

				selection = itemClicked;
				let name = pScene->GetName(selection).GetString();
				strcpy_s(renameBuf, 1024, name.c_str());

				if (pScene->IsValid(selection))
					if (let hierarchy = pScene->GetSublevelHierarchyFor(selection))
						hierarchy->SetRelativeScale(selection, vec3(1.2f, 1.2f, 1.2f));


			}
			if (!reparentItem.IsNil()) {
				pHierarchy->TryReparent(reparentItem, reparentTarget);
			}
		}
	}

	ImGui::End();
}


void Editor::ShowInspector() {
	let pScene = pWorld->GetScene();

	let ss = pDisplay->GetScreenSize();
	let w = float(ss.x / 5);

	ImGui::SetNextWindowPos(ImVec2(ss.x - 25.f - w, 25.f), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(w, ss.y - 50.f), ImGuiCond_FirstUseEver);

	auto toDelete = OBJECT_NIL;

	ImGui::Begin("Inspector");

	if (pScene->IsValid(selection)) {
		ImGui::Text("Object #%d", selection);
		ImGui::InputText("", renameBuf, 1024);
		ImGui::SameLine();
		if (ImGui::Button("Rename") && renameBuf[0]) {
			pScene->TryRename(selection, Name(renameBuf));
		}
		if (ImGui::Button("Delete?"))
			toDelete = selection;

		if (let pHierarchy = pScene->GetSublevelHierarchyFor(selection)) {
			let rel = pHierarchy->GetScenePose(selection);
			ImGui::LabelText("Position", "<%.2f, %.2f, %.2f>", rel->position.x, rel->position.y, rel->position.z);
		}

	} else {
		ImGui::Text("No Object Selected.");
	}


	ImGui::End();

	if (!toDelete.IsNil())
		pScene->TryReleaseObject(toDelete);
}

void Editor::Draw() {
	impl.Render(pDisplay->GetContext());
}


#endif 
