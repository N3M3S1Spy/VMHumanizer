#pragma once
#include <chrono>
#include <string>

namespace vmh {

class Timer {
public:
    Timer();
    void start();
    void stop();
    void reset();

    double elapsedMs() const;
    double elapsedSeconds() const;
    bool isRunning() const { return m_running; }

    std::string formatElapsed() const;

    static int getCurrentHour();
    static int getCurrentDayOfWeek(); // 0=Sunday
    static bool isWeekend();
    static bool isWorkingHours(int startHour = 8, int endHour = 18);

private:
    std::chrono::high_resolution_clock::time_point m_start;
    std::chrono::high_resolution_clock::time_point m_end;
    bool m_running = false;
};

} // namespace vmh
