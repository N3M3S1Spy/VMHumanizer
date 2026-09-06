#include "debug_ui.h"
#include <conio.h>
#include <cstdio>
#include <algorithm>

namespace vmh {

DebugUI::DebugUI() {
    m_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
}

DebugUI::DebugUI(const json& config) : DebugUI() {
    initialize(config);
}

DebugUI::~DebugUI() {
    stop();
}

void DebugUI::initialize(const json& config) {
    if (config.contains("debug_ui")) {
        const auto& ui = config["debug_ui"];
        m_refreshRateMs = ui.value<int>("refresh_rate_ms", 500);
        m_maxEventFeedSize = ui.value<int>("max_event_feed_size", 10);
        m_autoScrollFeed = ui.value<bool>("auto_scroll_feed", true);
    }
}

void DebugUI::setCursorPosition(int x, int y) {
    COORD pos = {static_cast<SHORT>(x), static_cast<SHORT>(y)};
    SetConsoleCursorPosition(m_hConsole, pos);
}

void DebugUI::setTextColor(ConsoleColor color) {
    SetConsoleTextAttribute(m_hConsole, static_cast<WORD>(color));
}

void DebugUI::resetColor() {
    SetConsoleTextAttribute(m_hConsole, COLOR_WHITE);
}

void DebugUI::clearLine(int y) {
    setCursorPosition(0, y);
    std::string blank(m_width, ' ');
    printf("%s", blank.c_str());
    setCursorPosition(0, y);
}

std::string DebugUI::formatDuration(int seconds) const {
    int h = seconds / 3600;
    int m = (seconds % 3600) / 60;
    int s = seconds % 60;
    char buf[32];
    if (h > 0) snprintf(buf, sizeof(buf), "%dh %02dm %02ds", h, m, s);
    else if (m > 0) snprintf(buf, sizeof(buf), "%dm %02ds", m, s);
    else snprintf(buf, sizeof(buf), "%ds", s);
    return buf;
}

std::string DebugUI::makeProgressBar(int percent, int width) const {
    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;
    int filled = (percent * width) / 100;
    std::string bar(filled, '\xDB');
    bar += std::string(width - filled, '\xB0');
    return bar;
}

void DebugUI::drawHeader() {
    int y = 0;
    setTextColor(COLOR_CYAN);
    setCursorPosition(0, y);
    printf("\xC9");
    for (int i = 0; i < m_width - 2; i++) printf("\xCD");
    printf("\xBB");

    y++;
    setCursorPosition(0, y);
    printf("\xBA");
    setTextColor(COLOR_GREEN);
    std::string title = "VMHumanizer Activity Monitor";
    int pad = (m_width - 2 - static_cast<int>(title.size())) / 2;
    printf("%*s%s%*s", pad, "", title.c_str(),
           m_width - 2 - pad - static_cast<int>(title.size()), "");
    setTextColor(COLOR_CYAN);
    printf("\xBA");
}

void DebugUI::drawStatusBar() {
    int y = 2;
    setTextColor(COLOR_CYAN);
    setCursorPosition(0, y);
    printf("\xCC");
    for (int i = 0; i < m_width - 2; i++) printf("\xCD");
    printf("\xB9");

    y++;
    setCursorPosition(0, y);
    printf("\xBA ");
    setTextColor(COLOR_WHITE);
    printf("STATUS: ");
    setTextColor(m_paused ? COLOR_YELLOW : COLOR_GREEN);
    printf("%-10s ", (m_paused ? "Paused" : m_status.c_str()));
    setTextColor(COLOR_CYAN);
    std::string bar = makeProgressBar(m_progressPercent, 10);
    printf("[%s] %3d%%", bar.c_str(), m_progressPercent);
    int remain = m_width - 2 - 38;
    printf("%*s", remain > 0 ? remain : 0, "");
    setTextColor(COLOR_CYAN);
    printf("\xBA");

    y++;
    setCursorPosition(0, y);
    printf("\xBA ");
    setTextColor(COLOR_WHITE);
    printf("Current Activity: ");
    setTextColor(COLOR_GREEN);
    std::string actStr = m_currentActivity.empty() ? "None" : m_currentActivity;
    if (actStr.size() > 40) actStr = actStr.substr(0, 37) + "...";
    printf("%-40s ", actStr.c_str());
    int remain2 = m_width - 2 - 60;
    printf("%*s", remain2 > 0 ? remain2 : 0, "");
    setTextColor(COLOR_CYAN);
    printf("\xBA");

    y++;
    setCursorPosition(0, y);
    printf("\xBA ");
    setTextColor(COLOR_WHITE);
    printf("Duration: %-15s  Profile: %-20s",
           formatDuration(m_activityElapsedSec).c_str(),
           m_profileName.c_str());
    int remain3 = m_width - 2 - 58;
    printf("%*s", remain3 > 0 ? remain3 : 0, "");
    setTextColor(COLOR_CYAN);
    printf("\xBA");
}

void DebugUI::drawEventFeed() {
    int y = 6;
    setTextColor(COLOR_CYAN);
    setCursorPosition(0, y);
    printf("\xCC");
    for (int i = 0; i < m_width - 2; i++) printf("\xCD");
    printf("\xB9");

    y++;
    setCursorPosition(0, y);
    printf("\xBA ");
    setTextColor(COLOR_YELLOW);
    printf("RECENT EVENTS:");
    int pad = m_width - 2 - 15;
    printf("%*s", pad > 0 ? pad : 0, "");
    setTextColor(COLOR_CYAN);
    printf("\xBA");

    std::vector<EventEntry> events;
    {
        std::lock_guard<std::mutex> lock(m_eventMutex);
        int start = std::max(0, static_cast<int>(m_eventFeed.size()) - m_maxEventFeedSize - m_feedScrollOffset);
        int end = std::max(0, static_cast<int>(m_eventFeed.size()) - m_feedScrollOffset);
        for (int i = start; i < end && i < static_cast<int>(m_eventFeed.size()); ++i) {
            events.push_back(m_eventFeed[i]);
        }
    }

    int displayCount = std::min(static_cast<int>(events.size()), m_maxEventFeedSize);
    int blankLines = m_maxEventFeedSize - displayCount;

    for (int i = 0; i < displayCount; ++i) {
        y++;
        setCursorPosition(0, y);
        printf("\xBA ");
        setTextColor(COLOR_GRAY);
        printf("[%s] ", events[i].timestamp.c_str());
        setTextColor(events[i].color);
        std::string text = events[i].text;
        int maxTextLen = m_width - 2 - 12;
        if (static_cast<int>(text.size()) > maxTextLen) text = text.substr(0, maxTextLen - 3) + "...";
        printf("%-*s", maxTextLen, text.c_str());
        setTextColor(COLOR_CYAN);
        printf("\xBA");
    }

    for (int i = 0; i < blankLines; ++i) {
        y++;
        setCursorPosition(0, y);
        printf("\xBA ");
        printf("%*s", m_width - 3, "");
        printf("\xBA");
    }
}

void DebugUI::drawMetrics() {
    int y = 8 + m_maxEventFeedSize;
    setTextColor(COLOR_CYAN);
    setCursorPosition(0, y);
    printf("\xCC");
    for (int i = 0; i < m_width - 2; i++) printf("\xCD");
    printf("\xB9");

    y++;
    setCursorPosition(0, y);
    printf("\xBA ");
    setTextColor(COLOR_YELLOW);
    printf("PERFORMANCE:");
    int pad = m_width - 2 - 13;
    printf("%*s", pad > 0 ? pad : 0, "");
    setTextColor(COLOR_CYAN);
    printf("\xBA");

    y++;
    setCursorPosition(0, y);
    printf("\xBA ");
    setTextColor(COLOR_WHITE);
    printf("CPU: ");
    setTextColor(m_metrics.cpuUsagePercent > 50 ? COLOR_RED :
                (m_metrics.cpuUsagePercent > 20 ? COLOR_YELLOW : COLOR_GREEN));
    printf("%5.1f%%", m_metrics.cpuUsagePercent);
    setTextColor(COLOR_WHITE);
    printf(" | Memory: ");
    setTextColor(COLOR_CYAN);
    printf("%dMB", m_metrics.memoryUsageMb);
    setTextColor(COLOR_WHITE);
    printf(" | Threads: ");
    setTextColor(COLOR_CYAN);
    printf("%d", m_metrics.activeThreads);
    int remain = m_width - 2 - 50;
    printf("%*s", remain > 0 ? remain : 0, "");
    setTextColor(COLOR_CYAN);
    printf("\xBA");

    y++;
    setCursorPosition(0, y);
    printf("\xBA ");
    setTextColor(COLOR_WHITE);
    printf("Activity Queue: ");
    setTextColor(COLOR_CYAN);
    printf("%d pending", m_metrics.activityQueueLength);
    printf("   Avg Duration: ");
    printf("%.0fms", m_metrics.avgActivityDurationMs);
    int remain2 = m_width - 2 - 55;
    printf("%*s", remain2 > 0 ? remain2 : 0, "");
    setTextColor(COLOR_CYAN);
    printf("\xBA");
}

void DebugUI::drawActiveTasks() {
    int y = 12 + m_maxEventFeedSize;
    setTextColor(COLOR_CYAN);
    setCursorPosition(0, y);
    printf("\xCC");
    for (int i = 0; i < m_width - 2; i++) printf("\xCD");
    printf("\xB9");

    y++;
    setCursorPosition(0, y);
    printf("\xBA ");
    setTextColor(COLOR_YELLOW);
    printf("ACTIVE TASKS:");
    int pad = m_width - 2 - 14;
    printf("%*s", pad > 0 ? pad : 0, "");
    setTextColor(COLOR_CYAN);
    printf("\xBA");

    std::vector<ActiveTaskInfo> tasks;
    {
        std::lock_guard<std::mutex> lock(m_taskMutex);
        tasks = m_activeTasks;
    }

    int maxTasks = 3;
    int count = std::min(static_cast<int>(tasks.size()), maxTasks);
    for (int i = 0; i < count; ++i) {
        y++;
        setCursorPosition(0, y);
        printf("\xBA ");
        std::string bar = makeProgressBar(tasks[i].progressPercent, 10);
        printf("[%s] ", bar.c_str());
        setTextColor(tasks[i].completed ? COLOR_GREEN : COLOR_WHITE);
        std::string taskName = tasks[i].name;
        if (taskName.size() > 30) taskName = taskName.substr(0, 27) + "...";
        if (tasks[i].completed) {
            printf("%-30s - Complete", taskName.c_str());
        } else {
            printf("%-30s - %d%%", taskName.c_str(), tasks[i].progressPercent);
        }
        int remain = m_width - 2 - 56;
        printf("%*s", remain > 0 ? remain : 0, "");
        setTextColor(COLOR_CYAN);
        printf("\xBA");
    }

    for (int i = count; i < maxTasks; ++i) {
        y++;
        setCursorPosition(0, y);
        printf("\xBA ");
        printf("%*s", m_width - 3, "");
        printf("\xBA");
    }
}

void DebugUI::drawStats() {
    int y = 16 + m_maxEventFeedSize;
    setTextColor(COLOR_CYAN);
    setCursorPosition(0, y);
    printf("\xCC");
    for (int i = 0; i < m_width - 2; i++) printf("\xCD");
    printf("\xB9");

    y++;
    setCursorPosition(0, y);
    printf("\xBA ");
    setTextColor(COLOR_YELLOW);
    printf("STATS:");
    int pad = m_width - 2 - 7;
    printf("%*s", pad > 0 ? pad : 0, "");
    setTextColor(COLOR_CYAN);
    printf("\xBA");

    y++;
    setCursorPosition(0, y);
    printf("\xBA ");
    setTextColor(COLOR_WHITE);
    printf("Total Activities: ");
    setTextColor(COLOR_CYAN);
    printf("%-5d", m_totalActivities);
    setTextColor(COLOR_WHITE);
    printf(" | Errors: ");
    setTextColor(m_errorCount > 0 ? COLOR_RED : COLOR_GREEN);
    printf("%-3d", m_errorCount);
    setTextColor(COLOR_WHITE);
    printf(" | Uptime: ");
    setTextColor(COLOR_CYAN);
    printf("%s", formatDuration(m_uptimeSeconds).c_str());
    int remain = m_width - 2 - 55;
    printf("%*s", remain > 0 ? remain : 0, "");
    setTextColor(COLOR_CYAN);
    printf("\xBA");
}

void DebugUI::drawControlBar() {
    int y = 19 + m_maxEventFeedSize;
    setTextColor(COLOR_CYAN);
    setCursorPosition(0, y);
    printf("\xCC");
    for (int i = 0; i < m_width - 2; i++) printf("\xCD");
    printf("\xB9");

    y++;
    setCursorPosition(0, y);
    printf("\xBA ");
    setTextColor(COLOR_GREEN);
    printf("[P]");
    setTextColor(COLOR_WHITE);
    printf("ause  ");
    setTextColor(COLOR_GREEN);
    printf("[R]");
    setTextColor(COLOR_WHITE);
    printf("esume  ");
    setTextColor(COLOR_GREEN);
    printf("[S]");
    setTextColor(COLOR_WHITE);
    printf("kip  ");
    setTextColor(COLOR_GREEN);
    printf("[D]");
    setTextColor(COLOR_WHITE);
    printf("etails  ");
    setTextColor(COLOR_RED);
    printf("[Q]");
    setTextColor(COLOR_WHITE);
    printf("uit");
    int remain = m_width - 2 - 48;
    printf("%*s", remain > 0 ? remain : 0, "");
    setTextColor(COLOR_CYAN);
    printf("\xBA");

    y++;
    setCursorPosition(0, y);
    printf("\xC8");
    for (int i = 0; i < m_width - 2; i++) printf("\xCD");
    printf("\xBC");
}

void DebugUI::drawFrame() {
    drawHeader();
    drawStatusBar();
    drawEventFeed();
    drawMetrics();
    drawActiveTasks();
    drawStats();
    drawControlBar();
    resetColor();
}

void DebugUI::handleKeyInput(char key) {
    switch (key) {
        case 'q': case 'Q':
            m_running = false;
            break;
        case 'p': case 'P':
            m_paused = true;
            break;
        case 'r': case 'R':
            m_paused = false;
            break;
        default:
            break;
    }

    if (m_commandCallback) {
        m_commandCallback(key);
    }
}

void DebugUI::run() {
    m_running = true;

    // Hide cursor
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(m_hConsole, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(m_hConsole, &cursorInfo);

    // Clear screen
    COORD origin = {0, 0};
    DWORD written;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(m_hConsole, &csbi);
    DWORD cells = csbi.dwSize.X * csbi.dwSize.Y;
    FillConsoleOutputCharacterW(m_hConsole, L' ', cells, origin, &written);
    FillConsoleOutputAttribute(m_hConsole, csbi.wAttributes, cells, origin, &written);
    SetConsoleCursorPosition(m_hConsole, origin);

    while (m_running) {
        drawFrame();

        // Non-blocking input check
        for (int i = 0; i < m_refreshRateMs / 50; ++i) {
            if (_kbhit()) {
                char ch = static_cast<char>(_getch());
                handleKeyInput(ch);
                if (!m_running) break;
            }
            Sleep(50);
        }
    }

    // Restore cursor
    cursorInfo.bVisible = TRUE;
    SetConsoleCursorInfo(m_hConsole, &cursorInfo);
    resetColor();
}

void DebugUI::stop() {
    m_running = false;
}

void DebugUI::update() {
    if (m_running) drawFrame();
}

void DebugUI::addEvent(const std::string& timestamp, const std::string& event, ConsoleColor color) {
    std::lock_guard<std::mutex> lock(m_eventMutex);
    EventEntry entry{timestamp, event, color};
    m_eventFeed.push_back(entry);
    while (static_cast<int>(m_eventFeed.size()) > m_maxEventFeedSize * 10) {
        m_eventFeed.pop_front();
    }
    if (m_autoScrollFeed) m_feedScrollOffset = 0;
}

void DebugUI::setStatus(const std::string& status, int progressPercent) {
    m_status = status;
    m_progressPercent = progressPercent;
}

void DebugUI::setCurrentActivity(const std::string& name, int elapsedSec) {
    m_currentActivity = name;
    m_activityElapsedSec = elapsedSec;
}

void DebugUI::setProfileName(const std::string& name) {
    m_profileName = name;
}

void DebugUI::updateMetrics(const TelemetryPipeline::TelemetryStats& stats) {
    m_metrics = stats;
}

void DebugUI::setActiveTasks(const std::vector<ActiveTaskInfo>& tasks) {
    std::lock_guard<std::mutex> lock(m_taskMutex);
    m_activeTasks = tasks;
}

void DebugUI::setTotalActivities(int total) { m_totalActivities = total; }
void DebugUI::setErrorCount(int errors) { m_errorCount = errors; }
void DebugUI::setUptimeSeconds(int seconds) { m_uptimeSeconds = seconds; }

void DebugUI::setCommandCallback(CommandCallback cb) { m_commandCallback = std::move(cb); }

} // namespace vmh
