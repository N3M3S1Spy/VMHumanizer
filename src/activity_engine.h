#pragma once
#include "common/json.hpp"
#include "common/types.h"
#include "common/timer_utils.h"
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <functional>

namespace vmh {

class MouseBehavior;
class KeystrokeEmulator;

class ActivityEngine {
public:
    enum class State { IDLE, ACTIVE, SWITCHING, BREAK, WAITING };

    ActivityEngine(const json& config);
    ~ActivityEngine();

    void start();
    void stop();
    void pause();
    void resume();

    State getCurrentState() const { return m_state; }
    std::string getStateString() const;

    void executeActivity(const std::string& activityName);

    struct ActivityStats {
        std::string currentApp;
        int elapsedSeconds;
        std::string currentAction;
        int totalActivities;
        int totalBreaks;
    };
    ActivityStats getStats() const;

    struct ActivityLogEntry {
        std::string timestamp;
        std::string app;
        std::string action;
        int durationMs;
        bool success;
    };
    std::vector<ActivityLogEntry> getActivityLog() const;

    void setMouseBehavior(MouseBehavior* mouse) { m_mouse = mouse; }
    void setKeystrokeEmulator(KeystrokeEmulator* keystroke) { m_keystroke = keystroke; }

private:
    void stateLoop();
    void selectNextActivity();
    void executeTask(const json& task);
    void injectBreak(const std::string& breakType);
    json getActivitiesForTimeWindow(int currentHour);
    void logActivity(const std::string& app, const std::string& action, int durationMs, bool success);
    double getIntensityForHour(int hour);

    json m_config;
    std::atomic<State> m_state{State::IDLE};
    std::atomic<bool> m_running{false};
    std::atomic<bool> m_paused{false};
    std::thread m_thread;
    mutable std::mutex m_mutex;

    MouseBehavior* m_mouse = nullptr;
    KeystrokeEmulator* m_keystroke = nullptr;

    std::string m_profileName;
    double m_breakFrequencyHours = 1.5;
    double m_distractionProbability = 0.15;
    double m_activityIntensity = 0.8;
    bool m_logActivities = true;
    int m_timezoneOffset = 1;

    Timer m_activityTimer;
    Timer m_sessionTimer;
    std::string m_currentApp;
    std::string m_currentAction;
    int m_totalActivities = 0;
    int m_totalBreaks = 0;

    std::vector<ActivityLogEntry> m_activityLog;
    json m_activityTemplates;
};

} // namespace vmh
