#pragma once
#include "singleton.h"
#include <string>
#include <atomic>

class CUpdater : public Singleton<CUpdater>
{
public:
	void UpdaterThread();
	std::atomic<bool> bUpdateAvailable = false;
private:
	void Init();
	std::string GetLatestVersion();
	bool CheckUpdateAvailable();
	void Cleanup();
};