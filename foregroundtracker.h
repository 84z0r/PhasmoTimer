#pragma once
#include <windows.h>
#include <chrono>
#include "singleton.h"

class CGameForegroundTracker : public Singleton<CGameForegroundTracker>
{
    friend class Singleton<CGameForegroundTracker>;

public:
    bool IsGameActive(bool& bSelfActive);
    void SetOwnHwnd(HWND hwnd);
    void SetProcessName(const wchar_t* exeName);

private:
    CGameForegroundTracker() : m_gamePid(0), m_lastCheckedPid(0), m_ownHwnd(nullptr), m_exeName(nullptr) {}

    DWORD m_gamePid;
    DWORD m_lastCheckedPid;
    HWND m_ownHwnd;
    const wchar_t* m_exeName;

    bool IsCorrectGameProcess(DWORD pid);
};