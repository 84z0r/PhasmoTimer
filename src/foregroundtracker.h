#pragma once
#include <windows.h>
#include <chrono>
#include "singleton.h"

class CGameForegroundTracker : public Singleton<CGameForegroundTracker>
{
    friend class Singleton<CGameForegroundTracker>;

public:
    bool IsGameActive(bool& bSelfActive, const std::chrono::steady_clock::time_point& now);
    void SetOwnHwnd(HWND hwnd);
    void SetProcessName(const std::string& exeName);

private:
    DWORD m_gamePid = 0;
    DWORD m_lastCheckedPid = 0;
    HWND m_lastCheckedHwnd = nullptr;
    HWND m_ownHwnd = nullptr;
    std::wstring m_exeName{};

    std::chrono::steady_clock::time_point t_lastCheck{};
    const std::chrono::milliseconds cache_time{ 100LL };

    bool bLastRet = false;
    bool bLastSelfActive = true;

    bool IsCorrectGameProcess(DWORD pid);
};