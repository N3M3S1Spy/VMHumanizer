#pragma once
#include "common/json.hpp"
#include "common/types.h"
#include <string>
#include <vector>
#include <windows.h>

namespace vmh {

class RegistryMRUEngine {
public:
    RegistryMRUEngine(const json& config);

    void populateAllMRU();

    void addTypedURL(const std::wstring& url);
    void addUserAssistEntry(const std::wstring& path);
    void addOfficeMRU(const std::wstring& filePath, const std::string& officeApp);
    void addRunMRUEntry(const std::wstring& command);

    void cleanupOldEntries();
    void clearAllMRU();

private:
    std::wstring rot13Encode(const std::wstring& input);
    FILETIME generateRealisticTimestamp(int daysBack);
    std::vector<uint8_t> buildUserAssistData(int runCount, const FILETIME& lastRun);
    void cleanupRegistry(HKEY hRoot, const std::wstring& subKey, int maxEntries);
    void populateTypedURLs();
    void populateUserAssist();
    void populateOfficeMRU();
    void populateRunMRU();

    RegistryMRUConfig m_cfg;
    json m_config;

    std::vector<std::wstring> m_defaultURLs;
    std::vector<std::wstring> m_defaultPrograms;
    std::vector<std::pair<std::wstring, std::string>> m_defaultDocuments;
    std::vector<std::wstring> m_defaultRunCommands;
};

} // namespace vmh
