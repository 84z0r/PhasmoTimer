#pragma once
#include <imgui.h>
#include <string>

namespace ImGui
{
    bool InputText(const char* label, std::string& str, ImGuiInputTextFlags flags = 0);
    bool InputText(const char* label, std::wstring& wstr, ImGuiInputTextFlags flags = 0);
    bool Link(const char* text, const wchar_t* url);
}