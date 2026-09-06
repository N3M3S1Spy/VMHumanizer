#pragma once
#include "common/json.hpp"
#include <string>
#include <vector>
#include <windows.h>

namespace vmh {

class LNKFileGenerator {
public:
    LNKFileGenerator(const json& config);

    void createLNKFile(const std::wstring& targetPath,
                       const std::wstring& lnkPath,
                       const std::wstring& description);

    void populateRecentDocuments();
    void populateDesktopShortcuts();
    void clearAllLNKFiles();

private:
    struct LNKEntry {
        std::wstring target;
        std::wstring name;
        std::wstring description;
    };

    std::wstring getRecentDirectory();
    std::wstring getDesktopDirectory();
    bool createShortcutCOM(const std::wstring& targetPath,
                           const std::wstring& lnkPath,
                           const std::wstring& description,
                           const std::wstring& workingDir = L"");

    json m_config;
    std::vector<LNKEntry> m_recentDocs;
    std::vector<LNKEntry> m_desktopShortcuts;
};

} // namespace vmh
