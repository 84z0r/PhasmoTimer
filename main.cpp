#include "render.h"
#include "config.h"
#include "updater.h"
#include <thread>

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    CConfig::Get().Load();

    if (!CRender::Get().Init(hInstance))
        return 1;

    std::thread inputThread([]()->void { CInput::Get().InputThread(); });

    if (CConfig::Get().bCheckUpdates)
    {
        std::thread updaterThread([]()->void { CUpdater::Get().UpdaterThread(); });
        updaterThread.detach();
    }

    CRender::Get().RenderLoop();
    CInput::Get().StopInputThread();

    inputThread.join();
    CRender::Get().Cleanup();
    return 0;
}