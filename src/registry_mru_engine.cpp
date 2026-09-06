#include "registry_mru_engine.h"
#include "common/utils.h"
#include "windows/winapi_wrapper.h"
#include <algorithm>
#include <sstream>

namespace vmh {

static const wchar_t* TYPED_URLS_PATH = L"Software\\Microsoft\\Internet Explorer\\TypedURLs";
static const wchar_t* USERASSIST_CEBFF5CD_PATH = L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\UserAssist\\{CEBFF5CD-ACE2-4F4F-9178-9926F41749EA}\\Count";
static const wchar_t* USERASSIST_F4E57C4B_PATH = L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\UserAssist\\{F4E57C4B-2036-45F0-A9AB-443BCFE33D9F}\\Count";
static const wchar_t* RUN_MRU_PATH = L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\RunMRU";

RegistryMRUEngine::RegistryMRUEngine(const json& config) : m_config(config) {
    if (config.contains("registry_mru")) {
        const auto& r = config["registry_mru"];
        m_cfg.enableTypedURLs = r.value<bool>("enable_typed_urls", true);
        m_cfg.enableUserAssist = r.value<bool>("enable_user_assist", true);
        m_cfg.enableOfficeMRU = r.value<bool>("enable_office_mru", true);
        m_cfg.maxURLs = r.value<int>("max_urls", 20);
        m_cfg.maxOfficeEntries = r.value<int>("max_office_entries", 15);

        if (r.contains("include_programs") && r["include_programs"].isArray()) {
            for (auto& el : r["include_programs"].elements())
                m_cfg.includePrograms.push_back(el.getString());
        }
        if (r.contains("include_documents") && r["include_documents"].isArray()) {
            for (auto& el : r["include_documents"].elements())
                m_cfg.includeDocuments.push_back(el.getString());
        }
    }

    m_defaultURLs = {
        L"https://www.google.com", L"https://www.github.com",
        L"https://stackoverflow.com", L"https://www.microsoft.com",
        L"https://outlook.office365.com", L"https://teams.microsoft.com",
        L"https://www.linkedin.com", L"https://mail.google.com",
        L"https://docs.microsoft.com", L"https://www.youtube.com",
        L"https://news.ycombinator.com", L"https://www.reddit.com",
        L"https://www.amazon.de", L"https://www.heise.de",
        L"https://www.spiegel.de", L"https://www.wikipedia.org",
        L"https://portal.azure.com", L"https://jira.atlassian.com",
        L"https://confluence.atlassian.com", L"https://www.npmjs.com",
    };

    m_defaultPrograms = {
        L"notepad.exe", L"calc.exe", L"explorer.exe",
        L"cmd.exe", L"powershell.exe", L"taskmgr.exe",
        L"mspaint.exe", L"wordpad.exe", L"regedit.exe",
        L"snippingtool.exe",
    };
    if (!m_cfg.includePrograms.empty()) {
        m_defaultPrograms.clear();
        for (auto& p : m_cfg.includePrograms)
            m_defaultPrograms.push_back(utf8ToWide(p));
    }

    m_defaultDocuments = {
        {L"C:\\Users\\User\\Documents\\Projektplan.docx", "Word"},
        {L"C:\\Users\\User\\Documents\\Budget_2026.xlsx", "Excel"},
        {L"C:\\Users\\User\\Documents\\Praesentation_Q3.pptx", "PowerPoint"},
        {L"C:\\Users\\User\\Documents\\Meeting_Notes.docx", "Word"},
        {L"C:\\Users\\User\\Documents\\Zeiterfassung.xlsx", "Excel"},
        {L"C:\\Users\\User\\Desktop\\Bericht_September.docx", "Word"},
        {L"C:\\Users\\User\\Documents\\Angebot_Kunde_Mueller.docx", "Word"},
        {L"C:\\Users\\User\\Documents\\Umsatzanalyse.xlsx", "Excel"},
        {L"C:\\Users\\User\\Desktop\\Teammeeting_Agenda.pptx", "PowerPoint"},
        {L"C:\\Users\\User\\Documents\\Protokoll_2026-08-28.docx", "Word"},
        {L"\\\\fileserver\\shared\\Templates\\Briefvorlage.docx", "Word"},
        {L"\\\\fileserver\\shared\\Berichte\\Monatsbericht.xlsx", "Excel"},
    };

    m_defaultRunCommands = {
        L"cmd", L"powershell", L"notepad", L"calc", L"mstsc",
        L"control", L"devmgmt.msc", L"services.msc", L"regedit",
        L"taskmgr", L"msconfig", L"cleanmgr",
    };

    logInfo("RegistryMRUEngine initialized (URLs=%s, UserAssist=%s, OfficeMRU=%s)",
            m_cfg.enableTypedURLs ? "on" : "off",
            m_cfg.enableUserAssist ? "on" : "off",
            m_cfg.enableOfficeMRU ? "on" : "off");
}

std::wstring RegistryMRUEngine::rot13Encode(const std::wstring& input) {
    std::wstring output = input;
    for (auto& ch : output) {
        if ((ch >= L'A' && ch <= L'Z')) {
            ch = L'A' + ((ch - L'A' + 13) % 26);
        } else if (ch >= L'a' && ch <= L'z') {
            ch = L'a' + ((ch - L'a' + 13) % 26);
        }
    }
    return output;
}

FILETIME RegistryMRUEngine::generateRealisticTimestamp(int daysBack) {
    FILETIME now = getCurrentFileTime();
    int64_t offsetSec = -static_cast<int64_t>(daysBack) * 86400;
    offsetSec += randomInt(-43200, 43200); // +/- 12 hours variation
    return offsetFileTime(now, offsetSec);
}

std::vector<uint8_t> RegistryMRUEngine::buildUserAssistData(int runCount, const FILETIME& lastRun) {
    // UserAssist data structure (Windows 7+ format, 72 bytes)
    std::vector<uint8_t> data(72, 0);

    // Session (offset 0, 4 bytes) - set to 0
    // Run count (offset 4, 4 bytes)
    *reinterpret_cast<uint32_t*>(&data[4]) = static_cast<uint32_t>(runCount);

    // Focus count (offset 8, 4 bytes)
    *reinterpret_cast<uint32_t*>(&data[8]) = static_cast<uint32_t>(runCount + randomInt(0, 5));

    // Focus time in ms (offset 12, 4 bytes)
    *reinterpret_cast<uint32_t*>(&data[12]) = static_cast<uint32_t>(runCount * randomInt(30000, 300000));

    // Last execution time (offset 60, 8 bytes - FILETIME)
    *reinterpret_cast<FILETIME*>(&data[60]) = lastRun;

    return data;
}

void RegistryMRUEngine::cleanupRegistry(HKEY hRoot, const std::wstring& subKey, int maxEntries) {
    auto values = WinAPI::regEnumValues(hRoot, subKey);
    if (static_cast<int>(values.size()) <= maxEntries) return;

    HKEY hKey;
    if (WinAPI::regCreateKey(hRoot, subKey, hKey) != ERROR_SUCCESS) return;

    // Remove oldest entries (highest numbered)
    std::sort(values.begin(), values.end());
    for (size_t i = maxEntries; i < values.size(); ++i) {
        WinAPI::regDeleteValue(hKey, values[i]);
    }
    WinAPI::regCloseKey(hKey);
    logDebug("Cleaned up registry key, removed %zu entries", values.size() - maxEntries);
}

void RegistryMRUEngine::addTypedURL(const std::wstring& url) {
    HKEY hKey;
    LONG result = WinAPI::regCreateKey(HKEY_CURRENT_USER, TYPED_URLS_PATH, hKey);
    if (result != ERROR_SUCCESS) {
        logError("Failed to open TypedURLs key: 0x%08lX", result);
        return;
    }

    DWORD count = WinAPI::regGetValueCount(HKEY_CURRENT_USER, TYPED_URLS_PATH);
    std::wstring valueName = L"url" + std::to_wstring(count + 1);
    WinAPI::regSetString(hKey, valueName, url);
    WinAPI::regCloseKey(hKey);
}

void RegistryMRUEngine::addUserAssistEntry(const std::wstring& path) {
    std::wstring encoded = rot13Encode(path);

    HKEY hKey;
    LONG result = WinAPI::regCreateKey(HKEY_CURRENT_USER, USERASSIST_CEBFF5CD_PATH, hKey);
    if (result != ERROR_SUCCESS) {
        logError("Failed to open UserAssist key: 0x%08lX", result);
        return;
    }

    int runCount = randomInt(1, 50);
    FILETIME lastRun = generateRealisticTimestamp(randomInt(0, 14));
    auto data = buildUserAssistData(runCount, lastRun);

    WinAPI::regSetBinary(hKey, encoded, data);
    WinAPI::regCloseKey(hKey);
}

void RegistryMRUEngine::addOfficeMRU(const std::wstring& filePath, const std::string& officeApp) {
    std::wstring appName;
    if (officeApp == "Word") appName = L"Word";
    else if (officeApp == "Excel") appName = L"Excel";
    else if (officeApp == "PowerPoint") appName = L"PowerPoint";
    else return;

    std::wstring keyPath = L"Software\\Microsoft\\Office\\16.0\\" + appName + L"\\File MRU";

    HKEY hKey;
    LONG result = WinAPI::regCreateKey(HKEY_CURRENT_USER, keyPath, hKey);
    if (result != ERROR_SUCCESS) {
        logWarning("Failed to open Office MRU key for %s: 0x%08lX", officeApp.c_str(), result);
        return;
    }

    DWORD count = WinAPI::regGetValueCount(HKEY_CURRENT_USER, keyPath);
    std::wstring valueName = L"Item " + std::to_wstring(count + 1);

    // Office MRU format: [F00000000][T<hex_filetime>]*<filepath>
    FILETIME ft = generateRealisticTimestamp(randomInt(0, 30));
    ULARGE_INTEGER ul;
    ul.LowPart = ft.dwLowDateTime;
    ul.HighPart = ft.dwHighDateTime;

    wchar_t timeHex[32];
    swprintf(timeHex, 32, L"%016llX", ul.QuadPart);

    std::wstring mruValue = L"[F00000000][T" + std::wstring(timeHex) + L"]*" + filePath;
    WinAPI::regSetString(hKey, valueName, mruValue);
    WinAPI::regCloseKey(hKey);
}

void RegistryMRUEngine::addRunMRUEntry(const std::wstring& command) {
    HKEY hKey;
    LONG result = WinAPI::regCreateKey(HKEY_CURRENT_USER, RUN_MRU_PATH, hKey);
    if (result != ERROR_SUCCESS) {
        logError("Failed to open RunMRU key: 0x%08lX", result);
        return;
    }

    DWORD count = WinAPI::regGetValueCount(HKEY_CURRENT_USER, RUN_MRU_PATH);
    if (count > 26) count = 0;

    wchar_t letter = L'a' + static_cast<wchar_t>(count % 26);
    std::wstring valueName(1, letter);

    // RunMRU format: command\1
    std::wstring mruValue = command + L"\\1";
    WinAPI::regSetString(hKey, valueName, mruValue);

    // Update MRUList
    std::wstring mruList;
    for (DWORD i = 0; i <= count && i < 26; ++i) {
        mruList += static_cast<wchar_t>(L'a' + (count - i) % 26);
    }
    WinAPI::regSetString(hKey, L"MRUList", mruList);

    WinAPI::regCloseKey(hKey);
}

void RegistryMRUEngine::populateTypedURLs() {
    if (!m_cfg.enableTypedURLs) return;

    std::vector<std::wstring> urls = m_defaultURLs;
    // Shuffle for variety
    for (size_t i = urls.size() - 1; i > 0; --i) {
        size_t j = randomInt(0, static_cast<int>(i));
        std::swap(urls[i], urls[j]);
    }

    int count = std::min(m_cfg.maxURLs, static_cast<int>(urls.size()));
    for (int i = 0; i < count; ++i) {
        addTypedURL(urls[i]);
    }
    logInfo("Populated %d TypedURL entries", count);
}

void RegistryMRUEngine::populateUserAssist() {
    if (!m_cfg.enableUserAssist) return;

    for (auto& prog : m_defaultPrograms) {
        std::wstring fullPath = L"{6D809377-6AF0-444B-8957-A3773F02200E}\\" + prog;
        addUserAssistEntry(fullPath);
    }
    logInfo("Populated %zu UserAssist entries", m_defaultPrograms.size());
}

void RegistryMRUEngine::populateOfficeMRU() {
    if (!m_cfg.enableOfficeMRU) return;

    int count = std::min(m_cfg.maxOfficeEntries, static_cast<int>(m_defaultDocuments.size()));
    for (int i = 0; i < count; ++i) {
        addOfficeMRU(m_defaultDocuments[i].first, m_defaultDocuments[i].second);
    }
    logInfo("Populated %d Office MRU entries", count);
}

void RegistryMRUEngine::populateRunMRU() {
    int count = std::min(12, static_cast<int>(m_defaultRunCommands.size()));
    for (int i = 0; i < count; ++i) {
        addRunMRUEntry(m_defaultRunCommands[i]);
    }
    logInfo("Populated %d RunMRU entries", count);
}

void RegistryMRUEngine::populateAllMRU() {
    logInfo("Starting MRU population...");
    populateTypedURLs();
    populateUserAssist();
    populateOfficeMRU();
    populateRunMRU();
    logInfo("MRU population complete");
}

void RegistryMRUEngine::cleanupOldEntries() {
    cleanupRegistry(HKEY_CURRENT_USER, TYPED_URLS_PATH, m_cfg.maxURLs);
    cleanupRegistry(HKEY_CURRENT_USER, RUN_MRU_PATH, 26);
    logInfo("Old MRU entries cleaned up");
}

void RegistryMRUEngine::clearAllMRU() {
    auto clearKey = [](HKEY root, const std::wstring& path) {
        auto values = WinAPI::regEnumValues(root, path);
        if (values.empty()) return;
        HKEY hKey;
        if (WinAPI::regCreateKey(root, path, hKey) != ERROR_SUCCESS) return;
        for (auto& v : values) {
            WinAPI::regDeleteValue(hKey, v);
        }
        WinAPI::regCloseKey(hKey);
    };

    clearKey(HKEY_CURRENT_USER, TYPED_URLS_PATH);
    clearKey(HKEY_CURRENT_USER, USERASSIST_CEBFF5CD_PATH);
    clearKey(HKEY_CURRENT_USER, RUN_MRU_PATH);

    // Clear Office MRU for each app
    for (auto& app : {L"Word", L"Excel", L"PowerPoint"}) {
        std::wstring path = std::wstring(L"Software\\Microsoft\\Office\\16.0\\") + app + L"\\File MRU";
        clearKey(HKEY_CURRENT_USER, path);
    }

    logInfo("All MRU entries cleared");
}

} // namespace vmh
