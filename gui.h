#pragma once
#include "singleton.h"
#include <windows.h>
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <dxgi1_2.h>
#include <dcomp.h>
#include "input.h"
#include "timer.h"

#ifndef IM_PI
#define IM_PI 3.14159265358979323846f
#endif

struct FadeAnim
{
	float from = 1.0f;
	float to = 1.0f;
	float duration = 0.3f;
	float elapsed = 0.0f;
	bool  active = false;
};

class CGui : public Singleton<CGui>
{
public:
	bool Init(HINSTANCE hInstance);
	void Cleanup();
	void RenderLoop();
	
private:
	static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void ApplyStyle(ImGuiStyle& style);
	bool CreateDeviceD3D(int width, int height);
	void CleanupDeviceD3D();
	bool CreateRenderTarget();
	void CleanupRenderTarget();
	bool LoadTextureFromMemory(const void* data, size_t data_size, ID3D11ShaderResourceView** out_srv, int* out_width, int* out_height);
	bool LoadTextureFromFile(const char* filename, ID3D11ShaderResourceView** out_srv, int* out_width, int* out_height);
	void UpdateClickThroughState();
	bool StartNewFrame();
	void EndNewFrame();
	void TimerWindow();
	void SettingsWindow();
	void AboutWindow();
	void UpdaterWindow();
	bool KeyBindButton(const char* label, int* vk_key);
	void DrawTimerSection(const char* window_name, const char* time_value, float height_ratio, const ImVec2& available_size, const ImVec4* colors, bool bMain);
	void DrawTextOutline(ImDrawList* draw, ImFont* font, float size, const ImVec2& pos, ImU32 color, const char* text, float thickness = 1.0f);
	void DrawSoftGlow(ImDrawList* draw, ImFont* font, float size, const ImVec2& pos, const char* text, ImVec4 color, float flAlpha, float radius);
	void UpdateTimers();
	void StartFade(bool transparent);
	void UpdateFade();
	void ProcessFade();

	WNDCLASSEXW wc;
	ID3D11Device* g_pd3dDevice = nullptr;
	ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
	IDXGISwapChain* g_pSwapChain = nullptr;
	ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;
	ID3D11ShaderResourceView* logo_texture = nullptr;
	IDCompositionDevice* g_dcompDevice = nullptr;
	IDCompositionTarget* g_dcompTarget = nullptr;
	IDCompositionVisual* g_dcompVisual = nullptr;
	UINT g_ResizeWidth = 0U;
	UINT g_ResizeHeight = 0U;
	HWND hwnd = nullptr;
	bool g_SwapChainOccluded = false;
	bool bWantExit = false;
	bool bShowAbout = false;
	bool bShowSettings = false;

	int logo_width = 0;
	int logo_height = 0;

	std::chrono::steady_clock::time_point now_time{};
	CSmudgeTimer SmudgeTimerManual;
	CSmudgeTimer SmudgeTimerAuto;
	CObamboTimer ObamboTimer;
	CHuntTimer HuntTimer;
	FadeAnim fade;
	bool bAutoSmudgeTimer = false;
	bool bGameWindowActive = false;
	bool bSelfWindowActive = false;
};