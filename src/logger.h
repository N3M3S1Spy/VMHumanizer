#pragma once
#include "common/json.hpp"
#include <string>
#include <queue>
#include <mutex>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <functional>

namespace vmh {

class Logger {
public:
    static Logger& getInstance();

    void initialize(const json& config);
    void shutdown();

    void logDebug(const std::string& component, const std::string& message);
    void logInfo(const std::string& component, const std::string& message);
    void logWarn(const std::string& component, const std::string& message);
    void logError(const std::string& component, const std::string& message);

    void logEvent(const json& eventData);
    void logException(const std::string& exceptionType, const std::string& message,
                      const std::string& context);

    void flush();

    struct LogStats {
        int totalEvents;
        int errors;
        int warnings;
        int queuedEvents;
    };
    LogStats getStats() const;

    using EventCallback = std::function<void(const json&)>;
    void setEventCallback(EventCallback cb);

    std::string getActivitiesLogPath() const { return m_activitiesPath; }
    std::string getMetricsLogPath() const { return m_metricsPath; }
    std::string getAnomaliesLogPath() const { return m_anomaliesPath; }

private:
    Logger() = default;
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void logImpl(const std::string& level, const std::string& component,
                 const std::string& message);
    void writerThread();
    void rotateLogIfNeeded(const std::string& filePath);
    void writeToFile(const std::string& filePath, const std::string& data);

    std::string m_activitiesPath = "data/logs/activities.log";
    std::string m_metricsPath = "data/logs/metrics.log";
    std::string m_anomaliesPath = "data/logs/anomalies.log";

    std::string m_logLevel = "INFO";
    int m_maxFileSizeMb = 100;
    int m_retentionDays = 30;
    bool m_asyncWrites = true;
    int m_queueSize = 1000;

    std::queue<std::pair<std::string, std::string>> m_writeQueue;
    std::mutex m_queueMutex;
    std::condition_variable m_queueCV;
    std::thread m_writerThread;
    std::atomic<bool> m_running{false};
    bool m_initialized = false;

    std::atomic<int> m_totalEvents{0};
    std::atomic<int> m_errorCount{0};
    std::atomic<int> m_warnCount{0};

    EventCallback m_eventCallback;
    std::mutex m_callbackMutex;
};

} // namespace vmh
