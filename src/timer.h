#pragma once
#include <chrono>
#include <imgui/imgui.h>

class CTimer
{
public:
	void Set(const std::chrono::steady_clock::time_point& start_time);
	void Reset();
	const char* GetString();
	const ImVec4* GetColors();
protected:
	void UpdateString();
	std::chrono::steady_clock::time_point start_time{};
	int64_t value_ms = 0LL;
	int64_t start_from_ms = 0LL;
	char buffer[16] = {};
	ImVec4 color[2] = {};
	bool bRunning = false;
};

class CSmudgeTimer : public CTimer
{
public:
	void Update(const std::chrono::steady_clock::time_point& now, const int64_t& start_from_ms);
private:
	void UpdateColors();
};

class CObamboTimer : public CTimer
{
public:
	void Set(const std::chrono::steady_clock::time_point& start_time);
	void Update(const std::chrono::steady_clock::time_point& now);
private:
	void UpdateColors();
	bool bAggressive = false;
};

class CHuntTimer : public CTimer
{
public:
	void Update(const std::chrono::steady_clock::time_point& now, const int64_t& start_from_ms);
private:
	void UpdateColors();
};

class CCandleTimer : public CTimer
{
public:
	void Update(const std::chrono::steady_clock::time_point& now);
private:
	void UpdateColors();
};