#pragma once
#include "singleton.h"
#include "imgui.h"
#include <string>
#include <array>
#include <cstddef>

class CTools : public Singleton<CTools>
{
public:
	std::wstring Utf8ToWString(const char* utf8Str);
	std::string WStringToUtf8(const std::wstring& wstr);
	ImVec4 Lerp(const ImVec4& a, const ImVec4& b, float t);
	const char* GetKeyNameVK(int vk);
	bool parseULL(const std::string& s, unsigned long long& value);
};