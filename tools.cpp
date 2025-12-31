#include "tools.h"
#include <windows.h>
#include <charconv>

std::wstring CTools::Utf8ToWString(const char* utf8Str)
{
    if (!utf8Str) return L"";

    int size_needed = MultiByteToWideChar(CP_UTF8, 0, utf8Str, -1, nullptr, 0);
    std::wstring wstr(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8Str, -1, &wstr[0], size_needed);

    if (!wstr.empty() && wstr.back() == L'\0')
        wstr.pop_back();

    return wstr;
}

std::string CTools::WStringToUtf8(const std::wstring& wstr)
{
    if (wstr.empty())
        return std::string();

    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
    std::string str(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &str[0], size_needed, nullptr, nullptr);

    return str;
}

ImVec4 CTools::Lerp(const ImVec4& a, const ImVec4& b, float t)
{ 
    return ImVec4(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t, a.w + (b.w - a.w) * t);
}

const char* CTools::GetKeyNameVK(int vk)
{
    static char keyName[128];

    UINT scanCode = MapVirtualKey(vk, MAPVK_VK_TO_VSC);

    switch (vk)
    {
    case VK_LBUTTON: return "Mouse Left";
    case VK_RBUTTON: return "Mouse Right";
    case VK_MBUTTON: return "Mouse Mid";
    case VK_XBUTTON1: return "Mouse 4";
    case VK_XBUTTON2: return "Mouse 5";
    case VK_LEFT:
    case VK_UP:
    case VK_RIGHT:
    case VK_DOWN:
    case VK_PRIOR:   // Page Up
    case VK_NEXT:    // Page Down
    case VK_END:
    case VK_HOME:
    case VK_INSERT:
    case VK_DELETE:
    case VK_DIVIDE:
    case VK_NUMLOCK:
        scanCode |= 0x100;
        break;
    }

    if (GetKeyNameTextA(scanCode << 16, keyName, sizeof(keyName)))
        return keyName;

    return "Unknown";
}

bool CTools::parseULL(const std::string& s, unsigned long long& value)
{
    auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), value);
    if (ec != std::errc())
        return false;

    while (ptr != s.data() + s.size() && std::isspace((unsigned char)*ptr))
        ++ptr;

    return ptr == s.data() + s.size();
}