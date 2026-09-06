#pragma once
#include "common/json.hpp"
#include "common/types.h"
#include <string>
#include <map>

namespace vmh {

class KeystrokeEmulator {
public:
    KeystrokeEmulator(const json& config);

    void typeString(const std::string& text, bool includeErrors = true);
    void pressKey(char key, int durationMs = -1);

    struct TypingStats {
        double avgIKD;
        double errorRate;
        int totalKeysTyped;
        int totalErrors;
    };
    TypingStats getStatistics() const;

private:
    int getIKDDuration(char prevKey, char currentKey);
    bool shouldInjectError();
    void injectTypo(const std::string& text, size_t& pos);
    int getKeyHoldDuration();
    char getAdjacentKey(char key);
    bool isPunctuation(char ch);
    bool isEndOfSentence(char ch);

    KeystrokeConfig m_cfg;
    std::map<std::string, int> m_digraphTimings;

    int m_totalKeysTyped = 0;
    int m_totalErrors = 0;
    double m_totalIKD = 0.0;
};

} // namespace vmh
