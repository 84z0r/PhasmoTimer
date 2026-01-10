#include "render.h"
#include "config.h"
#include "foregroundtracker.h"
#include "tools.h"
#include "imgui_wrappers.h"
#include "font_comicz.h"
#include "logo.h"
#include "resource.h"
#include "gui.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.hpp"

constexpr const UINT WM_APPBAR = WM_USER + 1;

void CRender::RenderLoop()
{
    while (!this->bWantExit)
    {
        if (this->StartNewFrame())
        {
            CGui::Get().OnRender();
            this->EndNewFrame();
        }
    }
}

bool CRender::StartNewFrame()
{
    MSG msg;
    while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
    {
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
        if (msg.message == WM_QUIT || msg.message == WM_CLOSE)
        {
            this->bWantExit = true;
            return false;
        }
    }

    if (this->bDpiChanged)
    {
        this->bDpiChanged = false;
        this->OnDpiChanged();
    }

    if (this->bWindowMovedOrResized)
    {
        this->bWindowMovedOrResized = false;
        this->OnWindowMovedOrResized();
    }

    this->now_time = std::chrono::steady_clock::now();
    this->bGameWindowActive = CConfig::Get().bCheckActiveWindow ? CGameForegroundTracker::Get().IsGameActive(this->bSelfWindowActive, this->now_time) : true;

    CInput::Get().OnFrameStart(this->bGameWindowActive || this->bSelfWindowActive);
    CGui::Get().OnFrameStart();

    // Handle window being minimized or screen locked
    if (g_SwapChainOccluded && g_pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        return false;
    }
    g_SwapChainOccluded = false;

    // Start the Dear ImGui frame
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    ImGui::BeginSnapFrame();

    return true;
}

void CRender::EndNewFrame()
{
    static const float clear_color_with_alpha[4] = { 0.f, 0.f, 0.f, 0.f };

    this->UpdateClickThroughState();
    CInput::Get().OnFrameEnd();
    CConfig::Get().OnFrameEnd();

    ImGui::Render();
    g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
    g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    HRESULT hr = g_pSwapChain->Present(1, 0);   // Present with vsync
    //HRESULT hr = g_pSwapChain->Present(0, 0); // Present without vsync
    g_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);
    g_dcompDevice->Commit();
}

bool CRender::Init(HINSTANCE hInstance)
{
    this->wc = { sizeof(this->wc), CS_CLASSDC, this->WndProc, 0L, 0L, hInstance, LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1)), nullptr, nullptr, nullptr, L"PT", nullptr };
    ::RegisterClassExW(&this->wc);

    ImGui_ImplWin32_EnableDpiAwareness();

    HMONITOR primaryMonitor = MonitorFromPoint({ 0, 0 }, MONITOR_DEFAULTTOPRIMARY);

    MONITORINFO mi{};
    mi.cbSize = sizeof(mi);
    GetMonitorInfo(primaryMonitor, &mi);

    int width = mi.rcMonitor.right - mi.rcMonitor.left;
    int height = mi.rcMonitor.bottom - mi.rcMonitor.top;

    if (CTools::Get().IsTaskbarAutoHideEnabled())
        height -= 1;

    this->hwnd = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_NOREDIRECTIONBITMAP, this->wc.lpszClassName, L"PhasmoTimer", WS_POPUP, mi.rcMonitor.left, mi.rcMonitor.top, width, height, nullptr, nullptr, this->wc.hInstance, nullptr);

    if (!this->hwnd)
        return false;

    this->g_CurrentMonitor = MonitorFromWindow(this->hwnd, MONITOR_DEFAULTTONEAREST);

    abd.cbSize = sizeof(abd);
    abd.hWnd = this->hwnd;
    abd.uCallbackMessage = WM_APPBAR;
    SHAppBarMessage(ABM_NEW, &abd);

    if (!CreateDeviceD3D(width, height))
    {
        CleanupDeviceD3D();
        ::DestroyWindow(this->hwnd);
        ::UnregisterClassW(this->wc.lpszClassName, this->wc.hInstance);
        return false;
    }

    if (FAILED(DCompositionCreateDevice(nullptr, IID_PPV_ARGS(&g_dcompDevice))))
        return false;

    g_dcompDevice->CreateTargetForHwnd(this->hwnd, TRUE, &g_dcompTarget);
    g_dcompDevice->CreateVisual(&g_dcompVisual);
    g_dcompVisual->SetContent(g_pSwapChain);
    g_dcompTarget->SetRoot(g_dcompVisual);
    g_dcompDevice->Commit();

    ::ShowWindow(this->hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(this->hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = nullptr;

    float dpiScale = ImGui_ImplWin32_GetDpiScaleForHwnd(this->hwnd);

    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(dpiScale);
    style.FontScaleDpi = dpiScale;
    ImGui::LoadTimerStyle(style);

    ImGui_ImplWin32_Init(this->hwnd);
    ImGui_ImplDX11_Init(this->g_pd3dDevice, this->g_pd3dDeviceContext);

    io.Fonts->AddFontFromMemoryCompressedTTF(Font::FONT_COMICZ_BIN, sizeof(Font::FONT_COMICZ_BIN), 16.0f);
    io.Fonts->AddFontFromMemoryCompressedTTF(Font::FONT_COMICZ_BIN, sizeof(Font::FONT_COMICZ_BIN), 96.0f);
    io.Fonts->Build();

    if (!this->LoadTextureFromMemory(Logo::LOGO_BIN, sizeof(Logo::LOGO_BIN), &this->logo_texture, &this->logo_width, &this->logo_height))
        MessageBoxW(nullptr, L"Failed to load logo texture", L"Error", MB_OK | MB_ICONERROR);

    CGameForegroundTracker::Get().SetOwnHwnd(this->hwnd);
    CGameForegroundTracker::Get().SetProcessName(CConfig::Get().strGameProcessName);

    return true;
}

void CRender::Cleanup()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    this->CleanupDComp();
    this->CleanupDeviceD3D();

    SHAppBarMessage(ABM_REMOVE, &this->abd);

    ::DestroyWindow(this->hwnd);
    ::UnregisterClassW(this->wc.lpszClassName, this->wc.hInstance);
}

bool CRender::CreateDeviceD3D(int width, int height)
{
    D3D_FEATURE_LEVEL featureLevel;
    if (FAILED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext)))
        return false;

    return CreateSwapChain(width, height);
}

bool CRender::CreateSwapChain(int width, int height)
{
    DXGI_SWAP_CHAIN_DESC1 sc_desc = {};
    sc_desc.Width = width;
    sc_desc.Height = height;
    sc_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    sc_desc.Stereo = FALSE;
    sc_desc.SampleDesc.Count = 1;
    sc_desc.SampleDesc.Quality = 0;
    sc_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sc_desc.BufferCount = 2;
    sc_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    sc_desc.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;
    sc_desc.Scaling = DXGI_SCALING_STRETCH;

    IDXGIFactory2* dxgiFactory = nullptr;
    IDXGIDevice* dxgiDevice = nullptr;
    g_pd3dDevice->QueryInterface(IID_PPV_ARGS(&dxgiDevice));
    IDXGIAdapter* adapter = nullptr;
    dxgiDevice->GetAdapter(&adapter);
    adapter->GetParent(IID_PPV_ARGS(&dxgiFactory));

    bool ret = true;

    if (FAILED(dxgiFactory->CreateSwapChainForComposition(g_pd3dDevice, &sc_desc, nullptr, (IDXGISwapChain1**)&g_pSwapChain)))
        ret = false;

    if (!CreateRenderTarget())
        ret = false;

    dxgiDevice->Release();
    adapter->Release();
    dxgiFactory->Release();

    return ret;
}

bool CRender::CreateRenderTarget()
{
    if (!this->g_pSwapChain || !this->g_pd3dDevice)
        return false;

    ID3D11Texture2D* pBackBuffer = nullptr;
    HRESULT hr = this->g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    if (FAILED(hr) || !pBackBuffer)
        return false;

    hr = this->g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &this->g_mainRenderTargetView);
    if (FAILED(hr))
    {
        this->g_mainRenderTargetView = nullptr;
        return false;
    }

    pBackBuffer->Release();
    return true;
}

void CRender::CleanupRenderTarget()
{
    if (this->g_mainRenderTargetView) { this->g_mainRenderTargetView->Release(); this->g_mainRenderTargetView = nullptr; }
}

void CRender::CleanupSwapChain()
{
    this->CleanupRenderTarget();
    if (this->g_pSwapChain) { this->g_pSwapChain->Release(); this->g_pSwapChain = nullptr; }
}

void CRender::CleanupDeviceD3D()
{
    this->CleanupSwapChain();
    if (this->g_pd3dDeviceContext) { this->g_pd3dDeviceContext->Release(); this->g_pd3dDeviceContext = nullptr; }
    if (this->g_pd3dDevice) { this->g_pd3dDevice->Release(); this->g_pd3dDevice = nullptr; }
}

void CRender::CleanupDComp()
{
    if (this->g_dcompTarget) { this->g_dcompTarget->Release(); this->g_dcompTarget = nullptr; }
    if (this->g_dcompVisual) { this->g_dcompVisual->Release(); this->g_dcompVisual = nullptr; }
    if (this->g_dcompDevice) { this->g_dcompDevice->Release(); this->g_dcompDevice = nullptr; }
}

void CRender::OnWindowMovedOrResized()
{
    HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);

    if ((hMonitor == this->g_CurrentMonitor) && !this->bForceResize)
        return;

    this->bForceResize = false;
    this->g_CurrentMonitor = hMonitor;
    
    MONITORINFO mi{ sizeof(mi) };
    GetMonitorInfo(hMonitor, &mi);

    int width = mi.rcMonitor.right - mi.rcMonitor.left;
    int height = mi.rcMonitor.bottom - mi.rcMonitor.top;

    if (CTools::Get().IsTaskbarAutoHideEnabled())
        height -= 1;

    ImGui_ImplWin32_Shutdown();

    if (this->g_dcompTarget) { this->g_dcompTarget->Release(); this->g_dcompTarget = nullptr; }
    if (this->g_dcompVisual) { this->g_dcompVisual->Release(); this->g_dcompVisual = nullptr; }

    this->CleanupSwapChain();

    SHAppBarMessage(ABM_REMOVE, &abd);

    this->bInternalRecreate = true;
    DestroyWindow(this->hwnd);
    this->hwnd = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_NOREDIRECTIONBITMAP, this->wc.lpszClassName, L"PhasmoTimer", WS_POPUP, mi.rcMonitor.left, mi.rcMonitor.top, width, height, nullptr, nullptr, this->wc.hInstance, nullptr);
    this->bInternalRecreate = false;

    abd.hWnd = this->hwnd;
    SHAppBarMessage(ABM_NEW, &abd);

    CGameForegroundTracker::Get().SetOwnHwnd(this->hwnd);

    this->CreateSwapChain(width, height);

    this->g_dcompDevice->CreateTargetForHwnd(this->hwnd, TRUE, &this->g_dcompTarget);
    this->g_dcompDevice->CreateVisual(&this->g_dcompVisual);
    this->g_dcompVisual->SetContent(this->g_pSwapChain);
    this->g_dcompTarget->SetRoot(this->g_dcompVisual);
    this->g_dcompDevice->Commit();

    ::ShowWindow(this->hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(this->hwnd);

    ImGui_ImplWin32_Init(hwnd);
}

void CRender::OnDpiChanged()
{
    ImGuiStyle& style = ImGui::GetStyle();
    ImGui::LoadDefaultStyle(style);
    style.ScaleAllSizes(this->flPendingDpiScale);
    style.FontScaleDpi = this->flPendingDpiScale;
    ImGui::LoadTimerStyle(style);
}

bool CRender::LoadTextureFromMemory(const void* data, size_t data_size, ID3D11ShaderResourceView** out_srv, int* out_width, int* out_height)
{
    int image_width = 0;
    int image_height = 0;

    unsigned char* image_data = stbi_load_from_memory((const unsigned char*)data, (int)data_size, &image_width, &image_height, NULL, 4);
    if (image_data == NULL)
        return false;

    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Width = image_width;
    desc.Height = image_height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;

    ID3D11Texture2D* pTexture = NULL;
    D3D11_SUBRESOURCE_DATA subResource;
    subResource.pSysMem = image_data;
    subResource.SysMemPitch = desc.Width * 4;
    subResource.SysMemSlicePitch = 0;

    HRESULT hr = this->g_pd3dDevice->CreateTexture2D(&desc, &subResource, &pTexture);
    if (FAILED(hr))
    {
        stbi_image_free(image_data);
        return false;
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = desc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = desc.MipLevels;

    hr = this->g_pd3dDevice->CreateShaderResourceView(pTexture, &srvDesc, out_srv);
    pTexture->Release();

    if (FAILED(hr))
    {
        stbi_image_free(image_data);
        return false;
    }

    *out_width = image_width;
    *out_height = image_height;
    stbi_image_free(image_data);

    return true;
}

bool CRender::LoadTextureFromFile(const char* filename, ID3D11ShaderResourceView** out_srv, int* out_width, int* out_height)
{
    FILE* f = nullptr;
    errno_t err = fopen_s(&f, filename, "rb");
    if (err != 0 || !f)
        return false;

    fseek(f, 0, SEEK_END);
    size_t file_size = (size_t)ftell(f);
    fseek(f, 0, SEEK_SET);
    void* file_data = malloc(file_size);
    if (file_data == nullptr)
    {
        fclose(f);
        return false;
    }
    fread(file_data, 1, file_size, f);
    fclose(f);

    bool ret = LoadTextureFromMemory(file_data, file_size, out_srv, out_width, out_height);

    free(file_data);
    return ret;
}

void CRender::UpdateClickThroughState()
{
    if (this->hwnd == nullptr)
        return;

    const ImGuiIO& io = ImGui::GetIO();

    LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);

    if (io.WantCaptureMouse)
    {
        if (exStyle & WS_EX_TRANSPARENT)
            SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle & ~WS_EX_TRANSPARENT);
    }
    else
    {
        if (!(exStyle & WS_EX_TRANSPARENT))
            SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle | WS_EX_TRANSPARENT);
    }
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI CRender::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_WINDOWPOSCHANGED:
    {
        CRender::Get().bWindowMovedOrResized = true;
        return 0;
    }
    case WM_DPICHANGED:
    {
        CRender::Get().bDpiChanged = true;
        CRender::Get().flPendingDpiScale = HIWORD(wParam) / 96.0f;
        return 0;
    }
    case WM_DISPLAYCHANGE:
    {
        CRender::Get().bWindowMovedOrResized = true;
        CRender::Get().bForceResize = true;
        return 0;
    }
    case WM_SYSCOMMAND:
    {
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    }
    case WM_APPBAR:
    {
        if (wParam == ABN_STATECHANGE)
        {
            CRender::Get().bWindowMovedOrResized = true;
            CRender::Get().bForceResize = true;
        }

        return 0;
    }
    case WM_CLOSE:
    {
        CRender::Get().bWantExit = true;
        return 0;
    }
    case WM_DESTROY:
    {
        if (!CRender::Get().bInternalRecreate)
            ::PostQuitMessage(0);

        return 0;
    }
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}