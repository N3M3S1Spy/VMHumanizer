#pragma once
#include "common/json.hpp"
#include <string>
#include <vector>
#include <mutex>

namespace vmh {

class AnomalyDetector {
public:
    AnomalyDetector();
    explicit AnomalyDetector(const json& config);

    void initialize(const json& config);

    struct Anomaly {
        std::string type;
        std::string description;
        double severity = 0.0;
        std::string suggestion;
        std::string ruleName;
    };

    std::vector<Anomaly> detectAnomalies(const json& recentEvents);
    std::vector<Anomaly> validateActivity(const json& activity);

    struct Statistics {
        double avgIKD = 120.0;
        double avgIKDStdDev = 30.0;
        double avgMouseSpeed = 800.0;
        double avgActivityDuration = 300000.0;
        int sampleCount = 0;
    };
    Statistics getBaselineStatistics() const;

    void updateBaseline(const json& observation);

    int getTotalAnomaliesDetected() const { return m_totalAnomalies; }

private:
    bool isOutlier(double value, double mean, double stdDev, double sigmaThreshold = 3.0) const;
    void loadAnomalyRules(const std::string& rulesFile);
    std::vector<Anomaly> checkAgainstRules(const json& event) const;

    struct Rule {
        std::string name;
        std::string type;
        std::string field;
        std::string op;
        double threshold = 0.0;
        double severity = 0.5;
        std::string correction;
        int correctionAmount = 0;
        std::string description;
    };

    std::vector<Rule> m_rules;
    mutable std::mutex m_baselineMutex;
    Statistics m_baseline;
    double m_sensitivity = 0.7;
    bool m_enabled = true;
    int m_totalAnomalies = 0;
};

} // namespace vmh
