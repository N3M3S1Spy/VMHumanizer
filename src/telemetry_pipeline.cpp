#include "telemetry_pipeline.h"
#include "logger.h"
#include "common/json_utils.h"

namespace vmh {

TelemetryPipeline::TelemetryPipeline() = default;

TelemetryPipeline::~TelemetryPipeline() {
    stopMetricsCollection();
}

void TelemetryPipeline::initialize(const json& config) {
    if (m_initialized) return;
    if (config.contains("telemetry")) {
        m_collectionIntervalSec = config["telemetry"].value<int>("collection_interval_sec", 60);
    }
    m_initialized = true;
}

void TelemetryPipeline::startMetricsCollection() {
    if (m_running) return;
    m_running = true;
    m_metricsThread = std::thread(&TelemetryPipeline::metricsThread, this);
}

void TelemetryPipeline::stopMetricsCollection() {
    m_running = false;
    if (m_metricsThread.joinable()) {
        m_metricsThread.join();
    }
}

void TelemetryPipeline::recordSystemMetrics() {
    auto& metrics = MetricsCollector::getInstance();
    auto sm = metrics.collectMetrics();

    auto entry = JsonUtils::createMetricsEntry(sm.cpuPercent, sm.memoryMb, sm.activeThreads);

    {
        std::lock_guard<std::mutex> lock(m_statsMutex);
        m_stats.cpuUsagePercent = sm.cpuPercent;
        m_stats.memoryUsageMb = sm.memoryMb;
        m_stats.activeThreads = sm.activeThreads;
    }

    JsonUtils::appendJsonLine(Logger::getInstance().getMetricsLogPath(), entry);
}

void TelemetryPipeline::recordActivityStart(const std::string& activityName,
                                             const std::string& activityChainId) {
    ActiveActivity aa;
    aa.name = activityName;
    aa.chainId = activityChainId;
    aa.startTime = std::chrono::steady_clock::now();

    {
        std::lock_guard<std::mutex> lock(m_activitiesMutex);
        m_activeActivities.push_back(aa);
    }

    auto entry = json::object();
    entry["timestamp"] = JsonUtils::currentTimestampISO();
    entry["level"] = "INFO";
    entry["component"] = "telemetry";
    entry["event_type"] = "activity_started";
    entry["activity_name"] = activityName;
    entry["activity_chain_id"] = activityChainId;

    Logger::getInstance().logEvent(entry);
}

void TelemetryPipeline::recordActivityEnd(const std::string& activityName,
                                           const std::string& activityChainId, bool success) {
    double durationMs = 0.0;

    {
        std::lock_guard<std::mutex> lock(m_activitiesMutex);
        for (auto it = m_activeActivities.begin(); it != m_activeActivities.end(); ++it) {
            if (it->chainId == activityChainId) {
                auto elapsed = std::chrono::steady_clock::now() - it->startTime;
                durationMs = std::chrono::duration<double, std::milli>(elapsed).count();
                m_activeActivities.erase(it);
                break;
            }
        }
    }

    {
        std::lock_guard<std::mutex> lock(m_statsMutex);
        if (success) {
            m_stats.activitiesCompleted++;
        } else {
            m_stats.activitiesFailed++;
        }
        m_completedDurations.push_back(durationMs);

        double total = 0.0;
        for (auto d : m_completedDurations) total += d;
        m_stats.avgActivityDurationMs = total / m_completedDurations.size();
    }

    auto entry = json::object();
    entry["timestamp"] = JsonUtils::currentTimestampISO();
    entry["level"] = success ? "INFO" : "WARN";
    entry["component"] = "telemetry";
    entry["event_type"] = "activity_ended";
    entry["activity_name"] = activityName;
    entry["activity_chain_id"] = activityChainId;
    entry["success"] = success;
    entry["duration_ms"] = durationMs;

    Logger::getInstance().logEvent(entry);
}

TelemetryPipeline::TelemetryStats TelemetryPipeline::getStats() const {
    std::lock_guard<std::mutex> lock(m_statsMutex);
    return m_stats;
}

std::vector<TelemetryPipeline::ActiveActivity> TelemetryPipeline::getActiveActivities() const {
    std::lock_guard<std::mutex> lock(m_activitiesMutex);
    return m_activeActivities;
}

void TelemetryPipeline::metricsThread() {
    while (m_running) {
        recordSystemMetrics();
        for (int i = 0; i < m_collectionIntervalSec * 10 && m_running; ++i) {
            Sleep(100);
        }
    }
}

} // namespace vmh
