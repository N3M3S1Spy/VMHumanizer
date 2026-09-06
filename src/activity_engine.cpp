#include "activity_engine.h"
#include "mouse_behavior.h"
#include "keystroke_emulator.h"
#include "common/utils.h"
#include "windows/winapi_wrapper.h"
#include <chrono>
#include <ctime>

namespace vmh {

ActivityEngine::ActivityEngine(const json& config) : m_config(config) {
    if (config.contains("activity_engine")) {
        const auto& ae = config["activity_engine"];
        m_profileName = ae.value<std::string>("profile", "developer");
        m_timezoneOffset = ae.value<int>("timezone_offset", 1);
        m_breakFrequencyHours = ae.value<double>("break_frequency_hours", 1.5);
        m_distractionProbability = ae.value<double>("distraction_probability", 0.15);
        m_activityIntensity = ae.value<double>("activity_intensity", 0.8);
        m_logActivities = ae.value<bool>("log_activities", true);
    }

    // Load activity templates
    m_activityTemplates = json::object();

    // Developer activities
    auto devMorning = json::object();
    devMorning["name"] = "Morning Routine";
    auto devMorningTasks = json::array();
    {
        auto t1 = json::object(); t1["app"] = "msedge.exe"; t1["action"] = "browse_github"; t1["duration_min"] = 300; t1["duration_max"] = 600;
        auto t2 = json::object(); t2["app"] = "cmd.exe"; t2["action"] = "git_pull"; t2["duration_min"] = 120; t2["duration_max"] = 180;
        auto t3 = json::object(); t3["app"] = "code.exe"; t3["action"] = "code_review"; t3["duration_min"] = 1800; t3["duration_max"] = 3600;
        devMorningTasks.push_back(t1); devMorningTasks.push_back(t2); devMorningTasks.push_back(t3);
    }
    devMorning["tasks"] = devMorningTasks;
    devMorning["time_start"] = 8; devMorning["time_end"] = 12;

    auto devAfternoon = json::object();
    devAfternoon["name"] = "Afternoon Coding";
    auto devAfternoonTasks = json::array();
    {
        auto t1 = json::object(); t1["app"] = "code.exe"; t1["action"] = "development"; t1["duration_min"] = 3600; t1["duration_max"] = 7200;
        auto t2 = json::object(); t2["app"] = "msedge.exe"; t2["action"] = "stackoverflow_search"; t2["duration_min"] = 300; t2["duration_max"] = 900;
        devAfternoonTasks.push_back(t1); devAfternoonTasks.push_back(t2);
    }
    devAfternoon["tasks"] = devAfternoonTasks;
    devAfternoon["time_start"] = 13; devAfternoon["time_end"] = 18;

    auto devEvening = json::object();
    devEvening["name"] = "Evening Browse";
    auto devEveningTasks = json::array();
    {
        auto t1 = json::object(); t1["app"] = "msedge.exe"; t1["action"] = "tech_news"; t1["duration_min"] = 600; t1["duration_max"] = 1800;
        devEveningTasks.push_back(t1);
    }
    devEvening["tasks"] = devEveningTasks;
    devEvening["time_start"] = 18; devEvening["time_end"] = 22;

    auto devActivities = json::array();
    devActivities.push_back(devMorning);
    devActivities.push_back(devAfternoon);
    devActivities.push_back(devEvening);
    m_activityTemplates["developer"] = devActivities;

    // Office worker activities
    auto offMorning = json::object();
    offMorning["name"] = "Email & Planning";
    auto offMorningTasks = json::array();
    {
        auto t1 = json::object(); t1["app"] = "OUTLOOK.EXE"; t1["action"] = "check_email"; t1["duration_min"] = 600; t1["duration_max"] = 1200;
        auto t2 = json::object(); t2["app"] = "EXCEL.EXE"; t2["action"] = "update_spreadsheet"; t2["duration_min"] = 1800; t2["duration_max"] = 3600;
        offMorningTasks.push_back(t1); offMorningTasks.push_back(t2);
    }
    offMorning["tasks"] = offMorningTasks;
    offMorning["time_start"] = 8; offMorning["time_end"] = 12;

    auto offAfternoon = json::object();
    offAfternoon["name"] = "Document Work";
    auto offAfternoonTasks = json::array();
    {
        auto t1 = json::object(); t1["app"] = "WINWORD.EXE"; t1["action"] = "write_report"; t1["duration_min"] = 2400; t1["duration_max"] = 5400;
        auto t2 = json::object(); t2["app"] = "POWERPNT.EXE"; t2["action"] = "edit_presentation"; t2["duration_min"] = 1200; t2["duration_max"] = 3000;
        offAfternoonTasks.push_back(t1); offAfternoonTasks.push_back(t2);
    }
    offAfternoon["tasks"] = offAfternoonTasks;
    offAfternoon["time_start"] = 13; offAfternoon["time_end"] = 17;

    auto offActivities = json::array();
    offActivities.push_back(offMorning);
    offActivities.push_back(offAfternoon);
    m_activityTemplates["office_worker"] = offActivities;

    // Student activities
    auto stuMorning = json::object();
    stuMorning["name"] = "Study Session";
    auto stuMorningTasks = json::array();
    {
        auto t1 = json::object(); t1["app"] = "msedge.exe"; t1["action"] = "research"; t1["duration_min"] = 1200; t1["duration_max"] = 3600;
        auto t2 = json::object(); t2["app"] = "WINWORD.EXE"; t2["action"] = "write_assignment"; t2["duration_min"] = 1800; t2["duration_max"] = 5400;
        stuMorningTasks.push_back(t1); stuMorningTasks.push_back(t2);
    }
    stuMorning["tasks"] = stuMorningTasks;
    stuMorning["time_start"] = 9; stuMorning["time_end"] = 13;

    auto stuActivities = json::array();
    stuActivities.push_back(stuMorning);
    m_activityTemplates["student"] = stuActivities;

    logInfo("ActivityEngine initialized (profile=%s, intensity=%.2f)", m_profileName.c_str(), m_activityIntensity);
}

ActivityEngine::~ActivityEngine() {
    stop();
}

std::string ActivityEngine::getStateString() const {
    switch (m_state.load()) {
    case State::IDLE:      return "IDLE";
    case State::ACTIVE:    return "ACTIVE";
    case State::SWITCHING: return "SWITCHING";
    case State::BREAK:     return "BREAK";
    case State::WAITING:   return "WAITING";
    }
    return "UNKNOWN";
}

void ActivityEngine::start() {
    if (m_running) return;
    m_running = true;
    m_paused = false;
    m_sessionTimer.start();
    m_thread = std::thread(&ActivityEngine::stateLoop, this);
    logInfo("ActivityEngine started");
}

void ActivityEngine::stop() {
    m_running = false;
    m_paused = false;
    if (m_thread.joinable()) {
        m_thread.join();
    }
    m_sessionTimer.stop();
    m_state = State::IDLE;
    logInfo("ActivityEngine stopped (total activities=%d, breaks=%d)", m_totalActivities, m_totalBreaks);
}

void ActivityEngine::pause() {
    m_paused = true;
    m_state = State::WAITING;
    logInfo("ActivityEngine paused");
}

void ActivityEngine::resume() {
    m_paused = false;
    logInfo("ActivityEngine resumed");
}

double ActivityEngine::getIntensityForHour(int hour) {
    if (hour >= 6 && hour < 9) return 0.7;     // Morning ramp-up
    if (hour >= 9 && hour < 12) return 1.0;    // Peak morning
    if (hour >= 12 && hour < 13) return 0.3;   // Lunch
    if (hour >= 13 && hour < 15) return 0.7;   // Post-lunch slump
    if (hour >= 15 && hour < 17) return 0.9;   // Afternoon recovery
    if (hour >= 17 && hour < 20) return 0.5;   // Evening wind-down
    if (hour >= 20 && hour < 23) return 0.3;   // Late evening
    return 0.1;                                 // Night
}

json ActivityEngine::getActivitiesForTimeWindow(int currentHour) {
    std::string profile = m_profileName;
    if (!m_activityTemplates.contains(profile)) {
        profile = "developer";
    }

    const auto& activities = m_activityTemplates[profile];
    for (size_t i = 0; i < activities.size(); ++i) {
        int start = static_cast<int>(activities[i]["time_start"].getInt(0));
        int end = static_cast<int>(activities[i]["time_end"].getInt(24));
        if (currentHour >= start && currentHour < end) {
            return activities[i];
        }
    }
    return json();
}

void ActivityEngine::executeTask(const json& task) {
    std::string app = task.value<std::string>("app", "notepad.exe");
    std::string action = task.value<std::string>("action", "idle");
    int durationMin = task.value<int>("duration_min", 300);
    int durationMax = task.value<int>("duration_max", 600);

    // Add variation
    int duration = randomInt(durationMin, durationMax);
    double variation = randomDouble(0.8, 1.2);
    duration = static_cast<int>(duration * variation * m_activityIntensity);

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_currentApp = app;
        m_currentAction = action;
    }
    m_state = State::ACTIVE;
    m_activityTimer.start();

    logInfo("Executing task: %s -> %s (duration=%ds)", app.c_str(), action.c_str(), duration);

    // Simulate the activity duration with periodic mouse/keystroke simulation
    int elapsed = 0;
    int stepMs = 5000; // Check every 5 seconds

    while (elapsed < duration * 1000 && m_running && !m_paused) {
        WinAPI::sleepMs(stepMs);
        elapsed += stepMs;

        // Occasional mouse movement during activity
        if (m_mouse && randomDouble(0, 1) < 0.3) {
            int x = randomInt(100, 1800);
            int y = randomInt(100, 1000);
            m_mouse->moveTo(x, y, randomInt(300, 800));
        }

        // Random distraction check
        if (randomDouble(0, 1) < m_distractionProbability * 0.01) {
            logDebug("Distraction event during %s", action.c_str());
            WinAPI::sleepMs(randomInt(2000, 8000));
        }
    }

    m_activityTimer.stop();
    m_totalActivities++;

    logActivity(app, action, elapsed, true);
    logInfo("Task completed: %s -> %s (actual=%dms)", app.c_str(), action.c_str(), elapsed);
}

void ActivityEngine::injectBreak(const std::string& breakType) {
    m_state = State::BREAK;
    int breakDuration;

    if (breakType == "coffee") {
        breakDuration = randomInt(300, 900) * 1000; // 5-15 min
        logInfo("Coffee break (%dms)", breakDuration);
    } else if (breakType == "lunch") {
        breakDuration = randomInt(1800, 3600) * 1000; // 30-60 min
        logInfo("Lunch break (%dms)", breakDuration);
    } else if (breakType == "bio") {
        breakDuration = randomInt(120, 300) * 1000; // 2-5 min
        logInfo("Bio break (%dms)", breakDuration);
    } else {
        breakDuration = randomInt(60, 300) * 1000;
        logInfo("Short break (%dms)", breakDuration);
    }

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_currentApp = "";
        m_currentAction = "break_" + breakType;
    }

    int elapsed = 0;
    while (elapsed < breakDuration && m_running && !m_paused) {
        WinAPI::sleepMs(1000);
        elapsed += 1000;
    }

    m_totalBreaks++;
    logActivity("", "break_" + breakType, breakDuration, true);
}

void ActivityEngine::stateLoop() {
    Timer breakTimer;
    breakTimer.start();

    while (m_running) {
        if (m_paused) {
            m_state = State::WAITING;
            WinAPI::sleepMs(1000);
            continue;
        }

        int currentHour = Timer::getCurrentHour();
        double intensity = getIntensityForHour(currentHour);

        // Night hours — minimal activity
        if (intensity < 0.2) {
            m_state = State::IDLE;
            WinAPI::sleepMs(randomInt(30000, 120000));
            continue;
        }

        // Check if it's break time
        if (breakTimer.elapsedSeconds() > m_breakFrequencyHours * 3600) {
            breakTimer.reset();
            breakTimer.start();

            // Choose break type based on time of day
            if (currentHour >= 11 && currentHour <= 13 && randomDouble(0, 1) < 0.5) {
                injectBreak("lunch");
            } else if (randomDouble(0, 1) < 0.6) {
                injectBreak("coffee");
            } else {
                injectBreak("bio");
            }
            continue;
        }

        // Get activities for current time window
        json activities = getActivitiesForTimeWindow(currentHour);
        if (activities.isNull() || !activities.contains("tasks")) {
            m_state = State::IDLE;
            WinAPI::sleepMs(randomInt(10000, 60000));
            continue;
        }

        // Task switching pause
        m_state = State::SWITCHING;
        WinAPI::sleepMs(randomInt(1000, 5000));

        // Select and execute a random task from current window
        const auto& tasks = activities["tasks"];
        if (tasks.size() > 0) {
            int idx = randomInt(0, static_cast<int>(tasks.size()) - 1);
            executeTask(tasks[idx]);
        }

        // Inter-task pause
        WinAPI::sleepMs(randomInt(2000, 10000));
    }
}

void ActivityEngine::executeActivity(const std::string& activityName) {
    auto task = json::object();
    task["app"] = "manual";
    task["action"] = activityName;
    task["duration_min"] = 60;
    task["duration_max"] = 300;
    executeTask(task);
}

ActivityEngine::ActivityStats ActivityEngine::getStats() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return {
        m_currentApp,
        static_cast<int>(m_activityTimer.elapsedSeconds()),
        m_currentAction,
        m_totalActivities,
        m_totalBreaks
    };
}

void ActivityEngine::logActivity(const std::string& app, const std::string& action, int durationMs, bool success) {
    if (!m_logActivities) return;

    SYSTEMTIME st;
    GetLocalTime(&st);
    char ts[64];
    snprintf(ts, sizeof(ts), "%04d-%02d-%02dT%02d:%02d:%02d",
             st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

    std::lock_guard<std::mutex> lock(m_mutex);
    m_activityLog.push_back({ts, app, action, durationMs, success});

    // Keep log bounded
    if (m_activityLog.size() > 10000) {
        m_activityLog.erase(m_activityLog.begin(), m_activityLog.begin() + 5000);
    }
}

std::vector<ActivityEngine::ActivityLogEntry> ActivityEngine::getActivityLog() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_activityLog;
}

} // namespace vmh
