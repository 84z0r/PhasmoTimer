#pragma once
#include "singleton.h"
#include <imgui/imgui.h>
#include <string>

class CTools : public Singleton<CTools>
{
public:
	std::wstring Utf8ToWString(const char* utf8Str);
	std::string WStringToUtf8(const std::wstring& wstr);
	std::wstring ToLower(std::wstring str);
	std::string GeneratePlaceholderString(std::string inputText, char placeholderChar);
	ImVec4 Lerp(const ImVec4& a, const ImVec4& b, float t);
	inline double Saturate(double f) { return (f < 0.0) ? 0.0 : (f > 1.0) ? 1.0 : f; }
	const char* GetKeyNameVK(int vk);
	bool parseULL(const std::string& s, unsigned long long& value);
	bool IsTaskbarAutoHideEnabled();
};