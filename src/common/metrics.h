#pragma once
#include <string>
#include <atomic>
#include <windows.h>

namespace vmh {

struct SystemMetrics {
    double cpuPercent;
    int memoryMb;
    int diskMbUsed;
    int activeThreads;
    int activityQueueLength;
};

class MetricsCollector {
public:
    static MetricsCollector& getInstance();

    SystemMetrics collectMetrics();

    double getCPUUsage();
    int getMemoryUsageMb();
    int getThreadCount();

private:
    MetricsCollector();
    MetricsCollector(const MetricsCollector&) = delete;
    MetricsCollector& operator=(const MetricsCollector&) = delete;

    ULARGE_INTEGER m_lastCPU;
    ULARGE_INTEGER m_lastSysCPU;
    ULARGE_INTEGER m_lastUserCPU;
    int m_numProcessors;
    HANDLE m_hProcess;
    bool m_initialized = false;

    void initCPUTracking();
};

} // namespace vmh
