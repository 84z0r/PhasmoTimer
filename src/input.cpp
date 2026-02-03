#include "input.h"

constexpr std::chrono::milliseconds HOLD_REG_DELAY_MS(400LL);

void CInput::InputThread()
{
    WNDCLASS wc = {};
    wc.lpfnWndProc = this->InputWndProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = L"InputHiddenWindow";
    RegisterClass(&wc);

    HWND hWndLocal = CreateWindowEx(0, wc.lpszClassName, L"HiddenInputWindow", 0, 0, 0, 0, 0, nullptr, nullptr, wc.hInstance, nullptr);

    if (!hWndLocal)
    {
        MessageBox(nullptr, L"Failed to create input window", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    this->hwnd.store(hWndLocal, std::memory_order_release);

    RAWINPUTDEVICE rid[2];

    rid[0].usUsagePage = 0x01; // Generic Desktop
    rid[0].usUsage = 0x02; // Mouse
    rid[0].dwFlags = RIDEV_INPUTSINK;
    rid[0].hwndTarget = hWndLocal;

    rid[1].usUsagePage = 0x01; // Generic Desktop
    rid[1].usUsage = 0x06; // Keyboard
    rid[1].dwFlags = RIDEV_INPUTSINK;
    rid[1].hwndTarget = hWndLocal;

    if (!RegisterRawInputDevices(rid, 2, sizeof(RAWINPUTDEVICE)))
        MessageBox(nullptr, L"RegisterRawInputDevices failed", L"Error", MB_OK | MB_ICONERROR);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    this->hwnd.store(nullptr, std::memory_order_release);
}

void CInput::StopInputThread()
{
    HWND h = this->hwnd.load(std::memory_order_acquire);
    if (h)
        PostMessage(h, WM_CLOSE, 0, 0);
}

LRESULT CALLBACK CInput::InputWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_INPUT:
    {
        CInput::Get().ProcessNewMessage(lParam);
        return 0;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

void CInput::ProcessNewMessage(LPARAM lParam)
{
    auto now = std::chrono::steady_clock::now();
    RAWINPUT raw;
    UINT size = sizeof(raw);
    GetRawInputData((HRAWINPUT)lParam, RID_INPUT, &raw, &size, sizeof(RAWINPUTHEADER));

    if (raw.header.dwType == RIM_TYPEMOUSE)
    {
        USHORT flags = raw.data.mouse.usButtonFlags;

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
    else if (raw.header.dwType == RIM_TYPEKEYBOARD)
        this->keyQueue.push(KeyEvent(raw.data.keyboard.VKey, !(raw.data.keyboard.Flags & RI_KEY_BREAK), now));
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

        auto CalcButtonRegTime = [&keyData, &e, this]()->void {
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