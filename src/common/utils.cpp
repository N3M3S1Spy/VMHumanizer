#include "utils.h"
#include <windows.h>
#include <cstdio>
#include <ctime>
#include <random>
#include <fstream>
#include <filesystem>

namespace vmh {

static std::mutex g_logMutex;
static std::ofstream g_logFile;
static bool g_logToFile = false;

static thread_local std::mt19937 g_rng([]() {
    std::random_device rd;
    return rd();
}());

static void logImpl(LogLevel level, const char* fmt, va_list args) {
    char buffer[2048];
    vsnprintf(buffer, sizeof(buffer), fmt, args);

    const char* levelStr = "INFO";
    switch (level) {
    case LogLevel::DEBUG: levelStr = "DEBUG"; break;
    case LogLevel::INFO:  levelStr = "INFO";  break;
    case LogLevel::WARN:  levelStr = "WARN";  break;
    case LogLevel::ERR:   levelStr = "ERROR"; break;
    }

    SYSTEMTIME st;
    GetLocalTime(&st);
    char timeStr[64];
    snprintf(timeStr, sizeof(timeStr), "%04d-%02d-%02d %02d:%02d:%02d.%03d",
             st.wYear, st.wMonth, st.wDay,
             st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

    char line[2200];
    snprintf(line, sizeof(line), "[%s] [%s] %s\n", timeStr, levelStr, buffer);

    std::lock_guard<std::mutex> lock(g_logMutex);
    fprintf(stderr, "%s", line);
    if (g_logToFile && g_logFile.is_open()) {
        g_logFile << line;
        g_logFile.flush();
    }
}

void logInit(const std::string& logFile) {
    if (!logFile.empty()) {
        std::filesystem::create_directories(std::filesystem::path(logFile).parent_path());
        g_logFile.open(logFile, std::ios::app);
        g_logToFile = g_logFile.is_open();
    }
}

void logMessage(LogLevel level, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    logImpl(level, fmt, args);
    va_end(args);
}

void logInfo(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    logImpl(LogLevel::INFO, fmt, args);
    va_end(args);
}

void logWarning(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    logImpl(LogLevel::WARN, fmt, args);
    va_end(args);
}

void logError(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    logImpl(LogLevel::ERR, fmt, args);
    va_end(args);
}

void logDebug(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    logImpl(LogLevel::DEBUG, fmt, args);
    va_end(args);
}

std::string getExeDirectory() {
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    std::string p(path);
    auto pos = p.find_last_of("\\/");
    return (pos != std::string::npos) ? p.substr(0, pos) : ".";
}

std::string getDataDirectory() {
    return getExeDirectory() + "\\data";
}

std::wstring utf8ToWide(const std::string& str) {
    if (str.empty()) return {};
    int sz = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), nullptr, 0);
    std::wstring result(sz, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &result[0], sz);
    return result;
}

std::string wideToUtf8(const std::wstring& str) {
    if (str.empty()) return {};
    int sz = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), (int)str.size(), nullptr, 0, nullptr, nullptr);
    std::string result(sz, 0);
    WideCharToMultiByte(CP_UTF8, 0, str.c_str(), (int)str.size(), &result[0], sz, nullptr, nullptr);
    return result;
}

int randomInt(int minVal, int maxVal) {
    std::uniform_int_distribution<int> dist(minVal, maxVal);
    return dist(g_rng);
}

double randomDouble(double minVal, double maxVal) {
    std::uniform_real_distribution<double> dist(minVal, maxVal);
    return dist(g_rng);
}

double normalRandom(double mean, double stddev) {
    std::normal_distribution<double> dist(mean, stddev);
    return dist(g_rng);
}

FILETIME systemTimeToFileTime(const SYSTEMTIME& st) {
    FILETIME ft;
    SystemTimeToFileTime(&st, &ft);
    return ft;
}

SYSTEMTIME fileTimeToSystemTime(const FILETIME& ft) {
    SYSTEMTIME st;
    FileTimeToSystemTime(&ft, &st);
    return st;
}

FILETIME getCurrentFileTime() {
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    return ft;
}

FILETIME offsetFileTime(const FILETIME& ft, int64_t offsetSeconds) {
    ULARGE_INTEGER ul;
    ul.LowPart = ft.dwLowDateTime;
    ul.HighPart = ft.dwHighDateTime;
    ul.QuadPart += offsetSeconds * 10000000LL; // 100-nanosecond intervals
    FILETIME result;
    result.dwLowDateTime = ul.LowPart;
    result.dwHighDateTime = ul.HighPart;
    return result;
}

} // namespace vmh
