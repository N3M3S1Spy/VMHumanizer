#include "prefetch_generator.h"
#include "common/utils.h"
#include <fstream>
#include <filesystem>
#include <algorithm>

namespace vmh {

PrefetchGenerator::PrefetchGenerator(const json& config) : m_config(config) {
    if (config.contains("prefetch")) {
        const auto& pf = config["prefetch"];
        m_maxPrefetchFiles = pf.value<int>("max_prefetch_files", 128);

        if (pf.contains("apps_to_generate") && pf["apps_to_generate"].isArray()) {
            for (auto& app : pf["apps_to_generate"].elements()) {
                AppConfig ac;
                ac.name = utf8ToWide(app.value<std::string>("name", "notepad.exe"));
                if (app.contains("run_count") && app["run_count"].isArray()) {
                    ac.runCountMin = static_cast<int>(app["run_count"][0].getInt(10));
                    ac.runCountMax = static_cast<int>(app["run_count"][1].getInt(100));
                } else {
                    ac.runCountMin = 10;
                    ac.runCountMax = 100;
                }
                m_apps.push_back(ac);
            }
        }
    }

    if (m_apps.empty()) {
        m_apps = {
            {L"NOTEPAD.EXE", 50, 150},
            {L"CALC.EXE", 20, 80},
            {L"CMD.EXE", 30, 200},
            {L"EXPLORER.EXE", 100, 500},
            {L"TASKMGR.EXE", 10, 50},
            {L"POWERSHELL.EXE", 20, 100},
            {L"MSPAINT.EXE", 5, 30},
            {L"REGEDIT.EXE", 3, 15},
        };
    }

    logInfo("PrefetchGenerator initialized (%zu apps configured)", m_apps.size());
}

std::wstring PrefetchGenerator::getPrefetchDirectory() {
    return L"C:\\Windows\\Prefetch";
}

uint32_t PrefetchGenerator::calculatePrefetchHash(const std::wstring& path) {
    // SCCA hash algorithm (simplified)
    uint32_t hash = 0;
    for (wchar_t ch : path) {
        wchar_t upper = static_cast<wchar_t>(towupper(ch));
        hash = (hash * 37 + upper) & 0xFFFFFFFF;
    }
    return hash;
}

std::vector<std::wstring> PrefetchGenerator::getDefaultDependencies(const std::wstring& appName) {
    std::vector<std::wstring> deps = {
        L"\\DEVICE\\HARDDISKVOLUME2\\WINDOWS\\SYSTEM32\\NTDLL.DLL",
        L"\\DEVICE\\HARDDISKVOLUME2\\WINDOWS\\SYSTEM32\\KERNEL32.DLL",
        L"\\DEVICE\\HARDDISKVOLUME2\\WINDOWS\\SYSTEM32\\KERNELBASE.DLL",
        L"\\DEVICE\\HARDDISKVOLUME2\\WINDOWS\\SYSTEM32\\MSVCRT.DLL",
        L"\\DEVICE\\HARDDISKVOLUME2\\WINDOWS\\SYSTEM32\\ADVAPI32.DLL",
        L"\\DEVICE\\HARDDISKVOLUME2\\WINDOWS\\SYSTEM32\\USER32.DLL",
        L"\\DEVICE\\HARDDISKVOLUME2\\WINDOWS\\SYSTEM32\\GDI32.DLL",
        L"\\DEVICE\\HARDDISKVOLUME2\\WINDOWS\\SYSTEM32\\COMBASE.DLL",
        L"\\DEVICE\\HARDDISKVOLUME2\\WINDOWS\\SYSTEM32\\RPCRT4.DLL",
        L"\\DEVICE\\HARDDISKVOLUME2\\WINDOWS\\SYSTEM32\\SECHOST.DLL",
    };

    std::wstring upper = appName;
    std::transform(upper.begin(), upper.end(), upper.begin(), ::towupper);

    if (upper == L"CMD.EXE" || upper == L"POWERSHELL.EXE") {
        deps.push_back(L"\\DEVICE\\HARDDISKVOLUME2\\WINDOWS\\SYSTEM32\\BCRYPTPRIMITIVES.DLL");
        deps.push_back(L"\\DEVICE\\HARDDISKVOLUME2\\WINDOWS\\SYSTEM32\\OLEAUT32.DLL");
    }
    if (upper == L"NOTEPAD.EXE") {
        deps.push_back(L"\\DEVICE\\HARDDISKVOLUME2\\WINDOWS\\SYSTEM32\\COMDLG32.DLL");
        deps.push_back(L"\\DEVICE\\HARDDISKVOLUME2\\WINDOWS\\SYSTEM32\\SHELL32.DLL");
    }
    if (upper == L"EXPLORER.EXE") {
        deps.push_back(L"\\DEVICE\\HARDDISKVOLUME2\\WINDOWS\\SYSTEM32\\SHELL32.DLL");
        deps.push_back(L"\\DEVICE\\HARDDISKVOLUME2\\WINDOWS\\SYSTEM32\\SHLWAPI.DLL");
        deps.push_back(L"\\DEVICE\\HARDDISKVOLUME2\\WINDOWS\\SYSTEM32\\OLE32.DLL");
        deps.push_back(L"\\DEVICE\\HARDDISKVOLUME2\\WINDOWS\\SYSTEM32\\PROPSYS.DLL");
    }

    return deps;
}

void PrefetchGenerator::writePrefetchFile(const std::wstring& filename,
                                           const std::wstring& appName,
                                           int runCount,
                                           const std::vector<std::wstring>& dependencies) {
    std::wstring pfDir = getPrefetchDirectory();
    std::wstring fullPath = pfDir + L"\\" + filename;

    // Build binary content
    std::vector<uint8_t> data;

    // Header
    PrefetchHeader header = {};
    header.version = 30; // Windows 10
    header.signature = 0x41434353; // "SCCA"

    std::wstring upperApp = appName;
    std::transform(upperApp.begin(), upperApp.end(), upperApp.begin(), ::towupper);
    wcsncpy(header.executableName, upperApp.c_str(),
            std::min(static_cast<size_t>(29), upperApp.size()));

    header.prefetchHash = calculatePrefetchHash(
        L"\\DEVICE\\HARDDISKVOLUME2\\WINDOWS\\SYSTEM32\\" + upperApp);

    // Calculate sizes
    size_t headerSize = sizeof(PrefetchHeader);
    size_t fileInfoSize = sizeof(PrefetchFileInfo);
    size_t depsSize = 0;
    for (auto& dep : dependencies) {
        depsSize += (dep.size() + 1) * sizeof(wchar_t);
    }

    header.fileSize = static_cast<uint32_t>(headerSize + fileInfoSize + depsSize + 256);

    // Write header
    data.resize(header.fileSize, 0);
    memcpy(data.data(), &header, sizeof(header));

    // File info
    PrefetchFileInfo fileInfo = {};
    fileInfo.metricsOffset = static_cast<uint32_t>(headerSize + fileInfoSize);
    fileInfo.metricsCount = static_cast<uint32_t>(dependencies.size());
    fileInfo.filenameStringsOffset = fileInfo.metricsOffset;
    fileInfo.filenameStringsSize = static_cast<uint32_t>(depsSize);
    fileInfo.runCount = static_cast<uint32_t>(runCount);

    // Generate realistic last run times
    FILETIME now = getCurrentFileTime();
    for (int i = 0; i < 8; ++i) {
        FILETIME ft = offsetFileTime(now, -static_cast<int64_t>(i) * 86400 - randomInt(0, 43200));
        ULARGE_INTEGER ul;
        ul.LowPart = ft.dwLowDateTime;
        ul.HighPart = ft.dwHighDateTime;
        fileInfo.lastRunTimes[i] = ul.QuadPart;
    }

    memcpy(data.data() + headerSize, &fileInfo, sizeof(fileInfo));

    // Write dependency strings
    size_t offset = headerSize + fileInfoSize;
    for (auto& dep : dependencies) {
        if (offset + (dep.size() + 1) * sizeof(wchar_t) <= data.size()) {
            memcpy(data.data() + offset, dep.c_str(), (dep.size() + 1) * sizeof(wchar_t));
            offset += (dep.size() + 1) * sizeof(wchar_t);
        }
    }

    // Write to file
    try {
        std::ofstream file(fullPath, std::ios::binary);
        if (file.is_open()) {
            file.write(reinterpret_cast<const char*>(data.data()), data.size());
            logDebug("Prefetch file written: %s", wideToUtf8(filename).c_str());
        } else {
            logWarning("Cannot write prefetch file: %s (may need admin)", wideToUtf8(fullPath).c_str());
        }
    } catch (const std::exception& e) {
        logWarning("Prefetch write failed: %s", e.what());
    }
}

void PrefetchGenerator::generatePrefetchForApp(const std::wstring& appName, int runCount) {
    std::wstring upper = appName;
    std::transform(upper.begin(), upper.end(), upper.begin(), ::towupper);

    uint32_t hash = calculatePrefetchHash(
        L"\\DEVICE\\HARDDISKVOLUME2\\WINDOWS\\SYSTEM32\\" + upper);

    wchar_t filename[MAX_PATH];
    swprintf(filename, MAX_PATH, L"%s-%08X.pf", upper.c_str(), hash);

    auto deps = getDefaultDependencies(appName);
    writePrefetchFile(filename, appName, runCount, deps);
}

void PrefetchGenerator::generateAllPrefetches() {
    logInfo("Generating prefetch files for %zu apps...", m_apps.size());

    for (auto& app : m_apps) {
        int runCount = randomInt(app.runCountMin, app.runCountMax);
        generatePrefetchForApp(app.name, runCount);
    }

    logInfo("Prefetch generation complete");
}

void PrefetchGenerator::addFileDependency(const std::wstring& prefetchName,
                                           const std::wstring& filePath) {
    logDebug("Adding dependency to %s: %s",
             wideToUtf8(prefetchName).c_str(), wideToUtf8(filePath).c_str());
}

void PrefetchGenerator::cleanupOldPrefetches() {
    std::wstring pfDir = getPrefetchDirectory();
    try {
        std::vector<std::filesystem::directory_entry> entries;
        for (auto& entry : std::filesystem::directory_iterator(pfDir)) {
            if (entry.path().extension() == L".pf") {
                entries.push_back(entry);
            }
        }

        if (static_cast<int>(entries.size()) > m_maxPrefetchFiles) {
            std::sort(entries.begin(), entries.end(),
                [](const auto& a, const auto& b) {
                    return std::filesystem::last_write_time(a) < std::filesystem::last_write_time(b);
                });

            int toRemove = static_cast<int>(entries.size()) - m_maxPrefetchFiles;
            for (int i = 0; i < toRemove; ++i) {
                std::filesystem::remove(entries[i].path());
            }
            logInfo("Cleaned up %d old prefetch files", toRemove);
        }
    } catch (const std::exception& e) {
        logWarning("Prefetch cleanup failed: %s", e.what());
    }
}

void PrefetchGenerator::clearAllPrefetches() {
    logWarning("clearAllPrefetches: Clearing prefetch directory requires admin privileges");
}

} // namespace vmh
