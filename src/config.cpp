#include "config.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "tools.h"
#include "render.h"
#include <vector>

CConfig::CConfig()
{
    wchar_t exe_path_buf[MAX_PATH];
    if (GetModuleFileNameW(nullptr, exe_path_buf, MAX_PATH))
        this->configFilePath = std::filesystem::path(exe_path_buf).parent_path();

    if (this->configFilePath.empty())
        this->configFilePath = std::filesystem::current_path();

    this->configFilePath /= "config.json";
}

bool CConfig::Load(bool bFirstLoad)
{
    FILE* file = nullptr;
    errno_t err = _wfopen_s(&file, this->configFilePath.c_str(), L"rb");
    if (err != 0 || !file)
    {
        MessageBox(nullptr, L"Failed to open config file", L"Failed to load config!", MB_OK | MB_ICONERROR);
        return false;
    }

    if (fseek(file, 0, SEEK_END))
    {
        fclose(file);
        return false;
    }

    long size = ftell(file);
    if (size < 0)
    {
        fclose(file);
        return false;
    }
    rewind(file);

    std::vector<char> buffer(size + 1);
    size_t readBytes = fread(buffer.data(), 1, size, file);
    fclose(file);

    if (readBytes != (size_t)size)
    {
        MessageBox(nullptr, L"Failed to read entire file", L"Read file error", MB_OK | MB_ICONERROR);
        return false;
    }

    buffer[size] = '\0';

    rapidjson::Document doc;
    doc.ParseInsitu(buffer.data());
    if (doc.HasParseError() || !doc.IsObject())
    {
        MessageBox(nullptr, L"JSON parse error", L"Config parse error", MB_OK | MB_ICONERROR);
        return false;
    }

    if (doc.HasMember("bCheckActiveWindow") && doc["bCheckActiveWindow"].IsBool()) this->bCheckActiveWindow = doc["bCheckActiveWindow"].GetBool();
    if (doc.HasMember("bCheckUpdates") && doc["bCheckUpdates"].IsBool()) this->bCheckUpdates = doc["bCheckUpdates"].GetBool();
    if (doc.HasMember("bEnableSplitMode") && doc["bEnableSplitMode"].IsBool()) this->bEnableSplitMode = doc["bEnableSplitMode"].GetBool();
    if (doc.HasMember("bEnableSplitObambo") && doc["bEnableSplitObambo"].IsBool()) this->bEnableSplitObambo = doc["bEnableSplitObambo"].GetBool();
    if (doc.HasMember("bEnableSplitHunt") && doc["bEnableSplitHunt"].IsBool()) this->bEnableSplitHunt = doc["bEnableSplitHunt"].GetBool();
    if (doc.HasMember("bEnableSplitCandle") && doc["bEnableSplitCandle"].IsBool()) this->bEnableSplitCandle = doc["bEnableSplitCandle"].GetBool();
    if (doc.HasMember("bScanSystemFonts") && doc["bScanSystemFonts"].IsBool()) this->bScanSystemFonts = doc["bScanSystemFonts"].GetBool();
    if (doc.HasMember("bScanUserFonts") && doc["bScanUserFonts"].IsBool()) this->bScanUserFonts = doc["bScanUserFonts"].GetBool();
    if (doc.HasMember("bScanAppFonts") && doc["bScanAppFonts"].IsBool()) this->bScanAppFonts = doc["bScanAppFonts"].GetBool();
    if (doc.HasMember("bEnableStaminaBar") && doc["bEnableStaminaBar"].IsBool()) this->bEnableStaminaBar = doc["bEnableStaminaBar"].GetBool();
    if (doc.HasMember("bEnableStaminaBarGloss") && doc["bEnableStaminaBarGloss"].IsBool()) this->bEnableStaminaBarGloss = doc["bEnableStaminaBarGloss"].GetBool();
    if (doc.HasMember("bStaminaFlashOnExhausted") && doc["bStaminaFlashOnExhausted"].IsBool()) this->bStaminaFlashOnExhausted = doc["bStaminaFlashOnExhausted"].GetBool();

    if (doc.HasMember("vkSmudgeTimerBind") && doc["vkSmudgeTimerBind"].IsInt()) this->vkSmudgeTimerBind = doc["vkSmudgeTimerBind"].GetInt();
    if (doc.HasMember("vkSwitchSmudgeTimerModeBind") && doc["vkSwitchSmudgeTimerModeBind"].IsInt()) this->vkSwitchSmudgeTimerModeBind = doc["vkSwitchSmudgeTimerModeBind"].GetInt();
    if (doc.HasMember("vkHuntTimerBind") && doc["vkHuntTimerBind"].IsInt()) this->vkHuntTimerBind = doc["vkHuntTimerBind"].GetInt();
    if (doc.HasMember("vkCandleTimerBind") && doc["vkCandleTimerBind"].IsInt()) this->vkCandleTimerBind = doc["vkCandleTimerBind"].GetInt();
    if (doc.HasMember("vkFullResetBind") && doc["vkFullResetBind"].IsInt()) this->vkFullResetBind = doc["vkFullResetBind"].GetInt();
    if (doc.HasMember("vkResetBind") && doc["vkResetBind"].IsInt()) this->vkResetBind = doc["vkResetBind"].GetInt();
    if (doc.HasMember("vkTouchBind") && doc["vkTouchBind"].IsInt()) this->vkTouchBind = doc["vkTouchBind"].GetInt();
    if (doc.HasMember("vkUseBind") && doc["vkUseBind"].IsInt()) this->vkUseBind = doc["vkUseBind"].GetInt();
    if (doc.HasMember("vkSprintBind") && doc["vkSprintBind"].IsInt()) this->vkSprintBind = doc["vkSprintBind"].GetInt();
    if (doc.HasMember("vkForwardBind") && doc["vkForwardBind"].IsInt()) this->vkForwardBind = doc["vkForwardBind"].GetInt();
    if (doc.HasMember("vkBackwardBind") && doc["vkBackwardBind"].IsInt()) this->vkBackwardBind = doc["vkBackwardBind"].GetInt();
    if (doc.HasMember("vkLeftBind") && doc["vkLeftBind"].IsInt()) this->vkLeftBind = doc["vkLeftBind"].GetInt();
    if (doc.HasMember("vkRightBind") && doc["vkRightBind"].IsInt()) this->vkRightBind = doc["vkRightBind"].GetInt();

    if (doc.HasMember("iStartSmudgeTimerAt") && doc["iStartSmudgeTimerAt"].IsInt64()) this->iStartSmudgeTimerAt = doc["iStartSmudgeTimerAt"].GetInt64();
    if (doc.HasMember("iStartHuntTimerAt") && doc["iStartHuntTimerAt"].IsInt64()) this->iStartHuntTimerAt = doc["iStartHuntTimerAt"].GetInt64();
    if (doc.HasMember("iMaxMsSmudge") && doc["iMaxMsSmudge"].IsInt64()) this->iMaxMsSmudge = doc["iMaxMsSmudge"].GetInt64();
    if (doc.HasMember("iMaxMsHunt") && doc["iMaxMsHunt"].IsInt64()) this->iMaxMsHunt = doc["iMaxMsHunt"].GetInt64();

    if (doc.HasMember("flSize") && doc["flSize"].IsFloat()) this->flSize = doc["flSize"].GetFloat();
    if (doc.HasMember("flSmudgeTimerSize") && doc["flSmudgeTimerSize"].IsFloat()) this->flSmudgeTimerSize = doc["flSmudgeTimerSize"].GetFloat();
    if (doc.HasMember("flObamboTimerSize") && doc["flObamboTimerSize"].IsFloat()) this->flObamboTimerSize = doc["flObamboTimerSize"].GetFloat();
    if (doc.HasMember("flHuntTimerSize") && doc["flHuntTimerSize"].IsFloat()) this->flHuntTimerSize = doc["flHuntTimerSize"].GetFloat();
    if (doc.HasMember("flCandleTimerSize") && doc["flCandleTimerSize"].IsFloat()) this->flCandleTimerSize = doc["flCandleTimerSize"].GetFloat();
    if (doc.HasMember("flRounding") && doc["flRounding"].IsFloat()) this->flRounding = doc["flRounding"].GetFloat();
    if (doc.HasMember("flInactiveAlpha") && doc["flInactiveAlpha"].IsFloat()) this->flInactiveAlpha = doc["flInactiveAlpha"].GetFloat();
    if (doc.HasMember("flStaminaBarRounding") && doc["flStaminaBarRounding"].IsFloat()) this->flStaminaBarRounding = doc["flStaminaBarRounding"].GetFloat();
    if (doc.HasMember("flStaminaBarFillRounding") && doc["flStaminaBarFillRounding"].IsFloat()) this->flStaminaBarFillRounding = doc["flStaminaBarFillRounding"].GetFloat();
    if (doc.HasMember("flStaminaBarPadding") && doc["flStaminaBarPadding"].IsFloat()) this->flStaminaBarPadding = doc["flStaminaBarPadding"].GetFloat();

    auto LoadImVec2 = [&](const char* name, ImVec2& value)
        {
            if (doc.HasMember(name) && doc[name].IsArray() && doc[name].Size() == 2)
            {
                if (doc[name][0].IsFloat()) value.x = doc[name][0].GetFloat();
                if (doc[name][1].IsFloat()) value.y = doc[name][1].GetFloat();
            }
        };

    LoadImVec2("imvTimerWindowPos", this->imvTimerWindowPos);
    LoadImVec2("imvSmudgeTimerWindowPos", this->imvSmudgeTimerWindowPos);
    LoadImVec2("imvObamboTimerWindowPos", this->imvObamboTimerWindowPos);
    LoadImVec2("imvHuntTimerWindowPos", this->imvHuntTimerWindowPos);
    LoadImVec2("imvCandleTimerWindowPos", this->imvCandleTimerWindowPos);

    LoadImVec2("imvStaminaBarPos", this->imvStaminaBarPos);
    LoadImVec2("imvStaminaBarSize", this->imvStaminaBarSize);

    auto LoadImVec4 = [&](const char* name, ImVec4& value)
        {
            if (doc.HasMember(name) && doc[name].IsArray() && doc[name].Size() == 4)
            {
                if (doc[name][0].IsFloat()) value.x = doc[name][0].GetFloat();
                if (doc[name][1].IsFloat()) value.y = doc[name][1].GetFloat();
                if (doc[name][2].IsFloat()) value.z = doc[name][2].GetFloat();
                if (doc[name][3].IsFloat()) value.w = doc[name][3].GetFloat();
            }
        };

    LoadImVec4("imvBackgroundColor", this->imvBackgroundColor);
    LoadImVec4("imvBordersColor", this->imvBordersColor);
    LoadImVec4("imvGlowColor1", this->imvGlowColor1);
    LoadImVec4("imvGlowColor2", this->imvGlowColor2);

    LoadImVec4("imvSafeTimeColor1", this->imvSafeTimeColor1);
    LoadImVec4("imvSafeTimeColor2", this->imvSafeTimeColor2);
    LoadImVec4("imvDemonTimeColor1", this->imvDemonTimeColor1);
    LoadImVec4("imvDemonTimeColor2", this->imvDemonTimeColor2);
    LoadImVec4("imvHuntTimeColor1", this->imvHuntTimeColor1);
    LoadImVec4("imvHuntTimeColor2", this->imvHuntTimeColor2);

    LoadImVec4("imvObamboCalmColor1", this->imvObamboCalmColor1);
    LoadImVec4("imvObamboCalmColor2", this->imvObamboCalmColor2);
    LoadImVec4("imvObamboAggressiveColor1", this->imvObamboAggressiveColor1);
    LoadImVec4("imvObamboAggressiveColor2", this->imvObamboAggressiveColor2);

    LoadImVec4("imvHuntTimerColor1", this->imvHuntTimerColor1);
    LoadImVec4("imvHuntTimerColor2", this->imvHuntTimerColor2);

    LoadImVec4("imvCandleTimerColor1", this->imvCandleTimerColor1);
    LoadImVec4("imvCandleTimerColor2", this->imvCandleTimerColor2);

    LoadImVec4("imvStaminaBackgroundColor", this->imvStaminaBackgroundColor);
    LoadImVec4("imvStaminaBordersColor", this->imvStaminaBordersColor);
    LoadImVec4("imvStaminaColorTop", this->imvStaminaColorTop);
    LoadImVec4("imvStaminaColorBottom", this->imvStaminaColorBottom);
    LoadImVec4("imvStaminaColorExhaustedTop", this->imvStaminaColorExhaustedTop);
    LoadImVec4("imvStaminaColorExhaustedBottom", this->imvStaminaColorExhaustedBottom);
    LoadImVec4("imvStaminaFlashExhaustedColor", this->imvStaminaFlashExhaustedColor);

    if (doc.HasMember("strGameProcessName") && doc["strGameProcessName"].IsString())
        this->strGameProcessName = doc["strGameProcessName"].GetString();

    if (doc.HasMember("fontFileName") && doc["fontFileName"].IsString())
        this->fontFileName = doc["fontFileName"].GetString();

    this->bConfigUpdated = !bFirstLoad;
    return true;
}

bool CConfig::Save()
{
    rapidjson::Document doc;
    doc.SetObject();

    rapidjson::Document::AllocatorType& alloc = doc.GetAllocator();

    doc.AddMember("bCheckActiveWindow", this->bCheckActiveWindow, alloc);
    doc.AddMember("bCheckUpdates", this->bCheckUpdates, alloc);
    doc.AddMember("bEnableSplitMode", this->bEnableSplitMode, alloc);
    doc.AddMember("bEnableSplitObambo", this->bEnableSplitObambo, alloc);
    doc.AddMember("bEnableSplitHunt", this->bEnableSplitHunt, alloc);
    doc.AddMember("bEnableSplitCandle", this->bEnableSplitCandle, alloc);
    doc.AddMember("bScanSystemFonts", this->bScanSystemFonts, alloc);
    doc.AddMember("bScanUserFonts", this->bScanUserFonts, alloc);
    doc.AddMember("bScanAppFonts", this->bScanAppFonts, alloc);
    doc.AddMember("bEnableStaminaBar", this->bEnableStaminaBar, alloc);
    doc.AddMember("bEnableStaminaBarGloss", this->bEnableStaminaBarGloss, alloc);
    doc.AddMember("bStaminaFlashOnExhausted", this->bStaminaFlashOnExhausted, alloc);

    doc.AddMember("vkSmudgeTimerBind", this->vkSmudgeTimerBind, alloc);
    doc.AddMember("vkSwitchSmudgeTimerModeBind", this->vkSwitchSmudgeTimerModeBind, alloc);
    doc.AddMember("vkHuntTimerBind", this->vkHuntTimerBind, alloc);
    doc.AddMember("vkCandleTimerBind", this->vkCandleTimerBind, alloc);
    doc.AddMember("vkFullResetBind", this->vkFullResetBind, alloc);
    doc.AddMember("vkResetBind", this->vkResetBind, alloc);
    doc.AddMember("vkTouchBind", this->vkTouchBind, alloc);
    doc.AddMember("vkUseBind", this->vkUseBind, alloc);
    doc.AddMember("vkSprintBind", this->vkSprintBind, alloc);
    doc.AddMember("vkForwardBind", this->vkForwardBind, alloc);
    doc.AddMember("vkBackwardBind", this->vkBackwardBind, alloc);
    doc.AddMember("vkLeftBind", this->vkLeftBind, alloc);
    doc.AddMember("vkRightBind", this->vkRightBind, alloc);

    doc.AddMember("iStartSmudgeTimerAt", this->iStartSmudgeTimerAt, alloc);
    doc.AddMember("iStartHuntTimerAt", this->iStartHuntTimerAt, alloc);
    doc.AddMember("iMaxMsSmudge", this->iMaxMsSmudge, alloc);
    doc.AddMember("iMaxMsHunt", this->iMaxMsHunt, alloc);

    doc.AddMember("flSize", this->flSize, alloc);
    doc.AddMember("flSmudgeTimerSize", this->flSmudgeTimerSize, alloc);
    doc.AddMember("flObamboTimerSize", this->flObamboTimerSize, alloc);
    doc.AddMember("flHuntTimerSize", this->flHuntTimerSize, alloc);
    doc.AddMember("flCandleTimerSize", this->flCandleTimerSize, alloc);
    doc.AddMember("flRounding", this->flRounding, alloc);
    doc.AddMember("flInactiveAlpha", this->flInactiveAlpha, alloc);
    doc.AddMember("flStaminaBarRounding", this->flStaminaBarRounding, alloc);
    doc.AddMember("flStaminaBarFillRounding", this->flStaminaBarFillRounding, alloc);
    doc.AddMember("flStaminaBarPadding", this->flStaminaBarPadding, alloc);

    auto SaveImVec2 = [&](const char* name, const ImVec2& value)
        {
            rapidjson::Value arr(rapidjson::kArrayType);
            arr.PushBack(value.x, alloc);
            arr.PushBack(value.y, alloc);
            doc.AddMember(rapidjson::StringRef(name), arr, alloc);
        };

    SaveImVec2("imvTimerWindowPos", this->imvTimerWindowPos);
    SaveImVec2("imvSmudgeTimerWindowPos", this->imvSmudgeTimerWindowPos);
    SaveImVec2("imvObamboTimerWindowPos", this->imvObamboTimerWindowPos);
    SaveImVec2("imvHuntTimerWindowPos", this->imvHuntTimerWindowPos);
    SaveImVec2("imvCandleTimerWindowPos", this->imvCandleTimerWindowPos);

    SaveImVec2("imvStaminaBarPos", this->imvStaminaBarPos);
    SaveImVec2("imvStaminaBarSize", this->imvStaminaBarSize);

    auto SaveImVec4 = [&](const char* name, const ImVec4& value)
        {
            rapidjson::Value arr(rapidjson::kArrayType);
            arr.PushBack(value.x, alloc);
            arr.PushBack(value.y, alloc);
            arr.PushBack(value.z, alloc);
            arr.PushBack(value.w, alloc);
            doc.AddMember(rapidjson::StringRef(name), arr, alloc);
        };

    SaveImVec4("imvBackgroundColor", this->imvBackgroundColor);
    SaveImVec4("imvBordersColor", this->imvBordersColor);
    SaveImVec4("imvGlowColor1", this->imvGlowColor1);
    SaveImVec4("imvGlowColor2", this->imvGlowColor2);

    SaveImVec4("imvSafeTimeColor1", this->imvSafeTimeColor1);
    SaveImVec4("imvSafeTimeColor2", this->imvSafeTimeColor2);
    SaveImVec4("imvDemonTimeColor1", this->imvDemonTimeColor1);
    SaveImVec4("imvDemonTimeColor2", this->imvDemonTimeColor2);
    SaveImVec4("imvHuntTimeColor1", this->imvHuntTimeColor1);
    SaveImVec4("imvHuntTimeColor2", this->imvHuntTimeColor2);

    SaveImVec4("imvObamboCalmColor1", this->imvObamboCalmColor1);
    SaveImVec4("imvObamboCalmColor2", this->imvObamboCalmColor2);
    SaveImVec4("imvObamboAggressiveColor1", this->imvObamboAggressiveColor1);
    SaveImVec4("imvObamboAggressiveColor2", this->imvObamboAggressiveColor2);

    SaveImVec4("imvHuntTimerColor1", this->imvHuntTimerColor1);
    SaveImVec4("imvHuntTimerColor2", this->imvHuntTimerColor2);

    SaveImVec4("imvCandleTimerColor1", this->imvCandleTimerColor1);
    SaveImVec4("imvCandleTimerColor2", this->imvCandleTimerColor2);

    SaveImVec4("imvStaminaBackgroundColor", this->imvStaminaBackgroundColor);
    SaveImVec4("imvStaminaBordersColor", this->imvStaminaBordersColor);
    SaveImVec4("imvStaminaColorTop", this->imvStaminaColorTop);
    SaveImVec4("imvStaminaColorBottom", this->imvStaminaColorBottom);
    SaveImVec4("imvStaminaColorExhaustedTop", this->imvStaminaColorExhaustedTop);
    SaveImVec4("imvStaminaColorExhaustedBottom", this->imvStaminaColorExhaustedBottom);
    SaveImVec4("imvStaminaFlashExhaustedColor", this->imvStaminaFlashExhaustedColor);

    rapidjson::Value Value;
    Value.SetString(this->strGameProcessName.c_str(), static_cast<rapidjson::SizeType>(this->strGameProcessName.length()), alloc);
    doc.AddMember("strGameProcessName", Value, alloc);

    std::u8string strFontFileName = this->fontFileName.u8string();
    Value.SetString(reinterpret_cast<const char*>(strFontFileName.c_str()), static_cast<rapidjson::SizeType>(strFontFileName.length()), alloc);
    doc.AddMember("fontFileName", Value, alloc);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);

    FILE* file = nullptr; 
    errno_t err = _wfopen_s(&file, this->configFilePath.c_str(), L"wb");
    if (err != 0 || !file)
    {
        MessageBox(nullptr, L"Failed to open file for writing", L"Failed to write config", MB_OK | MB_ICONERROR);
        return false;
    }
    
    size_t written = fwrite(buffer.GetString(), 1, buffer.GetSize(), file);
    fclose(file);

    if (written != buffer.GetSize())
    {
        MessageBox(nullptr, L"Failed to write entire file", L"File write error", MB_OK | MB_ICONERROR);
        return false;
    }

    return true;
}

void CConfig::OnFrameEnd()
{
    this->bConfigUpdated = false;
}

std::vector<const int*> CConfig::GetAllKeybinds() const
{
    return {
        &this->vkSmudgeTimerBind,
        &this->vkSwitchSmudgeTimerModeBind,
        &this->vkHuntTimerBind,
        &this->vkCandleTimerBind,
        &this->vkFullResetBind,
        &this->vkResetBind,
        &this->vkTouchBind,
        &this->vkUseBind,
        &this->vkSprintBind,
        &this->vkForwardBind,
        &this->vkBackwardBind,
        &this->vkLeftBind,
		&this->vkRightBind
    };
}

bool CConfig::IsKeyAlreadyBound(int vk, int* except) const
{
	const auto allKeybinds = this->GetAllKeybinds();

    for (const auto keyPtr : allKeybinds)
    {
        if (keyPtr == except)
            continue;

        if (*keyPtr == vk)
            return true;
    }

    return false;
}