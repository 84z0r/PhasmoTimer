#include "imgui_wrappers.h"
#include "tools.h"
#include <windows.h>
#include <shellapi.h>
#include <vector>

namespace ImGuiSnapWindow
{
    std::vector<ImGui::SnapWindow> g_SnapWindowsPrev;
    std::vector<ImGui::SnapWindow> g_SnapWindowsCurr;
}

static ImVec2 ApplySnapping(ImGuiID self_id, ImVec2 pos, ImVec2 size, float snap_dist = 8.0f)
{
    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImVec2 vp_min = vp->WorkPos;
    ImVec2 vp_max(vp->WorkPos.x + vp->WorkSize.x, vp->WorkPos.y + vp->WorkSize.y);

    if (fabs(pos.x - vp_min.x) < snap_dist) pos.x = vp_min.x;
    if (fabs(pos.y - vp_min.y) < snap_dist) pos.y = vp_min.y;
    if (fabs((pos.x + size.x) - vp_max.x) < snap_dist)
        pos.x = vp_max.x - size.x;
    if (fabs((pos.y + size.y) - vp_max.y) < snap_dist)
        pos.y = vp_max.y - size.y;

    for (const auto& w : ImGuiSnapWindow::g_SnapWindowsPrev)
    {
        if (w.id == self_id)
            continue;

        bool local_snap_x = false;
        bool local_snap_y = false;

        if (fabs(pos.y - (w.pos.y + w.size.y)) < snap_dist)
        {
            pos.y = w.pos.y + w.size.y - 1.f;
            local_snap_y = true;
        }
        else if (fabs((pos.y + size.y) - w.pos.y) < snap_dist)
        {
            pos.y = w.pos.y - size.y + 1.f;
            local_snap_y = true;
        }

        if (fabs(pos.x - (w.pos.x + w.size.x)) < snap_dist)
        {
            pos.x = w.pos.x + w.size.x - 1.f;
            local_snap_x = true;
        }
        else if (fabs((pos.x + size.x) - w.pos.x) < snap_dist)
        {
            pos.x = w.pos.x - size.x + 1.f;
            local_snap_x = true;
        }

        if (local_snap_y)
        {
            if (fabs(pos.x - w.pos.x) < snap_dist)
                pos.x = w.pos.x;
            else if (fabs((pos.x + size.x) - (w.pos.x + w.size.x)) < snap_dist)
                pos.x = w.pos.x + w.size.x - size.x;
            else
            {
                float self_center = pos.x + size.x * 0.5f;
                float other_center = w.pos.x + w.size.x * 0.5f;

                if (fabs(self_center - other_center) < snap_dist)
                    pos.x = other_center - size.x * 0.5f;
            }

            break;
        }

        if (local_snap_x)
        {
            if (fabs(pos.y - w.pos.y) < snap_dist)
                pos.y = w.pos.y;
            else if (fabs((pos.y + size.y) - (w.pos.y + w.size.y)) < snap_dist)
                pos.y = w.pos.y + w.size.y - size.y;
            else
            {
                float self_center = pos.y + size.y * 0.5f;
                float other_center = w.pos.y + w.size.y * 0.5f;

                if (fabs(self_center - other_center) < snap_dist)
                    pos.y = other_center - size.y * 0.5f;
            }

            break;
        }
    }

    return pos;
}

static int InputTextCallback(ImGuiInputTextCallbackData* data)
{
    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
    {
        auto* str = static_cast<std::string*>(data->UserData);
        str->resize(data->BufTextLen);
        data->Buf = str->data();
    }
    return 0;
}

void ImGui::BeginSnapFrame()
{
    ImGuiSnapWindow::g_SnapWindowsPrev = std::move(ImGuiSnapWindow::g_SnapWindowsCurr);
    ImGuiSnapWindow::g_SnapWindowsCurr.clear();
}

bool ImGui::BeginSnap(const char* name, bool* p_open, ImGuiWindowFlags flags)
{
    if (!ImGui::Begin(name, p_open, flags))
        return false;

    ImGuiID id = ImGui::GetID(name);
    ImVec2 pos = ImGui::GetWindowPos();
    ImVec2 size = ImGui::GetWindowSize();

    if (ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows) && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
    {
        ImVec2 snapped = ApplySnapping(id, pos, size);
        if (snapped.x != pos.x || snapped.y != pos.y)
            ImGui::SetWindowPos(snapped, ImGuiCond_Always);
    }

    ImGuiSnapWindow::g_SnapWindowsCurr.emplace_back(id, pos, size);
    return true;
}

bool ImGui::InputText(const char* label, std::string& str, ImGuiInputTextFlags flags)
{
    flags |= ImGuiInputTextFlags_CallbackResize;
    return ImGui::InputText(label, str.data(), str.size() + 1, flags, InputTextCallback, &str);
}

bool ImGui::InputText(const char* label, std::wstring& wstr, ImGuiInputTextFlags flags)
{
    std::string str = CTools::Get().WStringToUtf8(wstr);
    bool ret = ImGui::InputText(label, str, flags);
    if (ret) wstr = CTools::Get().Utf8ToWString(str.c_str());
    return ret;
}

bool ImGui::Link(const char* text, const wchar_t* url)
{
    static const ImVec4 normalColor(0.2f, 0.6f, 1.0f, 1.0f);
    static const ImVec4 hoverColor(0.3f, 0.8f, 1.0f, 1.0f);
    bool ret = false;

    ImGui::PushStyleColor(ImGuiCol_Text, normalColor);
    ImGui::Text("%s", text);
    ImGui::PopStyleColor();

    if (ImGui::IsItemHovered())
    {
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 p = ImGui::GetItemRectMin();
        ImVec2 q = ImGui::GetItemRectMax();
        draw_list->AddRectFilled(p, q, IM_COL32(0, 0, 0, 0), 0.0f);
        draw_list->AddText(p, ImColor(hoverColor), text);
        draw_list->AddLine(ImVec2(p.x, q.y), ImVec2(q.x, q.y), ImColor(hoverColor), 1.0f);

        if (ImGui::IsItemClicked())
        {
            ret = true;
            ShellExecute(NULL, L"open", url, NULL, NULL, SW_SHOWNORMAL);
        }
    }

    return ret;
}