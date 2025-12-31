#include "foregroundtracker.h"

bool CGameForegroundTracker::IsGameActive(bool& bSelfActive)
{
    HWND fg = GetForegroundWindow();
    if (!fg)
    {
        bSelfActive = false;
        return false;
    }
        
    bSelfActive = (fg == this->m_ownHwnd);

    DWORD fgPid = 0;
    GetWindowThreadProcessId(fg, &fgPid);

    if (fgPid)
    {
        if (fgPid == this->m_gamePid) return true;
        else if (fgPid != this->m_lastCheckedPid)
        {
            if (this->IsCorrectGameProcess(fgPid))
                this->m_gamePid = fgPid;

            this->m_lastCheckedPid = fgPid;
        }
    }

    return fgPid == this->m_gamePid;
}

void CGameForegroundTracker::SetProcessName(const wchar_t* exeName)
{
    this->m_exeName = exeName;
    this->m_gamePid = 0;
    this->m_lastCheckedPid = 0;
}

void CGameForegroundTracker::SetOwnHwnd(HWND hwnd)
{
    this->m_ownHwnd = hwnd;
}

bool CGameForegroundTracker::IsCorrectGameProcess(DWORD pid)
{
    if (!m_exeName)
        return false;

    HANDLE h = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!h)
        return false;

    wchar_t path[MAX_PATH] = {};
    DWORD size = MAX_PATH;

    bool ok = QueryFullProcessImageNameW(h, 0, path, &size);
    CloseHandle(h);

    if (!ok)
        return false;

    const wchar_t* filename = wcsrchr(path, L'\\');
    filename = filename ? filename + 1 : path;

    return _wcsicmp(filename, m_exeName) == 0;
}