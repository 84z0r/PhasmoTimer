#include "render.h"
#include "config.h"
#include "updater.h"
#include <thread>

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
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