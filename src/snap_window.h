#pragma once
#include "singleton.h"
#include "rapidjson/document.h"
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <vector>
#include <unordered_map>
#include <unordered_set>

struct SnapCandidate
{
    ImVec2 pos;
    ImVec2 size;

	SnapCandidate(const ImVec2& p, const ImVec2& s) : pos(p), size(s) {}
};

class SnapWindow
{
public:
    ImGuiID id;
    ImVec2 pos;
    ImVec2 size;
    ImGuiID groupId = 0;

    SnapWindow(ImGuiID i, const ImVec2& p, const ImVec2& s, ImGuiID g) : id(i), pos(p), size(s), groupId(g) {}

    bool IsAdjacent(const SnapWindow& other, float snap_dist) const;
};

class CSnapWindow : public Singleton<CSnapWindow>
{
public:
    void OnFrameStart();
    bool Begin(const char* name, ImVec2& imvWindowPos, bool* p_open = nullptr, ImGuiWindowFlags flags = 0);
    void LoadGroups(const rapidjson::Document& doc);
	void SaveGroups(rapidjson::Document& doc, rapidjson::Document::AllocatorType& alloc);

private:
    ImDrawFlags ComputeRoundFlags(ImGuiID self_id, const ImVec2& pos, const ImVec2& wmax);
    ImVec2 ApplySnapping(const SnapWindow* snap_win, ImVec2 pos, const ImVec2& size);

    void CreateGroup(ImGuiID initial_window_id);
    void Ungroup(ImGuiID window_id);
    void FindAdjacentWindowsRecursive(ImGuiID current_id, std::unordered_set<ImGuiID>& visited, const std::vector<SnapWindow>& windows);
	void HandleSnappingAndDragging();

	SnapWindow* FindSnapWindow(ImGuiID id, const std::vector<SnapWindow>& windows) const;

    std::vector<SnapWindow> g_SnapWindowsPrev;
    std::vector<SnapWindow> g_SnapWindowsCurr;

    std::unordered_map<ImGuiID, ImGuiID> g_WindowToGroupMap;
};