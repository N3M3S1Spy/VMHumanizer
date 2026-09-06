#pragma once
#include "common/json.hpp"
#include <string>
#include <vector>
#include <windows.h>

namespace vmh {

class PrefetchGenerator {
public:
    PrefetchGenerator(const json& config);

    void generateAllPrefetches();
    void generatePrefetchForApp(const std::wstring& appName, int runCount);
    void addFileDependency(const std::wstring& prefetchName, const std::wstring& filePath);
    void cleanupOldPrefetches();
    void clearAllPrefetches();

private:
#pragma pack(push, 1)
    struct PrefetchHeader {
        uint32_t version;         // 30 for Win10
        uint32_t signature;       // "SCCA" = 0x41434353
        uint32_t unknown1;
        uint32_t fileSize;
        wchar_t  executableName[30];
        uint32_t prefetchHash;
        uint32_t unknown2;
    };

    struct PrefetchFileInfo {
        uint32_t metricsOffset;
        uint32_t metricsCount;
        uint32_t traceChainsOffset;
        uint32_t traceChainsCount;
        uint32_t filenameStringsOffset;
        uint32_t filenameStringsSize;
        uint32_t volumeInfoOffset;
        uint32_t volumeInfoCount;
        uint32_t volumeInfoSize;
        uint64_t lastRunTimes[8];
        uint64_t unknown3;
        uint32_t runCount;
        uint32_t unknown4;
        uint32_t unknown5;
    };
#pragma pack(pop)

    struct AppConfig {
        std::wstring name;
        int runCountMin;
        int runCountMax;
    };

    uint32_t calculatePrefetchHash(const std::wstring& path);
    void writePrefetchFile(const std::wstring& filename, const std::wstring& appName,
                           int runCount, const std::vector<std::wstring>& dependencies);
    std::vector<std::wstring> getDefaultDependencies(const std::wstring& appName);
    std::wstring getPrefetchDirectory();

    json m_config;
    std::vector<AppConfig> m_apps;
    int m_maxPrefetchFiles = 128;
};

} // namespace vmh
