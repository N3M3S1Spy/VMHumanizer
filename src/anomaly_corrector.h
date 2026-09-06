#pragma once
#include "common/json.hpp"
#include "anomaly_detector.h"
#include <string>
#include <vector>
#include <mutex>
#include <atomic>

namespace vmh {

class AnomalyCorrector {
public:
    AnomalyCorrector();
    explicit AnomalyCorrector(const json& config);

    void initialize(const json& config);

    struct Correction {
        std::string strategy;
        int parameter = 0;
        int affectedActivities = 5;
    };

    std::vector<Correction> getRecommendedCorrections(
        const std::vector<AnomalyDetector::Anomaly>& anomalies) const;

    void correctAnomalies(const std::vector<AnomalyDetector::Anomaly>& anomalies);

    void applyCorrectionStrategy(const std::string& strategy, int parameter);

    struct CorrectionRecord {
        std::string timestamp;
        std::string anomalyType;
        std::string strategyApplied;
        int parameter;
        bool wasSuccessful;
    };
    std::vector<CorrectionRecord> getCorrectionHistory() const;

    double getKeystrokeSpeedMultiplier() const { return m_keystrokeSpeedMultiplier.load(); }
    double getMouseSpeedMultiplier() const { return m_mouseSpeedMultiplier.load(); }
    int getPendingPauseMs() const;
    void consumePause();

    int getTotalCorrections() const { return m_totalCorrections; }

private:
    void strategySlowDown(int percentSlower);
    void strategySpeedUp(int percentFaster);
    void strategyInjectPauses(int pauseMs);
    void strategyAdjustTiming(double timeFactor);
    void strategyVaryParameters();
    void recordCorrection(const std::string& anomalyType, const std::string& strategy, int param);

    bool m_autoCorrect = true;
    bool m_enabled = true;
    std::vector<std::string> m_enabledStrategies;

    std::atomic<double> m_keystrokeSpeedMultiplier{1.0};
    std::atomic<double> m_mouseSpeedMultiplier{1.0};

    mutable std::mutex m_pauseMutex;
    int m_pendingPauseMs = 0;

    mutable std::mutex m_historyMutex;
    std::vector<CorrectionRecord> m_history;

    int m_totalCorrections = 0;
};

} // namespace vmh
