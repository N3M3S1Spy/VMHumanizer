#include "event_log_injector.h"
#include "common/utils.h"
#include "windows/winapi_wrapper.h"
#include <algorithm>
#include <sstream>

namespace vmh {

const std::wstring EventLogInjector::EVENT_SOURCE_NAME = L"VMHumanizer";

EventLogInjector::EventLogInjector(const json& config) : m_config(config) {
    if (config.contains("event_log")) {
        const auto& e = config["event_log"];
        m_cfg.enabled = e.value<bool>("enabled", true);
        m_cfg.maxEventsPerLog = e.value<int>("max_events_per_log", 10000);
        m_cfg.profileName = e.value<std::string>("profile_name", "developer");
    }

    m_computerName = WinAPI::getComputerName();
    m_userName = WinAPI::getCurrentUserName();
    m_userSID = WinAPI::getCurrentUserSID();

    loadTemplates();

    // Try to register event source (may need admin)
    WinAPI::registerEventSource(EVENT_SOURCE_NAME);

    logInfo("EventLogInjector initialized (profile=%s, maxEvents=%d)",
            m_cfg.profileName.c_str(), m_cfg.maxEventsPerLog);
}

void EventLogInjector::loadTemplates() {
    // Security events
    m_templates.push_back({
        4688, "Security", EVENTLOG_AUDIT_SUCCESS,
        L"A new process has been created.\n\nCreator Subject:\n\tSecurity ID:\t\t{sid}\n\tAccount Name:\t\t{user}\n\nProcess Information:\n\tNew Process Name:\t{process}\n\tCreator Process Name:\tC:\\Windows\\System32\\cmd.exe",
        "frequent", {L"notepad.exe", L"calc.exe", L"cmd.exe", L"explorer.exe", L"taskmgr.exe"}
    });

    m_templates.push_back({
        4624, "Security", EVENTLOG_AUDIT_SUCCESS,
        L"An account was successfully logged on.\n\nSubject:\n\tSecurity ID:\t\tSYSTEM\n\tAccount Name:\t\t{computer}$\n\nLogon Information:\n\tLogon Type:\t\t2\n\tAccount Name:\t\t{user}\n\tWorkstation Name:\t{computer}",
        "occasional", {}
    });

    m_templates.push_back({
        4634, "Security", EVENTLOG_AUDIT_SUCCESS,
        L"An account was logged off.\n\nSubject:\n\tSecurity ID:\t\t{sid}\n\tAccount Name:\t\t{user}\n\tLogon Type:\t\t2",
        "occasional", {}
    });

    // System events
    m_templates.push_back({
        6005, "EventLog", EVENTLOG_INFORMATION_TYPE,
        L"The Event log service was started.",
        "rare", {}
    });

    m_templates.push_back({
        6006, "EventLog", EVENTLOG_INFORMATION_TYPE,
        L"The Event log service was stopped.",
        "rare", {}
    });

    m_templates.push_back({
        7036, "Service Control Manager", EVENTLOG_INFORMATION_TYPE,
        L"The {service} service entered the running state.",
        "frequent", {}
    });

    m_templates.push_back({
        7040, "Service Control Manager", EVENTLOG_INFORMATION_TYPE,
        L"The start type of the {service} service was changed from demand start to auto start.",
        "rare", {}
    });

    // Application events
    m_templates.push_back({
        1000, "Application Error", EVENTLOG_ERROR_TYPE,
        L"Faulting application name: {process}, version: 10.0.19041.1, time stamp: 0x5f3e4a20\nFaulting module name: ntdll.dll\nException code: 0xc0000005",
        "rare", {L"explorer.exe", L"svchost.exe"}
    });

    m_templates.push_back({
        1001, "Windows Error Reporting", EVENTLOG_INFORMATION_TYPE,
        L"Fault bucket type: 0\nEvent Name: AppCrash\nResponse: Not available\nCab Id: 0\nProblem signature:\nP1: {process}\nP2: 10.0.19041.1",
        "rare", {L"notepad.exe", L"calc.exe"}
    });

    m_templates.push_back({
        63, "Microsoft-Windows-WMI", EVENTLOG_WARNING_TYPE,
        L"A provider, {process}, has been registered in the WMI namespace root\\cimv2 to use the LocalSystem account.",
        "rare", {}
    });

    // PowerShell events
    m_templates.push_back({
        400, "PowerShell", EVENTLOG_INFORMATION_TYPE,
        L"Engine state is changed from None to Available.\n\nDetails:\n\tNewEngineState=Available\n\tPreviousEngineState=None\n\tSequenceNumber=13\n\tHostName=ConsoleHost",
        "occasional", {}
    });

    m_templates.push_back({
        403, "PowerShell", EVENTLOG_INFORMATION_TYPE,
        L"Engine state is changed from Available to Stopped.\n\nDetails:\n\tNewEngineState=Stopped\n\tPreviousEngineState=Available\n\tSequenceNumber=15\n\tHostName=ConsoleHost",
        "occasional", {}
    });

    m_templates.push_back({
        4104, "Microsoft-Windows-PowerShell", EVENTLOG_INFORMATION_TYPE,
        L"Creating Scriptblock text (1 of 1):\nGet-Process | Where-Object {$_.CPU -gt 10} | Select-Object Name, CPU\n\nScriptBlock ID: {guid}",
        "occasional", {}
    });
}

std::wstring EventLogInjector::fillTemplate(const std::wstring& templ) {
    std::wstring result = templ;

    auto replace = [&](const std::wstring& token, const std::wstring& value) {
        size_t pos = 0;
        while ((pos = result.find(token, pos)) != std::wstring::npos) {
            result.replace(pos, token.size(), value);
            pos += value.size();
        }
    };

    replace(L"{user}", m_userName);
    replace(L"{computer}", m_computerName);
    replace(L"{sid}", m_userSID);

    // Generate a fake GUID
    wchar_t guid[64];
    swprintf(guid, 64, L"%08x-%04x-%04x-%04x-%012llx",
             randomInt(0, INT_MAX), randomInt(0, 0xFFFF), randomInt(0, 0xFFFF),
             randomInt(0, 0xFFFF), static_cast<long long>(randomInt(0, INT_MAX)) * randomInt(1, 1000));
    replace(L"{guid}", guid);

    // Services
    static const std::vector<std::wstring> services = {
        L"Windows Update", L"BITS", L"Windows Defender",
        L"Print Spooler", L"Windows Search", L"Themes",
        L"DHCP Client", L"DNS Client", L"Windows Firewall",
    };
    replace(L"{service}", services[randomInt(0, static_cast<int>(services.size()) - 1)]);

    return result;
}

SYSTEMTIME EventLogInjector::generateRealisticTime(int daysBack, int hourMin, int hourMax) {
    SYSTEMTIME st;
    GetLocalTime(&st);

    // Go back N days
    FILETIME ft;
    SystemTimeToFileTime(&st, &ft);
    ft = offsetFileTime(ft, -static_cast<int64_t>(daysBack) * 86400);
    FileTimeToSystemTime(&ft, &st);

    // Set realistic hour
    st.wHour = static_cast<WORD>(randomInt(hourMin, hourMax));
    st.wMinute = static_cast<WORD>(randomInt(0, 59));
    st.wSecond = static_cast<WORD>(randomInt(0, 59));
    st.wMilliseconds = static_cast<WORD>(randomInt(0, 999));

    return st;
}

void EventLogInjector::injectEvent(const std::string& logName,
                                    DWORD eventID,
                                    const std::wstring& message,
                                    WORD eventType) {
    if (!m_cfg.enabled) return;

    std::wstring source = EVENT_SOURCE_NAME;
    bool success = WinAPI::writeEventLog(source, eventType, eventID, message);

    if (success) {
        logDebug("Event injected: ID=%lu, Type=%d", eventID, eventType);
    } else {
        logWarning("Failed to inject event ID=%lu (error=%lu)", eventID, GetLastError());
    }
}

void EventLogInjector::generateSecurityEvents(int count) {
    std::vector<EventTemplate> securityTemplates;
    for (auto& t : m_templates) {
        if (t.source == "Security") securityTemplates.push_back(t);
    }
    if (securityTemplates.empty()) return;

    for (int i = 0; i < count; ++i) {
        auto& tmpl = securityTemplates[randomInt(0, static_cast<int>(securityTemplates.size()) - 1)];
        std::wstring msg = fillTemplate(tmpl.messageTemplate);

        if (!tmpl.processExamples.empty()) {
            auto& proc = tmpl.processExamples[randomInt(0, static_cast<int>(tmpl.processExamples.size()) - 1)];
            size_t pos = msg.find(L"{process}");
            if (pos != std::wstring::npos) {
                msg.replace(pos, 9, L"C:\\Windows\\System32\\" + proc);
            }
        }

        injectEvent("Security", tmpl.eventID, msg, tmpl.eventType);
        WinAPI::sleepMs(randomInt(50, 200));
    }
}

void EventLogInjector::generateSystemEvents(int count) {
    std::vector<EventTemplate> systemTemplates;
    for (auto& t : m_templates) {
        if (t.source == "EventLog" || t.source == "Service Control Manager")
            systemTemplates.push_back(t);
    }
    if (systemTemplates.empty()) return;

    for (int i = 0; i < count; ++i) {
        auto& tmpl = systemTemplates[randomInt(0, static_cast<int>(systemTemplates.size()) - 1)];
        std::wstring msg = fillTemplate(tmpl.messageTemplate);
        injectEvent("System", tmpl.eventID, msg, tmpl.eventType);
        WinAPI::sleepMs(randomInt(50, 200));
    }
}

void EventLogInjector::generateApplicationEvents(int count) {
    std::vector<EventTemplate> appTemplates;
    for (auto& t : m_templates) {
        if (t.source == "Application Error" || t.source == "Windows Error Reporting" ||
            t.source == "Microsoft-Windows-WMI")
            appTemplates.push_back(t);
    }
    if (appTemplates.empty()) return;

    for (int i = 0; i < count; ++i) {
        auto& tmpl = appTemplates[randomInt(0, static_cast<int>(appTemplates.size()) - 1)];
        std::wstring msg = fillTemplate(tmpl.messageTemplate);

        if (!tmpl.processExamples.empty()) {
            auto& proc = tmpl.processExamples[randomInt(0, static_cast<int>(tmpl.processExamples.size()) - 1)];
            size_t pos = msg.find(L"{process}");
            if (pos != std::wstring::npos) {
                msg.replace(pos, 9, proc);
            }
        }

        injectEvent("Application", tmpl.eventID, msg, tmpl.eventType);
        WinAPI::sleepMs(randomInt(100, 500));
    }
}

void EventLogInjector::generatePowerShellEvents(int count) {
    std::vector<EventTemplate> psTemplates;
    for (auto& t : m_templates) {
        if (t.source == "PowerShell" || t.source == "Microsoft-Windows-PowerShell")
            psTemplates.push_back(t);
    }
    if (psTemplates.empty()) return;

    for (int i = 0; i < count; ++i) {
        auto& tmpl = psTemplates[randomInt(0, static_cast<int>(psTemplates.size()) - 1)];
        std::wstring msg = fillTemplate(tmpl.messageTemplate);
        injectEvent("Application", tmpl.eventID, msg, tmpl.eventType);
        WinAPI::sleepMs(randomInt(100, 300));
    }
}

void EventLogInjector::populateEventsForProfile(const std::string& profileName, int count) {
    if (!m_cfg.enabled) {
        logWarning("Event log injection disabled");
        return;
    }

    logInfo("Populating events for profile '%s' (count=%d)", profileName.c_str(), count);

    // Distribute events by category based on profile
    int secCount, sysCount, appCount, psCount;

    if (profileName == "developer") {
        secCount = count * 30 / 100;
        sysCount = count * 20 / 100;
        appCount = count * 20 / 100;
        psCount  = count * 30 / 100;
    } else if (profileName == "office_worker") {
        secCount = count * 25 / 100;
        sysCount = count * 25 / 100;
        appCount = count * 40 / 100;
        psCount  = count * 10 / 100;
    } else { // student
        secCount = count * 20 / 100;
        sysCount = count * 30 / 100;
        appCount = count * 35 / 100;
        psCount  = count * 15 / 100;
    }

    generateSecurityEvents(secCount);
    generateSystemEvents(sysCount);
    generateApplicationEvents(appCount);
    generatePowerShellEvents(psCount);

    logInfo("Event population complete: sec=%d, sys=%d, app=%d, ps=%d",
            secCount, sysCount, appCount, psCount);
}

void EventLogInjector::injectBootSequence(const SYSTEMTIME& bootTime) {
    std::wstring bootMsg = L"The Event log service was started.";
    injectEvent("System", 6005, bootMsg, EVENTLOG_INFORMATION_TYPE);

    // Service starts after boot
    std::vector<std::wstring> bootServices = {
        L"Windows Defender", L"BITS", L"Windows Update",
        L"DHCP Client", L"DNS Client", L"Windows Search",
    };

    for (auto& svc : bootServices) {
        WinAPI::sleepMs(randomInt(100, 500));
        std::wstring msg = L"The " + svc + L" service entered the running state.";
        injectEvent("System", 7036, msg, EVENTLOG_INFORMATION_TYPE);
    }

    // Logon event
    WinAPI::sleepMs(randomInt(500, 2000));
    std::wstring logonMsg = fillTemplate(
        L"An account was successfully logged on.\n\nSubject:\n\tSecurity ID:\t\tSYSTEM\n\tAccount Name:\t\t{computer}$\n\nLogon Information:\n\tLogon Type:\t\t2\n\tAccount Name:\t\t{user}");
    injectEvent("Security", 4624, logonMsg, EVENTLOG_AUDIT_SUCCESS);

    logInfo("Boot sequence injected");
}

void EventLogInjector::injectShutdownSequence(const SYSTEMTIME& shutdownTime) {
    // Logoff
    std::wstring logoffMsg = fillTemplate(
        L"An account was logged off.\n\nSubject:\n\tSecurity ID:\t\t{sid}\n\tAccount Name:\t\t{user}\n\tLogon Type:\t\t2");
    injectEvent("Security", 4634, logoffMsg, EVENTLOG_AUDIT_SUCCESS);

    WinAPI::sleepMs(randomInt(500, 1500));

    std::wstring shutdownMsg = L"The Event log service was stopped.";
    injectEvent("System", 6006, shutdownMsg, EVENTLOG_INFORMATION_TYPE);

    logInfo("Shutdown sequence injected");
}

void EventLogInjector::clearAllEvents() {
    logWarning("clearAllEvents: Clearing event logs requires admin privileges and wevtutil.exe");
    // This would require running: wevtutil cl Application / wevtutil cl System etc.
    // Left as a controlled operation that the user should invoke manually
}

} // namespace vmh
