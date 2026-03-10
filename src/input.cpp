#include "input.h"
#include "render.h"
#include <hidusage.h>

constexpr const std::chrono::milliseconds HOLD_REG_DELAY_MS(400LL);
constexpr const size_t RAWINPUT_BUFFER_CAPACITY = 64ULL * 1024ULL;

void CInput::InputThread()
{
    WNDCLASS wc = {};
    wc.lpfnWndProc = this->InputWndProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = L"InputWnd";
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, wc.lpszClassName, L"InputWindow", 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, wc.hInstance, nullptr);
    if (!hwnd)
    {
        MessageBox(nullptr, L"Failed to create input window", L"Error", MB_OK | MB_ICONERROR);
        UnregisterClass(wc.lpszClassName, wc.hInstance);
        PostMessage(CRender::Get().GetHWND(), WM_CLOSE, 0, 0);
        return;
    }

    RAWINPUTDEVICE rid[2];

    rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
    rid[0].usUsage = HID_USAGE_GENERIC_MOUSE;
    rid[0].dwFlags = RIDEV_INPUTSINK;
    rid[0].hwndTarget = hwnd;

    rid[1].usUsagePage = HID_USAGE_PAGE_GENERIC;
    rid[1].usUsage = HID_USAGE_GENERIC_KEYBOARD;
    rid[1].dwFlags = RIDEV_INPUTSINK;
    rid[1].hwndTarget = hwnd;

    if (!RegisterRawInputDevices(rid, 2, sizeof(RAWINPUTDEVICE)))
    {
        MessageBox(nullptr, L"RegisterRawInputDevices failed", L"Error", MB_OK | MB_ICONERROR);
        DestroyWindow(hwnd);
        UnregisterClass(wc.lpszClassName, wc.hInstance);
		PostMessage(CRender::Get().GetHWND(), WM_CLOSE, 0, 0);
        return;
    }

    auto DestroyAll = [&rid, &wc, &hwnd]()->void {
        rid[0].dwFlags |= RIDEV_REMOVE;
        rid[0].hwndTarget = nullptr;
        rid[1].dwFlags |= RIDEV_REMOVE;
        rid[1].hwndTarget = nullptr;
        RegisterRawInputDevices(rid, 2, sizeof(RAWINPUTDEVICE));
        DestroyWindow(hwnd);
        UnregisterClass(wc.lpszClassName, wc.hInstance);
		};
   
    this->hEvent = CreateEvent(nullptr, TRUE, FALSE, NULL);
	if (!this->hEvent)
    {
        MessageBox(nullptr, L"CreateEvent failed", L"Error", MB_OK | MB_ICONERROR);
        DestroyAll();
        PostMessage(CRender::Get().GetHWND(), WM_CLOSE, 0, 0);
        return;
    }

    MSG msg{};
    std::vector<BYTE> rawBuffer(RAWINPUT_BUFFER_CAPACITY);
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);

    while (WaitForSingleObject(this->hEvent, 0) != WAIT_OBJECT_0)
    {
        auto result = MsgWaitForMultipleObjects(1, &this->hEvent, FALSE, INFINITE, QS_ALLINPUT);

        if (result == (WAIT_OBJECT_0 + 1))
        {
            DWORD queueStatus = GetQueueStatus(QS_ALLINPUT);
            WORD newMessages = HIWORD(queueStatus);

            if (newMessages & QS_RAWINPUT)
                this->ProcessRawInputBuffer(rawBuffer);

            while (PeekMessage(&msg, NULL, 0, WM_INPUT - 1, PM_REMOVE)) {}
            while (PeekMessage(&msg, NULL, WM_INPUT + 1, std::numeric_limits<UINT>::max(), PM_REMOVE)) {}
        }
        else if (result == WAIT_OBJECT_0)
            break;
    }

    ResetEvent(this->hEvent);
    CloseHandle(this->hEvent);
    this->hEvent = nullptr;
    DestroyAll();
}

void CInput::ProcessRawInputBuffer(std::vector<BYTE>& rawBuffer)
{
	auto now = std::chrono::steady_clock::now();
    UINT size = static_cast<UINT>(rawBuffer.size());
    while (true)
    {
        UINT count = GetRawInputBuffer(reinterpret_cast<PRAWINPUT>(rawBuffer.data()), &size, sizeof(RAWINPUTHEADER));

        if (count == static_cast<UINT>(-1))
        {
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
            {
                rawBuffer.resize(size);
                continue;
            }
            break;
        }

        if (!count)
            break;

        BYTE* ptr = rawBuffer.data();
        for (UINT i = 0; i < count; ++i)
        {
            RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(ptr);

            if (raw->header.dwType == RIM_TYPEMOUSE)
            {
                if (raw->data.mouse.usButtonFlags)
                    this->ProcessRawMouseInput(raw->data.mouse, now);
            }
            else if (raw->header.dwType == RIM_TYPEKEYBOARD)
                this->ProcessRawKeyboardInput(raw->data.keyboard, now);

            ptr += raw->header.dwSize;
        }
    }
}

void CInput::ProcessRawMouseInput(const RAWMOUSE& raw, const std::chrono::steady_clock::time_point& now)
{
    USHORT flags = raw.usButtonFlags;
    auto push = [&](int btn, bool pressed) { this->keyQueue.push(KeyEvent(btn, pressed, now)); };

    if (flags & RI_MOUSE_LEFT_BUTTON_DOWN)   push(VK_LBUTTON, true);
    if (flags & RI_MOUSE_LEFT_BUTTON_UP)     push(VK_LBUTTON, false);
    if (flags & RI_MOUSE_RIGHT_BUTTON_DOWN)  push(VK_RBUTTON, true);
    if (flags & RI_MOUSE_RIGHT_BUTTON_UP)    push(VK_RBUTTON, false);
    if (flags & RI_MOUSE_MIDDLE_BUTTON_DOWN) push(VK_MBUTTON, true);
    if (flags & RI_MOUSE_MIDDLE_BUTTON_UP)   push(VK_MBUTTON, false);
    if (flags & RI_MOUSE_BUTTON_4_DOWN)      push(VK_XBUTTON1, true);
    if (flags & RI_MOUSE_BUTTON_4_UP)        push(VK_XBUTTON1, false);
    if (flags & RI_MOUSE_BUTTON_5_DOWN)      push(VK_XBUTTON2, true);
    if (flags & RI_MOUSE_BUTTON_5_UP)        push(VK_XBUTTON2, false);
}

void CInput::ProcessRawKeyboardInput(const RAWKEYBOARD& raw, const std::chrono::steady_clock::time_point& now)
{
    if (raw.VKey)
        this->keyQueue.push(KeyEvent(raw.VKey, !(raw.Flags & RI_KEY_BREAK), now));
}

void CInput::StopInputThread()
{
    SetEvent(this->hEvent);
}

LRESULT CALLBACK CInput::InputWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_DESTROY)
    {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

void CInput::OnFrameStart(bool bActive)
{
    if (!bActive)
    {
        this->keyQueue.clear();
        return;
    }

    KeyEvent e;
    while (this->keyQueue.pop(e))
    {
        auto& keyData = this->buttonsMap[e.vk];
        if (e.pressed && keyData.bIsDown)
            continue;

        auto CalcButtonRegTime = [&keyData]()->void {
            if (std::chrono::duration_cast<std::chrono::milliseconds>(keyData.tLastReleaseTime - keyData.tLastPressTime) >= HOLD_REG_DELAY_MS)
                keyData.tUseRegTime = keyData.tLastPressTime + HOLD_REG_DELAY_MS;
            else
                keyData.tUseRegTime = keyData.tLastReleaseTime;
            };

        if (!keyData.bIsDown && e.pressed)
            keyData.bWasPressedThisFrame = true;

        keyData.bIsDown = e.pressed;
        if (keyData.bIsDown)
        {
            CalcButtonRegTime();
            keyData.tLastPressTime = e.time;
        }
        else
        {
            keyData.tLastReleaseTime = e.time;
            CalcButtonRegTime();
        }
    }
}

void CInput::OnFrameEnd()
{
    for (auto& button : this->buttonsMap)
        button.second.bWasPressedThisFrame = false;
}

const InputKeyData& CInput::GetKeyData(int vk)
{
    return this->buttonsMap[vk];
}

const std::unordered_map<int, InputKeyData>& CInput::GetKeyDataMap() const
{
    return this->buttonsMap;
}

bool CInput::IsMouseButton(int vk) const
{
    return vk > 0 && vk < 0x07 && vk != VK_CANCEL;
}