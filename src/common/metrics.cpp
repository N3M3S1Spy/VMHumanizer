#include "metrics.h"
#include <psapi.h>

#pragma comment(lib, "psapi.lib")

namespace vmh {

MetricsCollector& MetricsCollector::getInstance() {
    static MetricsCollector instance;
    return instance;
}

MetricsCollector::MetricsCollector() {
    m_hProcess = GetCurrentProcess();
    initCPUTracking();
}

void MetricsCollector::initCPUTracking() {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    m_numProcessors = sysInfo.dwNumberOfProcessors;

    FILETIME ftime, fsys, fuser;
    GetSystemTimeAsFileTime(&ftime);
    memcpy(&m_lastCPU, &ftime, sizeof(FILETIME));

    GetProcessTimes(m_hProcess, &ftime, &ftime, &fsys, &fuser);
    memcpy(&m_lastSysCPU, &fsys, sizeof(FILETIME));
    memcpy(&m_lastUserCPU, &fuser, sizeof(FILETIME));
    m_initialized = true;
}

double MetricsCollector::getCPUUsage() {
    if (!m_initialized) initCPUTracking();

    FILETIME ftime, fsys, fuser;
    ULARGE_INTEGER now, sys, user;

    GetSystemTimeAsFileTime(&ftime);
    memcpy(&now, &ftime, sizeof(FILETIME));

    GetProcessTimes(m_hProcess, &ftime, &ftime, &fsys, &fuser);
    memcpy(&sys, &fsys, sizeof(FILETIME));
    memcpy(&user, &fuser, sizeof(FILETIME));

    double percent = 0.0;
    if (now.QuadPart - m_lastCPU.QuadPart > 0) {
        percent = static_cast<double>(
            (sys.QuadPart - m_lastSysCPU.QuadPart) +
            (user.QuadPart - m_lastUserCPU.QuadPart));
        percent /= static_cast<double>(now.QuadPart - m_lastCPU.QuadPart);
        percent /= m_numProcessors;
        percent *= 100.0;
    }

    m_lastCPU = now;
    m_lastSysCPU = sys;
    m_lastUserCPU = user;

    return percent;
}

int MetricsCollector::getMemoryUsageMb() {
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (GetProcessMemoryInfo(m_hProcess, reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc), sizeof(pmc))) {
        return static_cast<int>(pmc.WorkingSetSize / (1024 * 1024));
    }
    return 0;
}

int MetricsCollector::getThreadCount() {
    DWORD processId = GetCurrentProcessId();
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return 0;

    THREADENTRY32 te32;
    te32.dwSize = sizeof(THREADENTRY32);
    int count = 0;

    if (Thread32First(hSnapshot, &te32)) {
        do {
            if (te32.th32OwnerProcessID == processId) count++;
        } while (Thread32Next(hSnapshot, &te32));
    }

    CloseHandle(hSnapshot);
    return count;
}

SystemMetrics MetricsCollector::collectMetrics() {
    SystemMetrics m;
    m.cpuPercent = getCPUUsage();
    m.memoryMb = getMemoryUsageMb();
    m.diskMbUsed = 0;
    m.activeThreads = getThreadCount();
    m.activityQueueLength = 0;
    return m;
}

} // namespace vmh
