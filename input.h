#pragma once
#include <chrono>
#include <unordered_map>
#include "singleton.h"
#include "lockfreequeue.hpp"

struct KeyEvent
{
    int vk;
    bool pressed;
    std::chrono::steady_clock::time_point time;

    KeyEvent() : vk(0), pressed(false), time{} {}
    KeyEvent(int vk_, bool pressed_, std::chrono::steady_clock::time_point time_) : vk(vk_), pressed(pressed_), time(time_) {}
};

struct InputKeyData
{
    bool bIsDown;
    bool bWasPressedThisFrame;
    std::chrono::steady_clock::time_point tLastPressTime;
    std::chrono::steady_clock::time_point tLastReleaseTime;
    std::chrono::steady_clock::time_point tUseRegTime;

    InputKeyData() : bIsDown(false), bWasPressedThisFrame(false), tLastPressTime{}, tLastReleaseTime {} {}
};

class CInput : public Singleton<CInput>
{
public:
    //Input Thread
    void ProcessNewMessage(LPARAM lParam);
    void InputThread();
    void StopInputThread();
    //RenderThread
    void OnFrameStart(bool bActive);
    void OnFrameEnd();
    const InputKeyData& GetKeyData(int vk);
    const std::unordered_map<int, InputKeyData>& GetKeyDataMap();

private:
    static LRESULT CALLBACK InputWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    bool IsMouseButton(int vk);
    std::atomic<HWND> hwnd{ nullptr };
    LockFreeQueue<KeyEvent, 2048> keyQueue;
    std::unordered_map<int, InputKeyData> buttonsMap;
};