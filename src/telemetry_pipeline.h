#pragma once
#include "common/json.hpp"
#include "common/metrics.h"
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <vector>
#include <chrono>

namespace vmh {

class TelemetryPipeline {
public:
    TelemetryPipeline();
    ~TelemetryPipeline();

    void initialize(const json& config);

    void startMetricsCollection();
    void stopMetricsCollection();

    void recordSystemMetrics();

    void recordActivityStart(const std::string& activityName, const std::string& activityChainId);
    void recordActivityEnd(const std::string& activityName, const std::string& activityChainId, bool success);

    struct TelemetryStats {
        int activitiesCompleted = 0;
        int activitiesFailed = 0;
        double avgActivityDurationMs = 0.0;
        double cpuUsagePercent = 0.0;
        int memoryUsageMb = 0;
        int activeThreads = 0;
        int activityQueueLength = 0;
    };
    TelemetryStats getStats() const;

    struct ActiveActivity {
        std::string name;
        std::string chainId;
        std::chrono::steady_clock::time_point startTime;
    };
    std::vector<ActiveActivity> getActiveActivities() const;

private:
    void metricsThread();

    int m_collectionIntervalSec = 60;
    std::thread m_metricsThread;
    std::atomic<bool> m_running{false};
    bool m_initialized = false;

    mutable std::mutex m_statsMutex;
    TelemetryStats m_stats;

    mutable std::mutex m_activitiesMutex;
    std::vector<ActiveActivity> m_activeActivities;
    std::vector<double> m_completedDurations;
};

} // namespace vmh
