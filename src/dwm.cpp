#include "dwm.h"
#include <Dwmapi.h>
#include <cmath>
#include <algorithm>
#include "tools.h"

bool CDWMSync::Init()
{
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    this->qpc_frequency = freq.QuadPart;
	this->update_interval_qpc = this->qpc_frequency / 8;

    this->bFallbackTimer = false;

    this->hFrameTimer = CreateWaitableTimerEx(nullptr, nullptr, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);
    if (!this->hFrameTimer)
    {
        timeBeginPeriod(1);
        this->bFallbackTimer = true;
        this->hFrameTimer = CreateWaitableTimer(nullptr, FALSE, nullptr);	
    }

    if (!this->hFrameTimer)
    {
        if (this->bFallbackTimer)
            timeEndPeriod(1);

		return false;
    }

    QueryPerformanceCounter(&this->now);
    this->UpdateDWMTiming();
    this->avg_render_ticks = this->dwm_refresh_period / 8;
    this->prev_frame_start = this->now.QuadPart - this->avg_render_ticks;

    return true;
}

void CDWMSync::Cleanup()
{
    if (this->hFrameTimer)
    {
        CloseHandle(this->hFrameTimer);
        this->hFrameTimer = nullptr;
    }

	if (this->bFallbackTimer)
        timeEndPeriod(1);
}

void CDWMSync::UpdateDWMTiming()
{
    DWM_TIMING_INFO timing{};
    timing.cbSize = sizeof(timing);

    if (FAILED(DwmGetCompositionTimingInfo(nullptr, &timing)))
        return;

    this->dwm_refresh_period = timing.qpcRefreshPeriod;
    this->dwm_compose_base = timing.qpcCompose;

    UINT32 den = timing.rateCompose.uiDenominator ? timing.rateCompose.uiDenominator : 1;
    this->refresh_rate = (timing.rateCompose.uiNumerator + den / 2) / den;
}

LONGLONG CDWMSync::GetTargetTime()
{
    LONGLONG predicted_render = (this->avg_render_ticks * 3) >> 1;
    LONGLONG safety_margin = std::max(predicted_render, std::llabs(this->avg_sleep_error_ticks * 2)) + this->avg_sleep_error_ticks;
    LONGLONG delta = this->now.QuadPart + safety_margin - this->dwm_compose_base;

    LONGLONG periods;
    if (delta >= 0)
        periods = (delta + this->dwm_refresh_period - 1) / this->dwm_refresh_period;
    else
        periods = delta / this->dwm_refresh_period;

    LONGLONG compose = this->dwm_compose_base + periods * this->dwm_refresh_period;
    LONGLONG target = compose - safety_margin;

    return target;
}

bool CDWMSync::WaitFor(LONGLONG ticks)
{
    if (ticks <= 0)
        return false;

    LONGLONG due_100ns = (ticks / this->qpc_frequency) * 10000000LL + (ticks % this->qpc_frequency) * 10000000LL / this->qpc_frequency;

    if (due_100ns < 1)
        due_100ns = 1;

    LARGE_INTEGER due_time;
    due_time.QuadPart = -due_100ns;

    SetWaitableTimer(this->hFrameTimer, &due_time, 0, nullptr, nullptr, FALSE);
    WaitForSingleObject(this->hFrameTimer, INFINITE);
    return true;
}

void CDWMSync::Sync()
{
    auto& tools = CTools::Get();

    QueryPerformanceCounter(&this->now);

    LONGLONG frame_ticks = std::clamp(this->now.QuadPart - this->prev_frame_start, 0LL, this->dwm_refresh_period * 2);
    this->avg_render_ticks = tools.EMA(this->avg_render_ticks, frame_ticks, this->refresh_rate);

    if (this->now.QuadPart - this->dwm_last_update_qpc > this->update_interval_qpc)
    {
        this->UpdateDWMTiming();
        this->dwm_last_update_qpc = this->now.QuadPart;
    }

    LONGLONG target = this->GetTargetTime();
    LONGLONG wait_ticks = target - this->now.QuadPart;

    if (this->WaitFor(wait_ticks))
    {
        QueryPerformanceCounter(&this->now);

        LONGLONG error = this->now.QuadPart - target;
        LONGLONG max_error = this->dwm_refresh_period / 4;

        if (error > max_error) error = max_error;
        if (error < -max_error) error = -max_error;

        this->avg_sleep_error_ticks = tools.EMA(this->avg_sleep_error_ticks, error, this->refresh_rate);
    }

    this->prev_frame_start = this->now.QuadPart;
}