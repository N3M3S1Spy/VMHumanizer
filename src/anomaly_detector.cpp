#include "anomaly_detector.h"
#include "logger.h"
#include "common/json_utils.h"
#include <cmath>
#include <fstream>

namespace vmh {

AnomalyDetector::AnomalyDetector() = default;

AnomalyDetector::AnomalyDetector(const json& config) {
    initialize(config);
}

void AnomalyDetector::initialize(const json& config) {
    if (config.contains("anomaly_detection")) {
        const auto& ac = config["anomaly_detection"];
        m_enabled = ac.value<bool>("enabled", true);
        m_sensitivity = ac.value<double>("sensitivity", 0.7);

        std::string rulesFile = ac.value<std::string>("anomaly_rules", "data/templates/anomaly_rules.json");
        loadAnomalyRules(rulesFile);
    } else {
        loadAnomalyRules("data/templates/anomaly_rules.json");
    }
}

void AnomalyDetector::loadAnomalyRules(const std::string& rulesFile) {
    try {
        auto data = json::loadFromFile(rulesFile);
        if (!data.contains("rules") || !data["rules"].isArray()) return;

        for (int i = 0; i < data["rules"].size(); ++i) {
            const auto& r = data["rules"][i];
            Rule rule;
            rule.name = r.value<std::string>("name", "");
            rule.type = r.value<std::string>("type", "");
            rule.field = r.value<std::string>("field", "");
            rule.op = r.value<std::string>("operator", "gt");
            rule.threshold = r.value<double>("threshold", 0.0);
            rule.severity = r.value<double>("severity", 0.5);
            rule.correction = r.value<std::string>("correction", "");
            rule.correctionAmount = r.value<int>("correction_amount", 0);
            rule.description = r.value<std::string>("description", "");
            m_rules.push_back(rule);
        }
    } catch (...) {
        Logger::getInstance().logWarn("anomaly_detector", "Could not load anomaly rules file");
    }
}

bool AnomalyDetector::isOutlier(double value, double mean, double stdDev, double sigmaThreshold) const {
    if (stdDev <= 0.0) return false;
    return std::abs(value - mean) > sigmaThreshold * stdDev;
}

std::vector<AnomalyDetector::Anomaly> AnomalyDetector::checkAgainstRules(const json& event) const {
    std::vector<Anomaly> anomalies;

    for (const auto& rule : m_rules) {
        if (!event.contains(rule.field)) continue;

        double val = event[rule.field].get<double>();
        bool triggered = false;

        if (rule.op == "lt") triggered = val < rule.threshold;
        else if (rule.op == "gt") triggered = val > rule.threshold;
        else if (rule.op == "eq") triggered = std::abs(val - rule.threshold) < 0.001;

        if (triggered && rule.severity >= (1.0 - m_sensitivity)) {
            Anomaly a;
            a.type = rule.type;
            a.description = rule.description;
            a.severity = rule.severity;
            a.suggestion = rule.correction;
            a.ruleName = rule.name;
            anomalies.push_back(a);
        }
    }

    return anomalies;
}

std::vector<AnomalyDetector::Anomaly> AnomalyDetector::detectAnomalies(const json& recentEvents) {
    if (!m_enabled) return {};

    std::vector<Anomaly> allAnomalies;

    if (recentEvents.isArray()) {
        for (int i = 0; i < recentEvents.size(); ++i) {
            auto found = checkAgainstRules(recentEvents[i]);
            allAnomalies.insert(allAnomalies.end(), found.begin(), found.end());
        }
    } else {
        auto found = checkAgainstRules(recentEvents);
        allAnomalies.insert(allAnomalies.end(), found.begin(), found.end());
    }

    // Statistical baseline checks
    {
        std::lock_guard<std::mutex> lock(m_baselineMutex);
        if (m_baseline.sampleCount > 10 && recentEvents.isObject()) {
            if (recentEvents.contains("ikd_mean")) {
                double ikd = recentEvents["ikd_mean"].get<double>();
                if (isOutlier(ikd, m_baseline.avgIKD, m_baseline.avgIKDStdDev)) {
                    Anomaly a;
                    a.type = "keystroke_timing";
                    a.description = "IKD deviates >3 sigma from baseline (value=" +
                                   std::to_string(static_cast<int>(ikd)) + ", baseline=" +
                                   std::to_string(static_cast<int>(m_baseline.avgIKD)) + ")";
                    a.severity = 0.75;
                    a.suggestion = ikd < m_baseline.avgIKD ? "slow_down" : "speed_up";
                    a.ruleName = "statistical_ikd_outlier";
                    allAnomalies.push_back(a);
                }
            }

            if (recentEvents.contains("speed_px_per_sec")) {
                double speed = recentEvents["speed_px_per_sec"].get<double>();
                double speedStdDev = m_baseline.avgMouseSpeed * 0.3;
                if (isOutlier(speed, m_baseline.avgMouseSpeed, speedStdDev)) {
                    Anomaly a;
                    a.type = "mouse_movement";
                    a.description = "Mouse speed deviates significantly from baseline";
                    a.severity = 0.65;
                    a.suggestion = speed > m_baseline.avgMouseSpeed ? "slow_down" : "speed_up";
                    a.ruleName = "statistical_mouse_speed_outlier";
                    allAnomalies.push_back(a);
                }
            }
        }
    }

    m_totalAnomalies += static_cast<int>(allAnomalies.size());

    for (const auto& a : allAnomalies) {
        auto entry = JsonUtils::createAnomalyEntry(a.type, a.description, a.severity, a.suggestion);
        entry["rule_name"] = a.ruleName;
        JsonUtils::appendJsonLine(Logger::getInstance().getAnomaliesLogPath(), entry);
    }

    return allAnomalies;
}

std::vector<AnomalyDetector::Anomaly> AnomalyDetector::validateActivity(const json& activity) {
    return checkAgainstRules(activity);
}

AnomalyDetector::Statistics AnomalyDetector::getBaselineStatistics() const {
    std::lock_guard<std::mutex> lock(m_baselineMutex);
    return m_baseline;
}

void AnomalyDetector::updateBaseline(const json& observation) {
    std::lock_guard<std::mutex> lock(m_baselineMutex);

    m_baseline.sampleCount++;
    double n = static_cast<double>(m_baseline.sampleCount);

    if (observation.contains("ikd_mean")) {
        double ikd = observation["ikd_mean"].get<double>();
        m_baseline.avgIKD = m_baseline.avgIKD * ((n - 1) / n) + ikd / n;
    }

    if (observation.contains("ikd_stddev")) {
        double sd = observation["ikd_stddev"].get<double>();
        m_baseline.avgIKDStdDev = m_baseline.avgIKDStdDev * ((n - 1) / n) + sd / n;
    }

    if (observation.contains("mouse_speed")) {
        double ms = observation["mouse_speed"].get<double>();
        m_baseline.avgMouseSpeed = m_baseline.avgMouseSpeed * ((n - 1) / n) + ms / n;
    }

    if (observation.contains("activity_duration_ms")) {
        double dur = observation["activity_duration_ms"].get<double>();
        m_baseline.avgActivityDuration = m_baseline.avgActivityDuration * ((n - 1) / n) + dur / n;
    }
}

} // namespace vmh
