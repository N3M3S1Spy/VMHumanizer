#include "timer_utils.h"
#include <windows.h>
#include <cstdio>

namespace vmh {

Timer::Timer() {
    reset();
}

void Timer::start() {
    m_start = std::chrono::high_resolution_clock::now();
    m_running = true;
}

void Timer::stop() {
    m_end = std::chrono::high_resolution_clock::now();
    m_running = false;
}

void Timer::reset() {
    m_start = std::chrono::high_resolution_clock::now();
    m_end = m_start;
    m_running = false;
}

double Timer::elapsedMs() const {
    auto end = m_running ? std::chrono::high_resolution_clock::now() : m_end;
    return std::chrono::duration<double, std::milli>(end - m_start).count();
}

double Timer::elapsedSeconds() const {
    return elapsedMs() / 1000.0;
}

std::string Timer::formatElapsed() const {
    double totalSec = elapsedSeconds();
    int hours = static_cast<int>(totalSec) / 3600;
    int mins = (static_cast<int>(totalSec) % 3600) / 60;
    int secs = static_cast<int>(totalSec) % 60;

    char buf[64];
    if (hours > 0) {
        snprintf(buf, sizeof(buf), "%dh %02dm %02ds", hours, mins, secs);
    } else if (mins > 0) {
        snprintf(buf, sizeof(buf), "%dm %02ds", mins, secs);
    } else {
        snprintf(buf, sizeof(buf), "%ds", secs);
    }
    return buf;
}

int Timer::getCurrentHour() {
    SYSTEMTIME st;
    GetLocalTime(&st);
    return st.wHour;
}

int Timer::getCurrentDayOfWeek() {
    SYSTEMTIME st;
    GetLocalTime(&st);
    return st.wDayOfWeek;
}

bool Timer::isWeekend() {
    int dow = getCurrentDayOfWeek();
    return dow == 0 || dow == 6;
}

bool Timer::isWorkingHours(int startHour, int endHour) {
    int hour = getCurrentHour();
    return hour >= startHour && hour < endHour && !isWeekend();
}

} // namespace vmh
