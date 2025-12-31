#include "imgui_wrappers.h"
#include "tools.h"
#include <windows.h>
#include <shellapi.h>

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