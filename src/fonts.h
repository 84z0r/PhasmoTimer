#pragma once
#include "singleton.h"
#include <vector>
#include <string>
#include <filesystem>

class CFonts : public Singleton<CFonts>
{
public:
	enum class FontFolderType
	{
		System,
		User,
		App
	};

	bool Init();
	void OnFrameStart();
	//CFonts::OnFrameEnd() should be called before CConfig::Get().OnFrameEnd()
	void OnFrameEnd();
	void ScanFonts();
	bool SetFontToLoad(size_t idx);
	
	inline char GetPlaceholderChar() { return this->placeholderChar; }
	inline float GetFontScaleFactor() { return this->flFontScaleFactor; }
	inline float GetWindowWidthFactor() { return this->flWindowWidthFactor; }
	inline const std::vector<std::filesystem::path>& GetAvailableFonts() { return this->availableFonts; }
	inline size_t GetCustomFontIndex() { return this->iFontIndex; }

private:
	bool LoadFonts();
	void InitializeFontFolders();

	size_t iFontIndex = 0ULL;

	float flFontScaleFactor = 1.3f;
	float flWindowWidthFactor = 3.87f;

	std::vector<std::pair<std::filesystem::path, FontFolderType>> fontFolders;
	std::vector<std::filesystem::path> availableFonts;

	std::filesystem::path fontPathToLoad;

	inline static const std::filesystem::path DEFAULT_FONT_NAME = "Default";

	char placeholderChar = '0';
};