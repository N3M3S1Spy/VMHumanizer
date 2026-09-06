#pragma once
#include "json.hpp"
#include <string>
#include <vector>

namespace vmh {

namespace JsonUtils {
    std::string generateUUID();
    std::string currentTimestampISO();
    json createLogEntry(const std::string& level, const std::string& component,
                        const std::string& message);
    json createMetricsEntry(double cpu, int memMb, int threads);
    json createAnomalyEntry(const std::string& type, const std::string& description,
                            double severity, const std::string& suggestion);

    bool appendJsonLine(const std::string& filePath, const json& entry);
    std::vector<json> readJsonLines(const std::string& filePath, int maxLines = 1000);
}

} // namespace vmh
