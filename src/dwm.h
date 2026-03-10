#pragma once
#include "singleton.h"
#include <windows.h>

class CDWMSync : public Singleton<CDWMSync>
{
public:
	bool Init();
	void Cleanup();	
	void Sync();

private:
	void UpdateDWMTiming();
	bool WaitFor(LONGLONG ticks);
	LONGLONG GetTargetTime();

	LARGE_INTEGER now;

	LONGLONG dwm_refresh_period = 0LL;
	LONGLONG qpc_frequency = 0LL;
	LONGLONG avg_sleep_error_ticks = 0LL;
	LONGLONG dwm_last_update_qpc = 0LL;
	LONGLONG dwm_compose_base = 0LL;
	LONGLONG prev_frame_start = 0LL;
	LONGLONG update_interval_qpc = 0LL;
	LONGLONG avg_render_ticks = 0LL;

	UINT32 refresh_rate = 0LL;

	HANDLE hFrameTimer = nullptr;

	bool bFallbackTimer = false;
};