#include "json_utils.h"
#include "utils.h"
#include <fstream>
#include <filesystem>
#include <cstdio>

namespace vmh {
namespace JsonUtils {

std::string generateUUID() {
    char uuid[64];
    snprintf(uuid, sizeof(uuid), "%08x-%04x-%04x-%04x-%04x%08x",
             randomInt(0, INT_MAX),
             randomInt(0, 0xFFFF),
             0x4000 | randomInt(0, 0x0FFF),
             0x8000 | randomInt(0, 0x3FFF),
             randomInt(0, 0xFFFF),
             randomInt(0, INT_MAX));
    return uuid;
}

std::string currentTimestampISO() {
    SYSTEMTIME st;
    GetSystemTime(&st);
    char buf[64];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ",
             st.wYear, st.wMonth, st.wDay,
             st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
    return buf;
}

json createLogEntry(const std::string& level, const std::string& component,
                    const std::string& message) {
    auto entry = json::object();
    entry["timestamp"] = currentTimestampISO();
    entry["level"] = level;
    entry["component"] = component;
    entry["message"] = message;
    return entry;
}

json createMetricsEntry(double cpu, int memMb, int threads) {
    auto entry = json::object();
    entry["timestamp"] = currentTimestampISO();
    entry["level"] = "INFO";
    entry["component"] = "metrics";
    entry["event_type"] = "system_metrics";
    entry["cpu_percent"] = cpu;
    entry["memory_mb"] = memMb;
    entry["active_threads"] = threads;
    return entry;
}

json createAnomalyEntry(const std::string& type, const std::string& description,
                         double severity, const std::string& suggestion) {
    auto entry = json::object();
    entry["timestamp"] = currentTimestampISO();
    entry["level"] = severity > 0.7 ? "ERROR" : (severity > 0.3 ? "WARN" : "INFO");
    entry["component"] = "anomaly_detector";
    entry["event_type"] = "anomaly_detected";
    entry["anomaly_type"] = type;
    entry["description"] = description;
    entry["severity"] = severity;
    entry["suggestion"] = suggestion;
    return entry;
}

bool appendJsonLine(const std::string& filePath, const json& entry) {
    try {
        std::filesystem::create_directories(std::filesystem::path(filePath).parent_path());
        std::ofstream f(filePath, std::ios::app);
        if (!f.is_open()) return false;
        f << entry.dump(-1) << "\n";
        return true;
    } catch (...) {
        return false;
    }
}

std::vector<json> readJsonLines(const std::string& filePath, int maxLines) {
    std::vector<json> entries;
    try {
        std::ifstream f(filePath);
        if (!f.is_open()) return entries;

        std::string line;
        while (std::getline(f, line) && static_cast<int>(entries.size()) < maxLines) {
            if (line.empty()) continue;
            try {
                entries.push_back(json::parse(line));
            } catch (...) {}
        }
    } catch (...) {}
    return entries;
}

} // namespace JsonUtils
} // namespace vmh
