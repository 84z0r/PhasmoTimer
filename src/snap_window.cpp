#include "snap_window.h"
#include "config.h"
#include "render.h"
#include "gui.h"
#include "imgui_wrappers.h"

void CSnapWindow::OnFrameStart()
{
    this->g_SnapWindowsPrev = std::move(this->g_SnapWindowsCurr);
    this->g_SnapWindowsCurr.clear();
    this->g_SnapWindowsCurr.reserve(this->g_SnapWindowsPrev.capacity());
    this->HandleSnappingAndDragging();
}

bool CSnapWindow::Begin(const char* name, ImVec2& imvWindowPos, bool* p_open, ImGuiWindowFlags flags)
{
    const ImGuiStyle& style = ImGui::GetStyle();
    float border_size = style.WindowBorderSize;
    auto& config = CConfig::Get();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, config.flRounding);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, config.imvBackgroundColor);
    config.imvBorderColor.w = 1.f;
    ImGui::PushStyleColor(ImGuiCol_Border, config.imvBorderColor);

    ImGui::SetNextWindowBgAlpha(0.0f);
    ImGui::SetNextWindowPos(imvWindowPos, ImGuiCond_Once);

    if (!ImGui::Begin(name, p_open, flags))
    {
        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar(3);
        return false;
    }

    if (config.IsConfigUpdated())
        ImGui::SetWindowPos(imvWindowPos);

    imvWindowPos = ImGui::GetWindowPos();
    ImVec2 size = ImGui::GetWindowSize();

    ImGuiWindow* window = ImGui::GetCurrentWindow();
	ImGuiID id = window->ID;

    ImGuiID group_id = 0;
    auto it_group = this->g_WindowToGroupMap.find(id);
    if (it_group != this->g_WindowToGroupMap.end())
        group_id = it_group->second;

    ImVec2 wmax(imvWindowPos.x + size.x, imvWindowPos.y + size.y);
    ImDrawFlags round_flags = this->ComputeRoundFlags(id, imvWindowPos, wmax);
    ImDrawList* draw = ImGui::GetWindowDrawList();

    draw->AddRectFilled(imvWindowPos, ImVec2(wmax.x - 1.f, wmax.y - 1.f), ImGui::GetColorU32(ImGuiCol_WindowBg), style.WindowRounding, round_flags);
    if (config.bEnableBorders && border_size > 0.0f)
        draw->AddRect(imvWindowPos, ImVec2(wmax.x, wmax.y), ImGui::GetColorU32(ImGuiCol_Border), style.WindowRounding, round_flags, border_size);

    this->g_SnapWindowsCurr.emplace_back(id, imvWindowPos, size, group_id);

    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(3);

    if (ImGui::BeginPopupContextWindow("SnapContextMenu"))
    {
        if (ImGui::MenuItem("Settings")) CGui::Get().bShowSettings = true;
        if (!group_id) { if (ImGui::MenuItem("Group")) this->CreateGroup(id); }
        else { if (ImGui::MenuItem("Ungroup")) this->Ungroup(id); }
        if (ImGui::MenuItem("About")) CGui::Get().bShowAbout = true;
        if (ImGui::MenuItem("Exit")) CRender::Get().bWantExit = true;
        ImGui::EndPopup();
    }

    return true;
}

ImDrawFlags CSnapWindow::ComputeRoundFlags(ImGuiID self_id, const ImVec2& pos, const ImVec2& wmax)
{
    bool tl = true, tr = true, bl = true, br = true;

    ImVec2 tr_pos(wmax.x - 1.f, pos.y);
    ImVec2 bl_pos(pos.x, wmax.y - 1.f);
    ImVec2 br_pos(wmax.x - 1.f, wmax.y - 1.f);

    auto& config = CConfig::Get();
    for (const auto& w : this->g_SnapWindowsPrev)
    {
        if (w.id == self_id)
            continue;

        float wx1 = w.pos.x - config.flSnappingDistance;
        float wy1 = w.pos.y - config.flSnappingDistance;
        float wx2 = w.pos.x + w.size.x - 1.f + config.flSnappingDistance;
        float wy2 = w.pos.y + w.size.y - 1.f + config.flSnappingDistance;

        if (tl && pos.x > wx1 && pos.x < wx2 && pos.y > wy1 && pos.y < wy2) tl = false;
        if (tr && tr_pos.x > wx1 && tr_pos.x < wx2 && tr_pos.y > wy1 && tr_pos.y < wy2) tr = false;
        if (bl && bl_pos.x > wx1 && bl_pos.x < wx2 && bl_pos.y > wy1 && bl_pos.y < wy2) bl = false;
        if (br && br_pos.x > wx1 && br_pos.x < wx2 && br_pos.y > wy1 && br_pos.y < wy2) br = false;

        if (!tl && !tr && !bl && !br)
            break;
    }

    ImDrawFlags flags = ImDrawFlags_RoundCornersNone;
    if (tl) flags |= ImDrawFlags_RoundCornersTopLeft;
    if (tr) flags |= ImDrawFlags_RoundCornersTopRight;
    if (bl) flags |= ImDrawFlags_RoundCornersBottomLeft;
    if (br) flags |= ImDrawFlags_RoundCornersBottomRight;

    return flags;
}

ImVec2 CSnapWindow::ApplySnapping(const SnapWindow* snap_win, ImVec2 pos, const ImVec2& size)
{
	if (!snap_win)
        return pos;

    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImVec2 vp_min = vp->WorkPos;
    ImVec2 vp_max(vp->WorkPos.x + vp->WorkSize.x, vp->WorkPos.y + vp->WorkSize.y);

    std::vector<SnapCandidate> group_members;
    group_members.reserve(this->g_SnapWindowsPrev.capacity());

    if (!snap_win->groupId)
    {
        group_members.emplace_back(pos, size);
    }
    else
    {
        ImVec2 offset(pos.x - snap_win->pos.x, pos.y - snap_win->pos.y);

        for (const auto& w : this->g_SnapWindowsPrev)
        {
            if (w.groupId == snap_win->groupId)
                group_members.emplace_back(ImVec2(w.pos.x + offset.x, w.pos.y + offset.y), w.size);
        }
    }

    ImVec2 correction(0.f, 0.f);
    bool snapped_x = false, snapped_y = false;
    auto& config = CConfig::Get();
    const float flBordersCorrection = config.bEnableBorders ? 0.f : 1.f;

    for (const auto& m : group_members)
    {
        if (!snapped_x)
        {
            if (std::fabs(m.pos.x - vp_min.x) < config.flSnappingDistance)
            {
                correction.x = vp_min.x - m.pos.x;
                snapped_x = true;
            }
            else if (std::fabs((m.pos.x + m.size.x - flBordersCorrection) - vp_max.x) < config.flSnappingDistance)
            {
                correction.x = vp_max.x - (m.pos.x + m.size.x - flBordersCorrection);
                snapped_x = true;
            }
        }

        if (!snapped_y)
        {
            if (std::fabs(m.pos.y - vp_min.y) < config.flSnappingDistance)
            {
                correction.y = vp_min.y - m.pos.y;
                snapped_y = true;
            }
            else if (std::fabs((m.pos.y + m.size.y - flBordersCorrection) - vp_max.y) < config.flSnappingDistance)
            {
                correction.y = vp_max.y - (m.pos.y + m.size.y - flBordersCorrection);
                snapped_y = true;
            }
        }

        if (snapped_x && snapped_y)
            break;
    }

    pos.x += correction.x;
    pos.y += correction.y;

    for (auto& m : group_members)
    {
        m.pos.x += correction.x;
        m.pos.y += correction.y;
    }

    for (const auto& w : this->g_SnapWindowsPrev)
    {
        if (w.id == snap_win->id || (snap_win->groupId && w.groupId == snap_win->groupId))
            continue;

        for (const auto& m : group_members)
        {
            bool overlap_x = (m.pos.x < w.pos.x + w.size.x - 1.f + config.flSnappingDistance) && (m.pos.x + m.size.x - 1.f + config.flSnappingDistance > w.pos.x);
            bool overlap_y = (m.pos.y < w.pos.y + w.size.y - 1.f + config.flSnappingDistance) && (m.pos.y + m.size.y - 1.f + config.flSnappingDistance > w.pos.y);

            ImVec2 local_corr(0.f, 0.f);

            if (!snapped_y && overlap_x)
            {
                if (std::fabs(m.pos.y - (w.pos.y + w.size.y - 1.f)) < config.flSnappingDistance)
                {
                    local_corr.y = (w.pos.y + w.size.y - 1.f) - m.pos.y;
                    snapped_y = true;
                }
                else if (std::fabs((m.pos.y + m.size.y - 1.f) - w.pos.y) < config.flSnappingDistance)
                {
                    local_corr.y = (w.pos.y - m.size.y + 1.f) - m.pos.y;
                    snapped_y = true;
                }

                if (snapped_y)
                {
                    if (!snapped_x)
                    {
                        if (std::fabs(m.pos.x - w.pos.x) < config.flSnappingDistance) local_corr.x = w.pos.x - m.pos.x;
                        else if (std::fabs((m.pos.x + m.size.x - 1.f) - (w.pos.x + w.size.x - 1.f)) < config.flSnappingDistance) local_corr.x = (w.pos.x + w.size.x - m.size.x) - m.pos.x;
                    }

                    pos.x += local_corr.x;
                    pos.y += local_corr.y;

                    if (local_corr.x != 0.f)
                        snapped_x = true;

                    break;
                }
            }

            if (!snapped_x && overlap_y)
            {
                if (std::fabs(m.pos.x - (w.pos.x + w.size.x - 1.f)) < config.flSnappingDistance)
                {
                    local_corr.x = (w.pos.x + w.size.x - 1.f) - m.pos.x;
                    snapped_x = true;
                }
                else if (std::fabs((m.pos.x + m.size.x - 1.f) - w.pos.x) < config.flSnappingDistance)
                {
                    local_corr.x = (w.pos.x - m.size.x + 1.f) - m.pos.x;
                    snapped_x = true;
                }

                if (snapped_x)
                {
                    if (!snapped_y)
                    {
                        if (std::fabs(m.pos.y - w.pos.y) < config.flSnappingDistance) local_corr.y = w.pos.y - m.pos.y;
                        else if (std::fabs((m.pos.y + m.size.y - 1.f) - (w.pos.y + w.size.y - 1.f)) < config.flSnappingDistance) local_corr.y = (w.pos.y + w.size.y - m.size.y) - m.pos.y;
                    }

                    pos.x += local_corr.x;
                    pos.y += local_corr.y;

                    if (local_corr.y != 0.f)
                        snapped_y = true;

                    break;
                }
            }
        }

        if (snapped_x && snapped_y)
            break;
    }

    return pos;
}

void CSnapWindow::CreateGroup(ImGuiID initial_window_id)
{
    if (this->g_WindowToGroupMap.contains(initial_window_id))
        return;

    std::unordered_set<ImGuiID> adjacent_windows;
    adjacent_windows.reserve(this->g_SnapWindowsPrev.capacity());

    FindAdjacentWindowsRecursive(initial_window_id, adjacent_windows, this->g_SnapWindowsPrev);

    if (adjacent_windows.size() <= 1)
        return;

    for (ImGuiID id : adjacent_windows)
        g_WindowToGroupMap[id] = initial_window_id;
}

void CSnapWindow::Ungroup(ImGuiID window_id)
{
    auto it = g_WindowToGroupMap.find(window_id);
    if (it == g_WindowToGroupMap.end())
        return;

    ImGuiID group_id = it->second;

    for (auto it2 = g_WindowToGroupMap.begin(); it2 != g_WindowToGroupMap.end(); )
    {
        if (it2->second == group_id)
            it2 = g_WindowToGroupMap.erase(it2);
        else
            ++it2;
    }
}

void CSnapWindow::FindAdjacentWindowsRecursive(ImGuiID current_id, std::unordered_set<ImGuiID>& visited, const std::vector<SnapWindow>& windows)
{
    visited.insert(current_id);
	const SnapWindow* curr = this->FindSnapWindow(current_id, windows);
    if (!curr)
        return;

    auto& config = CConfig::Get();

    for (const auto& other : windows)
    {
        if (visited.count(other.id))
            continue;

        if (this->g_WindowToGroupMap.contains(other.id))
            continue;

        if (curr->IsAdjacent(other, config.flSnappingDistance))
            this->FindAdjacentWindowsRecursive(other.id, visited, windows);
    }
}

void CSnapWindow::HandleSnappingAndDragging()
{
    ImGuiContext& g = *GImGui;

    if (!g.MovingWindow)
        return;

    ImGuiID id = g.MovingWindow->ID;

    const SnapWindow* snap_win = this->FindSnapWindow(id, this->g_SnapWindowsPrev);
    if (!snap_win)
        return;

    ImGuiID group_id = snap_win->groupId;

    ImVec2 snapped = this->ApplySnapping(snap_win, g.MovingWindow->Pos, g.MovingWindow->Size);

    if (snapped.x != g.MovingWindow->Pos.x || snapped.y != g.MovingWindow->Pos.y)
        ImGui::SetWindowPos(g.MovingWindow, snapped);

    if (!group_id)
        return;

    ImVec2 delta(g.MovingWindow->Pos.x - snap_win->pos.x, g.MovingWindow->Pos.y - snap_win->pos.y);
    if (!delta.x && !delta.y)
        return;

    for (auto& win : this->g_SnapWindowsPrev)
    {
        if (win.groupId != group_id)
            continue;

        if (win.id == id)
            continue;

        ImVec2 new_pos = ImVec2(win.pos.x + delta.x, win.pos.y + delta.y);
		ImGuiWindow* window = ImGui::FindWindowByID(win.id);
		if (window)
            ImGui::SetWindowPos(window, new_pos);
    }
}

SnapWindow* CSnapWindow::FindSnapWindow(ImGuiID id, const std::vector<SnapWindow>& windows) const
{
    for (const auto& w : windows)
    {
        if (w.id == id)
            return const_cast<SnapWindow*>(&w);
    }
	return nullptr;
}

void CSnapWindow::LoadGroups(const rapidjson::Document& doc)
{
    this->g_WindowToGroupMap.clear();

    if (!doc.HasMember("snap_map"))
        return;

    const rapidjson::Value& mapArr = doc["snap_map"];
    if (!mapArr.IsArray())
        return;

    for (const auto& pairVal : mapArr.GetArray())
    {
        if (!pairVal.IsArray() || pairVal.Size() != 2)
            continue;

        const rapidjson::Value& idVal = pairVal[0];
        const rapidjson::Value& gidVal = pairVal[1];

        ImGuiID id = 0U;
        ImGuiID group_id = 0U;

        if (idVal.IsUint())
            id = static_cast<ImGuiID>(idVal.GetUint());
        else
            continue;

        if (gidVal.IsUint())
            group_id = gidVal.GetUint();
        else
            continue;

        this->g_WindowToGroupMap[id] = group_id;
    }
}

void CSnapWindow::SaveGroups(rapidjson::Document& doc, rapidjson::Document::AllocatorType& alloc)
{
    rapidjson::Value mapArr(rapidjson::kArrayType);

    for (const auto& kv : this->g_WindowToGroupMap)
    {
        rapidjson::Value pair(rapidjson::kArrayType);
        pair.PushBack(kv.first, alloc);
        pair.PushBack(kv.second, alloc);
        mapArr.PushBack(pair, alloc);
    }

    doc.AddMember("snap_map", mapArr, alloc);
}

bool SnapWindow::IsAdjacent(const SnapWindow& other, float snap_dist) const
{
    bool overlap_x = (this->pos.x < other.pos.x + other.size.x - 1.f + snap_dist) && (this->pos.x + this->size.x - 1.f + snap_dist > other.pos.x);
    bool overlap_y = (this->pos.y < other.pos.y + other.size.y - 1.f + snap_dist) && (this->pos.y + this->size.y - 1.f + snap_dist > other.pos.y);

    if (!overlap_x && !overlap_y)
        return false;

    bool near_x = std::fabs(this->pos.x - (other.pos.x + other.size.x - 1.f)) < snap_dist || std::fabs((this->pos.x + this->size.x - 1.f) - other.pos.x) < snap_dist;
    bool near_y = std::fabs(this->pos.y - (other.pos.y + other.size.y - 1.f)) < snap_dist || std::fabs((this->pos.y + this->size.y - 1.f) - other.pos.y) < snap_dist;

    return (near_x && overlap_y) || (near_y && overlap_x);
}