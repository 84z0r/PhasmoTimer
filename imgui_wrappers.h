#pragma once
#include <imgui.h>
#include <string>

namespace ImGui
{
    struct SnapWindow
    {
        ImGuiID id;
        ImVec2  pos;
        ImVec2  size;

        SnapWindow(ImGuiID i, ImVec2 p, ImVec2 s) : id(i), pos(p), size(s) {}
    };

    void BeginSnapFrame();
    bool BeginSnap(const char* name, bool* p_open = nullptr, ImGuiWindowFlags flags = 0);
    bool InputText(const char* label, std::string& str, ImGuiInputTextFlags flags = 0);
    bool InputText(const char* label, std::wstring& wstr, ImGuiInputTextFlags flags = 0);
    bool Link(const char* text, const wchar_t* url);
    void LoadDefaultStyle(ImGuiStyle& style);
    void LoadTimerStyle(ImGuiStyle& style);
}