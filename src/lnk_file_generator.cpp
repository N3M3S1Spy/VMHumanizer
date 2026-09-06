#include "lnk_file_generator.h"
#include "common/utils.h"
#include <shlobj.h>
#include <objbase.h>
#include <shobjidl.h>
#include <filesystem>

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "shell32.lib")

namespace vmh {

LNKFileGenerator::LNKFileGenerator(const json& config) : m_config(config) {
    if (config.contains("lnk_files")) {
        const auto& lnk = config["lnk_files"];

        if (lnk.contains("recent_docs") && lnk["recent_docs"].isArray()) {
            for (auto& doc : lnk["recent_docs"].elements()) {
                m_recentDocs.push_back({
                    utf8ToWide(doc.value<std::string>("target", "")),
                    utf8ToWide(doc.value<std::string>("name", "")),
                    utf8ToWide(doc.value<std::string>("desc", ""))
                });
            }
        }

        if (lnk.contains("desktop_shortcuts") && lnk["desktop_shortcuts"].isArray()) {
            for (auto& sc : lnk["desktop_shortcuts"].elements()) {
                m_desktopShortcuts.push_back({
                    utf8ToWide(sc.value<std::string>("target", "")),
                    utf8ToWide(sc.value<std::string>("name", "")),
                    utf8ToWide(sc.value<std::string>("desc", ""))
                });
            }
        }
    }

    // Default recent documents
    if (m_recentDocs.empty()) {
        m_recentDocs = {
            {L"C:\\Users\\User\\Documents\\Projektplan.docx", L"Projektplan", L"Projektplan Dokument"},
            {L"C:\\Users\\User\\Documents\\Budget_2026.xlsx", L"Budget 2026", L"Budgetplanung"},
            {L"C:\\Users\\User\\Documents\\Praesentation_Q3.pptx", L"Praesentation Q3", L"Quartalspräsentation"},
            {L"C:\\Users\\User\\Documents\\Meeting_Notes.docx", L"Meeting Notes", L"Besprechungsnotizen"},
            {L"C:\\Users\\User\\Desktop\\Bericht_September.docx", L"Bericht September", L"Monatsbericht"},
            {L"C:\\Users\\User\\Documents\\Zeiterfassung.xlsx", L"Zeiterfassung", L"Stundenaufzeichnung"},
        };
    }

    if (m_desktopShortcuts.empty()) {
        m_desktopShortcuts = {
            {L"C:\\Windows\\System32\\notepad.exe", L"Editor", L"Windows Editor"},
            {L"C:\\Windows\\System32\\calc.exe", L"Rechner", L"Windows Rechner"},
            {L"C:\\Windows\\System32\\cmd.exe", L"Eingabeaufforderung", L"Windows Kommandozeile"},
        };
    }

    logInfo("LNKFileGenerator initialized (%zu recent, %zu desktop)",
            m_recentDocs.size(), m_desktopShortcuts.size());
}

std::wstring LNKFileGenerator::getRecentDirectory() {
    PWSTR path = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Recent, 0, nullptr, &path))) {
        std::wstring result = path;
        CoTaskMemFree(path);
        return result;
    }
    return L"";
}

std::wstring LNKFileGenerator::getDesktopDirectory() {
    PWSTR path = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Desktop, 0, nullptr, &path))) {
        std::wstring result = path;
        CoTaskMemFree(path);
        return result;
    }
    return L"";
}

bool LNKFileGenerator::createShortcutCOM(const std::wstring& targetPath,
                                          const std::wstring& lnkPath,
                                          const std::wstring& description,
                                          const std::wstring& workingDir) {
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    bool comInitialized = SUCCEEDED(hr) || hr == S_FALSE;

    IShellLinkW* psl = nullptr;
    hr = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER,
                          IID_IShellLinkW, reinterpret_cast<void**>(&psl));

    if (FAILED(hr)) {
        logError("CoCreateInstance for ShellLink failed: 0x%08lX", hr);
        if (comInitialized) CoUninitialize();
        return false;
    }

    psl->SetPath(targetPath.c_str());
    if (!description.empty()) psl->SetDescription(description.c_str());
    if (!workingDir.empty()) {
        psl->SetWorkingDirectory(workingDir.c_str());
    } else {
        std::wstring dir = targetPath;
        auto pos = dir.find_last_of(L"\\/");
        if (pos != std::wstring::npos) dir = dir.substr(0, pos);
        psl->SetWorkingDirectory(dir.c_str());
    }

    IPersistFile* ppf = nullptr;
    hr = psl->QueryInterface(IID_IPersistFile, reinterpret_cast<void**>(&ppf));

    bool success = false;
    if (SUCCEEDED(hr)) {
        hr = ppf->Save(lnkPath.c_str(), TRUE);
        success = SUCCEEDED(hr);
        if (!success) {
            logError("IPersistFile::Save failed: 0x%08lX", hr);
        }
        ppf->Release();
    }

    psl->Release();
    if (comInitialized) CoUninitialize();
    return success;
}

void LNKFileGenerator::createLNKFile(const std::wstring& targetPath,
                                      const std::wstring& lnkPath,
                                      const std::wstring& description) {
    if (createShortcutCOM(targetPath, lnkPath, description)) {
        logDebug("LNK created: %s -> %s",
                 wideToUtf8(lnkPath).c_str(), wideToUtf8(targetPath).c_str());
    } else {
        logWarning("Failed to create LNK: %s", wideToUtf8(lnkPath).c_str());
    }
}

void LNKFileGenerator::populateRecentDocuments() {
    std::wstring recentDir = getRecentDirectory();
    if (recentDir.empty()) {
        logError("Cannot determine Recent Documents directory");
        return;
    }

    logInfo("Populating %zu recent document shortcuts...", m_recentDocs.size());

    for (auto& doc : m_recentDocs) {
        std::wstring lnkName = doc.name.empty() ?
            std::filesystem::path(doc.target).stem().wstring() : doc.name;
        std::wstring lnkPath = recentDir + L"\\" + lnkName + L".lnk";

        createLNKFile(doc.target, lnkPath, doc.description);
    }

    logInfo("Recent documents populated");
}

void LNKFileGenerator::populateDesktopShortcuts() {
    std::wstring desktopDir = getDesktopDirectory();
    if (desktopDir.empty()) {
        logError("Cannot determine Desktop directory");
        return;
    }

    logInfo("Populating %zu desktop shortcuts...", m_desktopShortcuts.size());

    for (auto& sc : m_desktopShortcuts) {
        std::wstring lnkName = sc.name.empty() ?
            std::filesystem::path(sc.target).stem().wstring() : sc.name;
        std::wstring lnkPath = desktopDir + L"\\" + lnkName + L".lnk";

        createLNKFile(sc.target, lnkPath, sc.description);
    }

    logInfo("Desktop shortcuts populated");
}

void LNKFileGenerator::clearAllLNKFiles() {
    auto clearDir = [](const std::wstring& dir, const std::wstring& ext) {
        try {
            for (auto& entry : std::filesystem::directory_iterator(dir)) {
                if (entry.path().extension() == ext) {
                    std::filesystem::remove(entry.path());
                }
            }
        } catch (...) {}
    };

    std::wstring recentDir = getRecentDirectory();
    if (!recentDir.empty()) clearDir(recentDir, L".lnk");

    logInfo("All generated LNK files cleared");
}

} // namespace vmh
