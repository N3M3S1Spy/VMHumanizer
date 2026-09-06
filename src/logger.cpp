#include "logger.h"
#include "common/json_utils.h"
#include <filesystem>
#include <fstream>
#include <cstdio>

namespace vmh {

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

Logger::~Logger() {
    shutdown();
}

void Logger::initialize(const json& config) {
    if (m_initialized) return;

    if (config.contains("logging")) {
        const auto& lc = config["logging"];
        m_logLevel = lc.value<std::string>("log_level", "INFO");
        m_asyncWrites = lc.value<bool>("async_writes", true);
        m_queueSize = lc.value<int>("queue_size", 1000);

        if (lc.contains("files") && lc["files"].isObject()) {
            m_activitiesPath = lc["files"].value<std::string>("activities", m_activitiesPath);
            m_metricsPath = lc["files"].value<std::string>("metrics", m_metricsPath);
            m_anomaliesPath = lc["files"].value<std::string>("anomalies", m_anomaliesPath);
        }

        if (lc.contains("rotation") && lc["rotation"].isObject()) {
            m_maxFileSizeMb = lc["rotation"].value<int>("max_file_size_mb", 100);
            m_retentionDays = lc["rotation"].value<int>("retention_days", 30);
        }
    }

    // Ensure log directories exist
    for (auto& path : {m_activitiesPath, m_metricsPath, m_anomaliesPath}) {
        std::filesystem::create_directories(std::filesystem::path(path).parent_path());
    }

    if (m_asyncWrites) {
        m_running = true;
        m_writerThread = std::thread(&Logger::writerThread, this);
    }

    m_initialized = true;
}

void Logger::shutdown() {
    if (!m_initialized) return;
    m_running = false;
    m_queueCV.notify_all();
    if (m_writerThread.joinable()) {
        m_writerThread.join();
    }
    flush();
    m_initialized = false;
}

bool shouldLog(const std::string& level, const std::string& minLevel) {
    static const std::map<std::string, int> levels = {
        {"DEBUG", 0}, {"INFO", 1}, {"WARN", 2}, {"ERROR", 3}
    };
    auto itLevel = levels.find(level);
    auto itMin = levels.find(minLevel);
    if (itLevel == levels.end() || itMin == levels.end()) return true;
    return itLevel->second >= itMin->second;
}

void Logger::logImpl(const std::string& level, const std::string& component,
                     const std::string& message) {
    if (!shouldLog(level, m_logLevel)) return;

    auto entry = JsonUtils::createLogEntry(level, component, message);
    std::string line = entry.dump(-1) + "\n";

    m_totalEvents++;
    if (level == "ERROR") m_errorCount++;
    if (level == "WARN") m_warnCount++;

    // Notify callback
    {
        std::lock_guard<std::mutex> lock(m_callbackMutex);
        if (m_eventCallback) m_eventCallback(entry);
    }

    if (m_asyncWrites && m_running) {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        if (static_cast<int>(m_writeQueue.size()) < m_queueSize) {
            m_writeQueue.push({m_activitiesPath, line});
            m_queueCV.notify_one();
        }
    } else {
        writeToFile(m_activitiesPath, line);
    }
}

void Logger::logDebug(const std::string& component, const std::string& message) {
    logImpl("DEBUG", component, message);
}

void Logger::logInfo(const std::string& component, const std::string& message) {
    logImpl("INFO", component, message);
}

void Logger::logWarn(const std::string& component, const std::string& message) {
    logImpl("WARN", component, message);
}

void Logger::logError(const std::string& component, const std::string& message) {
    logImpl("ERROR", component, message);
}

void Logger::logEvent(const json& eventData) {
    std::string line = eventData.dump(-1) + "\n";
    m_totalEvents++;

    {
        std::lock_guard<std::mutex> lock(m_callbackMutex);
        if (m_eventCallback) m_eventCallback(eventData);
    }

    if (m_asyncWrites && m_running) {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        if (static_cast<int>(m_writeQueue.size()) < m_queueSize) {
            m_writeQueue.push({m_activitiesPath, line});
            m_queueCV.notify_one();
        }
    } else {
        writeToFile(m_activitiesPath, line);
    }
}

void Logger::logException(const std::string& exceptionType, const std::string& message,
                           const std::string& context) {
    auto entry = json::object();
    entry["timestamp"] = JsonUtils::currentTimestampISO();
    entry["level"] = "ERROR";
    entry["component"] = "exception_handler";
    entry["event_type"] = "exception";
    entry["exception_type"] = exceptionType;
    entry["message"] = message;
    entry["context"] = context;

    m_totalEvents++;
    m_errorCount++;

    std::string line = entry.dump(-1) + "\n";
    writeToFile(m_activitiesPath, line);
}

void Logger::flush() {
    std::lock_guard<std::mutex> lock(m_queueMutex);
    while (!m_writeQueue.empty()) {
        auto [path, data] = m_writeQueue.front();
        m_writeQueue.pop();
        writeToFile(path, data);
    }
}

void Logger::writerThread() {
    while (m_running) {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        m_queueCV.wait_for(lock, std::chrono::milliseconds(100), [this] {
            return !m_writeQueue.empty() || !m_running;
        });

        // Drain batch
        std::vector<std::pair<std::string, std::string>> batch;
        while (!m_writeQueue.empty() && batch.size() < 100) {
            batch.push_back(m_writeQueue.front());
            m_writeQueue.pop();
        }
        lock.unlock();

        // Group by file path and write
        std::map<std::string, std::string> grouped;
        for (auto& [path, data] : batch) {
            grouped[path] += data;
        }
        for (auto& [path, data] : grouped) {
            rotateLogIfNeeded(path);
            writeToFile(path, data);
        }
    }
}

void Logger::writeToFile(const std::string& filePath, const std::string& data) {
    try {
        std::ofstream f(filePath, std::ios::app);
        if (f.is_open()) {
            f << data;
        }
    } catch (...) {}
}

void Logger::rotateLogIfNeeded(const std::string& filePath) {
    try {
        if (!std::filesystem::exists(filePath)) return;
        auto fileSize = std::filesystem::file_size(filePath);
        if (fileSize < static_cast<uintmax_t>(m_maxFileSizeMb) * 1024 * 1024) return;

        // Rotate: rename to timestamped archive
        SYSTEMTIME st;
        GetLocalTime(&st);
        char archiveName[256];
        snprintf(archiveName, sizeof(archiveName), "%s.%04d%02d%02d_%02d%02d%02d",
                 filePath.c_str(), st.wYear, st.wMonth, st.wDay,
                 st.wHour, st.wMinute, st.wSecond);

        std::filesystem::rename(filePath, archiveName);
    } catch (...) {}
}

Logger::LogStats Logger::getStats() const {
    int queued = 0;
    {
        // Can't lock const method easily, approximate
        queued = 0;
    }
    return {
        m_totalEvents.load(),
        m_errorCount.load(),
        m_warnCount.load(),
        queued
    };
}

void Logger::setEventCallback(EventCallback cb) {
    std::lock_guard<std::mutex> lock(m_callbackMutex);
    m_eventCallback = std::move(cb);
}

} // namespace vmh
