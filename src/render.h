#pragma once
#include "singleton.h"
#include <windows.h>
#include <imgui/imgui_impl_win32.h>
#include <imgui/imgui_impl_dx11.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <dcomp.h>
#include <chrono>

class CRender : public Singleton<CRender>
{
public:
	bool Init(HINSTANCE hInstance);
	void Cleanup();
	void RenderLoop();

	inline bool IsGameWindowActive() const { return this->bGameWindowActive; }
	inline bool IsSelfWindowActive() const { return this->bSelfWindowActive; }
	inline const std::chrono::steady_clock::time_point& GetNowTime() const { return this->now_time; }
	
	ID3D11ShaderResourceView* logo_texture = nullptr;
	bool bWantExit = false;
	
private:
	static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void OnWindowMovedOrResized();
	void OnDpiChanged();
	bool CreateDeviceD3D(int width, int height);
	bool CreateSwapChain(int width, int height);
	void CleanupSwapChain();
	void CleanupDeviceD3D();
	void CleanupDComp();
	bool CreateRenderTarget();
	void CleanupRenderTarget();
	bool LoadTextureFromMemory(const void* data, size_t data_size, ID3D11ShaderResourceView** out_srv, int* out_width, int* out_height);
	bool LoadTextureFromFile(const char* filename, ID3D11ShaderResourceView** out_srv, int* out_width, int* out_height);
	void UpdateClickThroughState();
	bool StartNewFrame();
	void EndNewFrame();

	WNDCLASSEXW wc = {};
	APPBARDATA abd = {};
	ID3D11Device* g_pd3dDevice = nullptr;
	ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
	IDXGISwapChain* g_pSwapChain = nullptr;
	ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;
	IDCompositionDevice* g_dcompDevice = nullptr;
	IDCompositionTarget* g_dcompTarget = nullptr;
	IDCompositionVisual* g_dcompVisual = nullptr;
	HMONITOR g_CurrentMonitor = nullptr;
	HWND hwnd = nullptr;
	
	std::chrono::steady_clock::time_point now_time{};

	float flPendingDpiScale = 1.0f;

	int logo_width = 0;
	int logo_height = 0;

	bool g_SwapChainOccluded = false;
	bool bWindowMovedOrResized = false;
	bool bDpiChanged = false;
	bool bInternalRecreate = false;
	bool bForceResize = false;
	bool bGameWindowActive = false;
	bool bSelfWindowActive = false;
};