#include "fonts.h"
#include "font_comicz.h"
#include "config.h"
#include "tools.h"
#include <shlobj.h>

bool CFonts::Init()
{
    this->InitializeFontFolders();
    this->ScanFonts();

    if (!this->SetFontToLoad(this->iFontIndex))
    {
        if (!this->SetFontToLoad(0ULL))
            return false;
    }

	if (!this->LoadFonts())
        return false;

	this->fontPathToLoad.clear();
	return true;
}

void CFonts::OnFrameStart()
{
    if (this->fontPathToLoad.empty())
		return;

    this->LoadFonts();
    this->fontPathToLoad.clear();
}

void CFonts::OnFrameEnd()
{
    if (!CConfig::Get().IsConfigUpdated())
        return;

    this->ScanFonts();
	this->SetFontToLoad(this->iFontIndex);
}

bool CFonts::LoadFonts()
{
    bool load_from_file = !this->fontPathToLoad.empty() && this->fontPathToLoad != this->DEFAULT_FONT_NAME;

    ImGuiIO& io = ImGui::GetIO();
    float base_size = 96.0f;
    static const ImWchar used_ranges[] = { '.', '.', '0', ':', 0};

    auto load_font_or_fallback = [&](ImFontAtlas* atlas, const ImFontConfig* config) -> ImFont* {
        ImFont* font = nullptr;
        if (load_from_file)
        {
            const std::u8string u8_path_string = this->fontPathToLoad.u8string();
            font = atlas->AddFontFromFileTTF(reinterpret_cast<const char*>(u8_path_string.c_str()), base_size, config);
        }

        if (!font)
        {
            if (load_from_file)
            {
                load_from_file = false;
				this->iFontIndex = 0ULL;
                CConfig::Get().fontFileName = this->DEFAULT_FONT_NAME;
                this->ScanFonts();
            }
            font = atlas->AddFontFromMemoryCompressedTTF(Font::FONT_COMICZ_BIN, sizeof(Font::FONT_COMICZ_BIN), base_size, config);
        }
        return font;
        };

    ImFontAtlas temp_atlas;
    ImFontConfig temp_font_config;
	temp_font_config.GlyphRanges = used_ranges;
    ImFont* temp_font = load_font_or_fallback(&temp_atlas, &temp_font_config);
    if (!temp_font)
        return false;

    temp_atlas.Build();
    ImFontBaked* baked = temp_font->GetFontBaked(base_size);
    if (!baked)
        return false;

    float max_digit_height = 0.0f;
    float max_digit_width = 0.0f;
    for (char c = '0'; c <= '9'; ++c)
    {
        char digit_str[2] = { c, '\0' };
        ImVec2 digit_size = temp_font->CalcTextSizeA(temp_font->LegacySize, FLT_MAX, 0.0f, digit_str, digit_str + 1);
        if (digit_size.x > max_digit_width)
        {
            max_digit_width = digit_size.x;
            this->placeholderChar = c;
        }
        ImWchar wc = (ImWchar)c;
        int glyph_index = (wc < baked->IndexLookup.Size) ? baked->IndexLookup[wc] : -1;
        if (glyph_index != -1)
        {
            const ImFontGlyph& glyph = baked->Glyphs[glyph_index];
            float visual_height = glyph.Y1 - glyph.Y0;
            if (visual_height > max_digit_height) max_digit_height = visual_height;
        }
    }

    this->flFontScaleFactor = (max_digit_height > 0.0f) ? ((base_size * 0.76f) / max_digit_height) : 1.0f;

    char buf[16] = {};
    char p = this->placeholderChar;
    snprintf(buf, sizeof(buf), "%c:%c%c.%c%c", p, p, p, p, p);
    float scaled_font_size = base_size * this->flFontScaleFactor;
    ImVec2 text_size = temp_font->CalcTextSizeA(scaled_font_size, FLT_MAX, 0.0f, buf);
    float padding = scaled_font_size * 0.16f;
    this->flWindowWidthFactor = (text_size.x + padding) / base_size;

    io.Fonts->Clear();
    io.Fonts->AddFontFromMemoryCompressedTTF(Font::FONT_COMICZ_BIN, sizeof(Font::FONT_COMICZ_BIN), 16.0f);

    static const ImWchar digit_ranges[] = { '0', '9', 0 };
    ImFontConfig fontConfigNODigits;
    snprintf(fontConfigNODigits.Name, sizeof(fontConfigNODigits.Name), "TimerFont_NODigits");
    fontConfigNODigits.GlyphExcludeRanges = digit_ranges;
    load_font_or_fallback(io.Fonts, &fontConfigNODigits);

    ImFontConfig fontConfigDigits;
    snprintf(fontConfigDigits.Name, sizeof(fontConfigDigits.Name), "TimerFont_DigitsOnly");
    fontConfigDigits.GlyphRanges = digit_ranges;
    fontConfigDigits.GlyphMinAdvanceX = max_digit_width;
    fontConfigDigits.GlyphMaxAdvanceX = max_digit_width;
    fontConfigDigits.MergeMode = true;
    load_font_or_fallback(io.Fonts, &fontConfigDigits);

    io.Fonts->Build();

    return true;
}

void CFonts::ScanFonts()
{
    this->availableFonts.clear();
    this->availableFonts.push_back(this->DEFAULT_FONT_NAME);
    this->iFontIndex = 0ULL;

    const auto& cfg = CConfig::Get();
    const std::filesystem::path& config_font_filename = cfg.fontFileName;

    for (const auto& [folder_path, folder_type] : this->fontFolders)
    {
        if (folder_type == FontFolderType::System && !cfg.bScanSystemFonts)
            continue;

        if (folder_type == FontFolderType::User && !cfg.bScanUserFonts)
            continue;

        if (folder_type == FontFolderType::App && !cfg.bScanAppFonts)
            continue;

        if (!std::filesystem::exists(folder_path) || !std::filesystem::is_directory(folder_path))
            continue;

        for (const auto& entry : std::filesystem::directory_iterator(folder_path))
        {
            if (!entry.is_regular_file())
                continue;

            std::filesystem::path entry_path = entry.path();
            std::filesystem::path extension = entry_path.extension();

            if (extension == ".ttf" || extension == ".otf")
                this->availableFonts.emplace_back(entry_path);
        }
    }

    if (this->availableFonts.size() > 1)
    {
        std::sort(this->availableFonts.begin() + 1, this->availableFonts.end(), [](const std::filesystem::path& a, const std::filesystem::path& b) {
                return CTools::Get().ToLower(a.filename().wstring()) < CTools::Get().ToLower(b.filename().wstring());
            }
        );
    }

    auto it = std::find_if(this->availableFonts.begin(), this->availableFonts.end(), [&config_font_filename](const std::filesystem::path& p) {
            return CTools::Get().ToLower(p.filename().wstring()) == CTools::Get().ToLower(config_font_filename.wstring());
        }
    );

    if (it != this->availableFonts.end())
        this->iFontIndex = std::distance(this->availableFonts.begin(), it);
    else
        this->SetFontToLoad(this->iFontIndex);
}

bool CFonts::SetFontToLoad(size_t idx)
{
    if (idx >= this->availableFonts.size())
        return false;

    const std::filesystem::path& path_to_set = this->availableFonts[idx];
    this->fontPathToLoad = path_to_set;
    CConfig::Get().fontFileName = path_to_set.filename();
    this->iFontIndex = idx;

    return true;
}

void CFonts::InitializeFontFolders()
{
    this->fontFolders.clear();
    PWSTR path_ptr = nullptr;

    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Fonts, 0, NULL, &path_ptr)))
    {
        this->fontFolders.emplace_back(path_ptr, FontFolderType::System);
        CoTaskMemFree(path_ptr);
        path_ptr = nullptr;
    }

    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &path_ptr)))
    {
        this->fontFolders.emplace_back(std::filesystem::path(path_ptr) / L"Microsoft\\Windows\\Fonts", FontFolderType::User);
        CoTaskMemFree(path_ptr);
        path_ptr = nullptr;
    }

    wchar_t exe_path_buf[MAX_PATH];
    if (GetModuleFileNameW(nullptr, exe_path_buf, MAX_PATH))
    {
        std::filesystem::path app_path = exe_path_buf;
        this->fontFolders.emplace_back(app_path.parent_path() / "fonts", FontFolderType::App);
    }
}