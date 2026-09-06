#include "anomaly_corrector.h"
#include "logger.h"
#include "common/json_utils.h"
#include "common/utils.h"

namespace vmh {

AnomalyCorrector::AnomalyCorrector() = default;

AnomalyCorrector::AnomalyCorrector(const json& config) {
    initialize(config);
}

void AnomalyCorrector::initialize(const json& config) {
    if (config.contains("anomaly_detection")) {
        const auto& ac = config["anomaly_detection"];
        m_enabled = ac.value<bool>("enabled", true);
        m_autoCorrect = ac.value<bool>("auto_correct", true);

        if (ac.contains("correction_strategies") && ac["correction_strategies"].isArray()) {
            m_enabledStrategies.clear();
            for (int i = 0; i < ac["correction_strategies"].size(); ++i) {
                m_enabledStrategies.push_back(ac["correction_strategies"][i].get<std::string>());
            }
        }
    }

    if (m_enabledStrategies.empty()) {
        m_enabledStrategies = {"slow_down", "speed_up", "inject_pauses", "adjust_timing", "vary_parameters"};
    }
}

bool isStrategyEnabled(const std::vector<std::string>& strategies, const std::string& name) {
    for (const auto& s : strategies) {
        if (s == name) return true;
    }
    return false;
}

std::vector<AnomalyCorrector::Correction> AnomalyCorrector::getRecommendedCorrections(
    const std::vector<AnomalyDetector::Anomaly>& anomalies) const {
    std::vector<Correction> corrections;

    for (const auto& a : anomalies) {
        if (a.severity < 0.3) continue;

        Correction c;
        c.strategy = a.suggestion;

        if (a.suggestion == "slow_down") {
            c.parameter = static_cast<int>(a.severity * 50);
            c.affectedActivities = a.severity > 0.7 ? 10 : 5;
        } else if (a.suggestion == "speed_up") {
            c.parameter = static_cast<int>(a.severity * 30);
            c.affectedActivities = 5;
        } else if (a.suggestion == "inject_pauses") {
            c.parameter = static_cast<int>(a.severity * 5000);
            c.affectedActivities = 3;
        } else if (a.suggestion == "adjust_timing") {
            c.parameter = static_cast<int>(a.severity * 40);
            c.affectedActivities = 5;
        } else if (a.suggestion == "vary_parameters") {
            c.parameter = 0;
            c.affectedActivities = 5;
        } else {
            continue;
        }

        corrections.push_back(c);
    }

    return corrections;
}

void AnomalyCorrector::correctAnomalies(const std::vector<AnomalyDetector::Anomaly>& anomalies) {
    if (!m_enabled || !m_autoCorrect) return;

    auto corrections = getRecommendedCorrections(anomalies);

    for (size_t i = 0; i < corrections.size() && i < anomalies.size(); ++i) {
        const auto& c = corrections[i];
        if (!isStrategyEnabled(m_enabledStrategies, c.strategy)) continue;

        applyCorrectionStrategy(c.strategy, c.parameter);
        recordCorrection(anomalies[i].type, c.strategy, c.parameter);
    }
}

void AnomalyCorrector::applyCorrectionStrategy(const std::string& strategy, int parameter) {
    if (strategy == "slow_down") {
        strategySlowDown(parameter);
    } else if (strategy == "speed_up") {
        strategySpeedUp(parameter);
    } else if (strategy == "inject_pauses") {
        strategyInjectPauses(parameter);
    } else if (strategy == "adjust_timing") {
        strategyAdjustTiming(1.0 + parameter / 100.0);
    } else if (strategy == "vary_parameters") {
        strategyVaryParameters();
    }
}

void AnomalyCorrector::strategySlowDown(int percentSlower) {
    double factor = 1.0 + percentSlower / 100.0;
    m_keystrokeSpeedMultiplier.store(m_keystrokeSpeedMultiplier.load() * factor);
    m_mouseSpeedMultiplier.store(m_mouseSpeedMultiplier.load() * factor);

    Logger::getInstance().logInfo("anomaly_corrector",
        "Applied slow_down: keystroke/mouse multiplier now " +
        std::to_string(m_keystrokeSpeedMultiplier.load()));
}

void AnomalyCorrector::strategySpeedUp(int percentFaster) {
    double factor = 1.0 - (percentFaster / 100.0);
    if (factor < 0.3) factor = 0.3;
    m_keystrokeSpeedMultiplier.store(m_keystrokeSpeedMultiplier.load() * factor);
    m_mouseSpeedMultiplier.store(m_mouseSpeedMultiplier.load() * factor);

    Logger::getInstance().logInfo("anomaly_corrector",
        "Applied speed_up: keystroke/mouse multiplier now " +
        std::to_string(m_keystrokeSpeedMultiplier.load()));
}

void AnomalyCorrector::strategyInjectPauses(int pauseMs) {
    std::lock_guard<std::mutex> lock(m_pauseMutex);
    m_pendingPauseMs += pauseMs;

    Logger::getInstance().logInfo("anomaly_corrector",
        "Injecting pause of " + std::to_string(pauseMs) + "ms");
}

void AnomalyCorrector::strategyAdjustTiming(double timeFactor) {
    m_keystrokeSpeedMultiplier.store(timeFactor);
    m_mouseSpeedMultiplier.store(timeFactor);

    Logger::getInstance().logInfo("anomaly_corrector",
        "Adjusted timing factor to " + std::to_string(timeFactor));
}

void AnomalyCorrector::strategyVaryParameters() {
    double variation = 0.8 + randomDouble(0.0, 0.4);
    m_keystrokeSpeedMultiplier.store(variation);
    m_mouseSpeedMultiplier.store(0.8 + randomDouble(0.0, 0.4));

    Logger::getInstance().logInfo("anomaly_corrector",
        "Varied parameters: keystroke=" + std::to_string(m_keystrokeSpeedMultiplier.load()) +
        " mouse=" + std::to_string(m_mouseSpeedMultiplier.load()));
}

int AnomalyCorrector::getPendingPauseMs() const {
    std::lock_guard<std::mutex> lock(m_pauseMutex);
    return m_pendingPauseMs;
}

void AnomalyCorrector::consumePause() {
    std::lock_guard<std::mutex> lock(m_pauseMutex);
    m_pendingPauseMs = 0;
}

void AnomalyCorrector::recordCorrection(const std::string& anomalyType,
                                          const std::string& strategy, int param) {
    CorrectionRecord rec;
    rec.timestamp = JsonUtils::currentTimestampISO();
    rec.anomalyType = anomalyType;
    rec.strategyApplied = strategy;
    rec.parameter = param;
    rec.wasSuccessful = true;

    {
        std::lock_guard<std::mutex> lock(m_historyMutex);
        m_history.push_back(rec);
        if (m_history.size() > 500) {
            m_history.erase(m_history.begin(), m_history.begin() + 100);
        }
    }

    m_totalCorrections++;

    auto entry = json::object();
    entry["timestamp"] = rec.timestamp;
    entry["level"] = "INFO";
    entry["component"] = "anomaly_corrector";
    entry["event_type"] = "correction_applied";
    entry["anomaly_type"] = anomalyType;
    entry["strategy"] = strategy;
    entry["parameter"] = param;

    Logger::getInstance().logEvent(entry);
}

std::vector<AnomalyCorrector::CorrectionRecord> AnomalyCorrector::getCorrectionHistory() const {
    std::lock_guard<std::mutex> lock(m_historyMutex);
    return m_history;
}

} // namespace vmh
