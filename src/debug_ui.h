#pragma once
#include "common/json.hpp"
#include "telemetry_pipeline.h"
#include <string>
#include <vector>
#include <mutex>
#include <atomic>
#include <deque>

namespace vmh {

enum ConsoleColor {
    COLOR_BLACK = 0,
    COLOR_DARK_RED = 4,
    COLOR_DARK_GREEN = 2,
    COLOR_DARK_YELLOW = 6,
    COLOR_DARK_CYAN = 3,
    COLOR_GRAY = 7,
    COLOR_GREEN = 10,
    COLOR_CYAN = 11,
    COLOR_RED = 12,
    COLOR_YELLOW = 14,
    COLOR_WHITE = 15
};

class DebugUI {
public:
    DebugUI();
    explicit DebugUI(const json& config);
    ~DebugUI();

    void initialize(const json& config);

    void run();
    void stop();
    void update();

    void addEvent(const std::string& timestamp, const std::string& event, ConsoleColor color = COLOR_WHITE);
    void setStatus(const std::string& status, int progressPercent);
    void setCurrentActivity(const std::string& name, int elapsedSec);
    void setProfileName(const std::string& name);
    void updateMetrics(const TelemetryPipeline::TelemetryStats& stats);

    struct ActiveTaskInfo {
        std::string name;
        int progressPercent;
        bool completed;
    };
    void setActiveTasks(const std::vector<ActiveTaskInfo>& tasks);

    void setTotalActivities(int total);
    void setErrorCount(int errors);
    void setUptimeSeconds(int seconds);

    using CommandCallback = std::function<void(char)>;
    void setCommandCallback(CommandCallback cb);

    bool isRunning() const { return m_running; }

private:
    void drawFrame();
    void drawHeader();
    void drawStatusBar();
    void drawEventFeed();
    void drawMetrics();
    void drawActiveTasks();
    void drawStats();
    void drawControlBar();

    void handleKeyInput(char key);
    void setCursorPosition(int x, int y);
    void setTextColor(ConsoleColor color);
    void resetColor();
    void drawBox(int x, int y, int width, int height);
    void drawHLine(int x, int y, int width);
    void clearLine(int y);
    std::string formatDuration(int seconds) const;
    std::string makeProgressBar(int percent, int width) const;

    HANDLE m_hConsole = INVALID_HANDLE_VALUE;
    int m_width = 72;
    int m_refreshRateMs = 500;
    int m_maxEventFeedSize = 10;
    bool m_autoScrollFeed = true;
    std::atomic<bool> m_running{false};
    bool m_paused = false;

    std::string m_status = "Initializing";
    int m_progressPercent = 0;
    std::string m_currentActivity;
    int m_activityElapsedSec = 0;
    std::string m_profileName;

    TelemetryPipeline::TelemetryStats m_metrics;

    struct EventEntry {
        std::string timestamp;
        std::string text;
        ConsoleColor color;
    };
    std::mutex m_eventMutex;
    std::deque<EventEntry> m_eventFeed;

    std::mutex m_taskMutex;
    std::vector<ActiveTaskInfo> m_activeTasks;

    int m_totalActivities = 0;
    int m_errorCount = 0;
    int m_uptimeSeconds = 0;

    CommandCallback m_commandCallback;
    int m_feedScrollOffset = 0;
};

} // namespace vmh
