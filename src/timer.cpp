#include "timer.h"
#include "config.h"
#include "tools.h"
#include <algorithm>

namespace Timer
{
    namespace Smudge
    {
        constexpr const int64_t DEMON_TIME_MS = 60 * 1000;
        constexpr const int64_t HUNT_TIME_MS = 90 * 1000;
    }

    namespace Obambo
    {
        constexpr const int64_t MAX_MS = 2 * 60 * 1000;
        constexpr const int64_t START_MS = 60 * 1000;
    }

    namespace Candle
    {
        constexpr const int64_t MAX_MS = 30 * 1000;
        constexpr const int64_t START_MS = 30 * 1000;
    }

    constexpr const int64_t MAX_MS = 600000;
}

void CTimer::Set(const std::chrono::steady_clock::time_point& start_time)
{
	this->start_time = start_time;
    this->bRunning = true;
}

void CTimer::Reset()
{
    this->bRunning = false;
}

void CTimer::UpdateString()
{
    int minutes = static_cast<int>(this->value_ms / 60000);
    int seconds = static_cast<int>((this->value_ms % 60000) / 1000);
    int hundredths = static_cast<int>((this->value_ms % 1000) / 10);

    if (minutes > 0)
        std::snprintf(this->buffer, sizeof(this->buffer), "%d:%02d.%02d", minutes, seconds, hundredths);
    else
        std::snprintf(this->buffer, sizeof(this->buffer), "%d.%02d", seconds, hundredths);
}

const char* CTimer::GetString()
{
    return this->buffer;
}

const ImVec4* CTimer::GetColors()
{
    return this->color;
}

void CSmudgeTimer::Update(const std::chrono::steady_clock::time_point& now, const int64_t& start_from_ms)
{
    this->start_from_ms = start_from_ms;
    this->value_ms = this->bRunning ? (std::chrono::duration_cast<std::chrono::milliseconds>(now - this->start_time).count() + this->start_from_ms) : this->start_from_ms;

    if (this->value_ms >= std::clamp(CConfig::Get().iMaxMsSmudge, 1000LL, Timer::MAX_MS))
    {
        this->bRunning = false;
        this->value_ms = this->start_from_ms;
    }

    this->UpdateString();
    this->UpdateColors();
}

void CSmudgeTimer::UpdateColors()
{
    if (this->value_ms < Timer::Smudge::DEMON_TIME_MS)
    {
        this->color[0] = CConfig::Get().imvSafeTimeColor1;
        this->color[1] = CConfig::Get().imvSafeTimeColor2;
    }
    else if (this->value_ms < Timer::Smudge::HUNT_TIME_MS)
    {
        this->color[0] = CConfig::Get().imvDemonTimeColor1;
        this->color[1] = CConfig::Get().imvDemonTimeColor2;
    }
    else
    {
        this->color[0] = CConfig::Get().imvHuntTimeColor1;
        this->color[1] = CConfig::Get().imvHuntTimeColor2;
    }
}

void CObamboTimer::Set(const std::chrono::steady_clock::time_point& start_time)
{
    if (!this->bRunning)
    {
        this->start_time = start_time;
        this->bRunning = true;
    }
}

void CObamboTimer::Update(const std::chrono::steady_clock::time_point& now)
{
    this->start_from_ms = Timer::Obambo::START_MS;
    int64_t shifted = std::chrono::duration_cast<std::chrono::milliseconds>(now - this->start_time).count() + this->start_from_ms;
    int64_t currentCycle = 0LL;

    if (this->bRunning)
    {
        currentCycle = shifted / Timer::Obambo::MAX_MS;
        this->value_ms = Timer::Obambo::MAX_MS - (shifted % Timer::Obambo::MAX_MS);
    }
    else this->value_ms = this->start_from_ms;

    this->bAggressive = (currentCycle & 1);

    this->UpdateString();
    this->UpdateColors();
}

void CObamboTimer::UpdateColors()
{
    if (this->bAggressive)
    {
        this->color[0] = CConfig::Get().imvObamboAggressiveColor1;
        this->color[1] = CConfig::Get().imvObamboAggressiveColor2;
    }
    else
    {
        this->color[0] = CConfig::Get().imvObamboCalmColor1;
        this->color[1] = CConfig::Get().imvObamboCalmColor2;
    }
}

void CHuntTimer::Update(const std::chrono::steady_clock::time_point& now, const int64_t& start_from_ms)
{
    this->start_from_ms = start_from_ms;
    this->value_ms = this->bRunning ? (std::chrono::duration_cast<std::chrono::milliseconds>(now - this->start_time).count() + this->start_from_ms) : this->start_from_ms;

    if (this->value_ms >= std::clamp(CConfig::Get().iMaxMsHunt, 1000LL, Timer::MAX_MS))
    {
        this->bRunning = false;
        this->value_ms = this->start_from_ms;
    }

    this->UpdateString();
    this->UpdateColors();
}

void CHuntTimer::UpdateColors()
{
    this->color[0] = CConfig::Get().imvHuntTimerColor1;
    this->color[1] = CConfig::Get().imvHuntTimerColor2;
}

void CCandleTimer::Update(const std::chrono::steady_clock::time_point& now)
{
    this->start_from_ms = Timer::Candle::START_MS;
    int64_t shifted = std::chrono::duration_cast<std::chrono::milliseconds>(now - this->start_time).count() + this->start_from_ms;

    if (this->bRunning)
    {
        if ((shifted / Timer::Candle::MAX_MS) == 1LL)
            this->value_ms = Timer::Candle::MAX_MS - (shifted % Timer::Candle::MAX_MS);
        else
        {
            this->bRunning = false;
            this->value_ms = this->start_from_ms;
        }
    }
    else this->value_ms = this->start_from_ms;

    this->UpdateString();
    this->UpdateColors();
}

void CCandleTimer::UpdateColors()
{
    this->color[0] = CConfig::Get().imvCandleTimerColor1;
    this->color[1] = CConfig::Get().imvCandleTimerColor2;
}