#pragma once
#include "common/json.hpp"
#include "common/types.h"
#include <string>
#include <vector>
#include <windows.h>

namespace vmh {

class EventLogInjector {
public:
    EventLogInjector(const json& config);

    void injectEvent(const std::string& logName,
                     DWORD eventID,
                     const std::wstring& message,
                     WORD eventType = EVENTLOG_INFORMATION_TYPE);

    void populateEventsForProfile(const std::string& profileName, int count = 50);

    void injectBootSequence(const SYSTEMTIME& bootTime);
    void injectShutdownSequence(const SYSTEMTIME& shutdownTime);

    void clearAllEvents();

private:
    struct EventTemplate {
        DWORD eventID;
        std::string source;
        WORD eventType;
        std::wstring messageTemplate;
        std::string frequency;
        std::vector<std::wstring> processExamples;
    };

    void loadTemplates();
    void generateSecurityEvents(int count);
    void generateSystemEvents(int count);
    void generateApplicationEvents(int count);
    void generatePowerShellEvents(int count);
    std::wstring fillTemplate(const std::wstring& templ);
    SYSTEMTIME generateRealisticTime(int daysBack, int hourMin = 6, int hourMax = 23);

    EventLogConfig m_cfg;
    json m_config;
    std::vector<EventTemplate> m_templates;
    std::wstring m_computerName;
    std::wstring m_userName;
    std::wstring m_userSID;

    static const std::wstring EVENT_SOURCE_NAME;
};

} // namespace vmh
