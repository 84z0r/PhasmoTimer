#include "updater.h"
#include "tools.h"
#include "version.h"
#define CURL_STATICLIB
#include <curl/curl.h>

static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t total = size * nmemb;
    std::string* response = static_cast<std::string*>(userp);
    response->append(static_cast<char*>(contents), total);
    return total;
}

void CUpdater::UpdaterThread()
{
    this->Init();
    bool res = this->CheckUpdateAvailable();
    this->bUpdateAvailable.store(res, std::memory_order::relaxed);
    this->Cleanup();
}

void CUpdater::Init()
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

void CUpdater::Cleanup()
{
    curl_global_cleanup();
}

std::string CUpdater::GetLatestVersion()
{
    const char* url = "https://raw.githubusercontent.com/84z0r/PhasmoTimer/refs/heads/main/version";

    std::string response = "";
    CURL* curl = curl_easy_init();
    if (!curl)
        return "";

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 0L);
    curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2TLS);
    curl_easy_setopt(curl, CURLOPT_DNS_CACHE_TIMEOUT, 60L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 2000L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 4000L);

    int retries = 3;
    CURLcode res = CURLcode::CURL_LAST;

    while (retries > 0)
    {
        res = curl_easy_perform(curl);
        if (res == CURLE_OK)
            return response;

        retries--;
    }

    curl_easy_cleanup(curl);
    return response;
}

bool CUpdater::CheckUpdateAvailable()
{
    std::string latest_ver = this->GetLatestVersion();
    uint64_t uLatestVersion = 0Ui64;
    if (!CTools::Get().parseULL(latest_ver, uLatestVersion))
        return false;

    if (uLatestVersion > Version::APP_VERSION_UINT64)
        return true;

    return false;
}