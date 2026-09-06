#pragma once
#include <string>
#include <cstdarg>
#include <mutex>
#include "types.h"

namespace vmh {

void logInit(const std::string& logFile = "");
void logMessage(LogLevel level, const char* fmt, ...);
void logInfo(const char* fmt, ...);
void logWarning(const char* fmt, ...);
void logError(const char* fmt, ...);
void logDebug(const char* fmt, ...);

std::string getExeDirectory();
std::string getDataDirectory();
std::wstring utf8ToWide(const std::string& str);
std::string wideToUtf8(const std::wstring& str);

int randomInt(int minVal, int maxVal);
double randomDouble(double minVal, double maxVal);
double normalRandom(double mean, double stddev);

FILETIME systemTimeToFileTime(const SYSTEMTIME& st);
SYSTEMTIME fileTimeToSystemTime(const FILETIME& ft);
FILETIME getCurrentFileTime();
FILETIME offsetFileTime(const FILETIME& ft, int64_t offsetSeconds);

} // namespace vmh
