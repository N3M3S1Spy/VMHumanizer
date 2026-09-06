#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <windows.h>

namespace vmh {

enum class LogLevel { DEBUG, INFO, WARN, ERR };

enum class SpeedProfile { SLOW_START, FAST_THEN_SLOW, UNIFORM };

enum class CurveType { QUADRATIC_BEZIER, CUBIC_BEZIER };

struct Point2D {
    double x;
    double y;
};

struct MouseConfig {
    CurveType curveType = CurveType::QUADRATIC_BEZIER;
    double smoothness = 0.85;
    int jitterMin = 2;
    int jitterMax = 5;
    bool pauseOnTarget = true;
    int pauseDurationMin = 100;
    int pauseDurationMax = 500;
    SpeedProfile speedProfile = SpeedProfile::FAST_THEN_SLOW;
};

struct KeystrokeConfig {
    int keyHoldMin = 40;
    int keyHoldMax = 200;
    int ikdMean = 120;
    int ikdStd = 40;
    double errorRate = 0.08;
    int wordPauseMin = 50;
    int wordPauseMax = 300;
    int sentencePauseMin = 200;
    int sentencePauseMax = 800;
};

struct RegistryMRUConfig {
    bool enableTypedURLs = true;
    bool enableUserAssist = true;
    bool enableOfficeMRU = true;
    int maxURLs = 20;
    int maxOfficeEntries = 15;
    std::vector<std::string> includePrograms;
    std::vector<std::string> includeDocuments;
};

struct EventLogConfig {
    bool enabled = true;
    int maxEventsPerLog = 10000;
    std::string profileName = "developer";
};

class VMHException : public std::runtime_error {
public:
    VMHException(const std::string& msg) : std::runtime_error(msg) {}
};

class RegistryException : public VMHException {
public:
    RegistryException(const std::string& msg, LONG errorCode = 0)
        : VMHException(msg), m_errorCode(errorCode) {}
    LONG errorCode() const { return m_errorCode; }
private:
    LONG m_errorCode;
};

class EventLogException : public VMHException {
public:
    EventLogException(const std::string& msg, DWORD errorCode = 0)
        : VMHException(msg), m_errorCode(errorCode) {}
    DWORD errorCode() const { return m_errorCode; }
private:
    DWORD m_errorCode;
};

} // namespace vmh
