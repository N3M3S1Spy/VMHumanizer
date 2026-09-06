#include "keystroke_emulator.h"
#include "common/utils.h"
#include "windows/winapi_wrapper.h"
#include <algorithm>
#include <cmath>

namespace vmh {

KeystrokeEmulator::KeystrokeEmulator(const json& config) {
    if (config.contains("keystroke")) {
        const auto& k = config["keystroke"];
        if (k.contains("key_hold_duration") && k["key_hold_duration"].isArray()) {
            m_cfg.keyHoldMin = static_cast<int>(k["key_hold_duration"][0].getInt(40));
            m_cfg.keyHoldMax = static_cast<int>(k["key_hold_duration"][1].getInt(200));
        }
        m_cfg.ikdMean = k.value<int>("inter_key_delay_mean", 120);
        m_cfg.ikdStd = k.value<int>("inter_key_delay_std", 40);
        m_cfg.errorRate = k.value<double>("error_rate", 0.08);
        if (k.contains("word_pause_range") && k["word_pause_range"].isArray()) {
            m_cfg.wordPauseMin = static_cast<int>(k["word_pause_range"][0].getInt(50));
            m_cfg.wordPauseMax = static_cast<int>(k["word_pause_range"][1].getInt(300));
        }
        if (k.contains("sentence_pause_range") && k["sentence_pause_range"].isArray()) {
            m_cfg.sentencePauseMin = static_cast<int>(k["sentence_pause_range"][0].getInt(200));
            m_cfg.sentencePauseMax = static_cast<int>(k["sentence_pause_range"][1].getInt(800));
        }

        if (k.contains("digraph_timings") && k["digraph_timings"].isObject()) {
            for (auto& [key, val] : k["digraph_timings"].items()) {
                m_digraphTimings[key] = static_cast<int>(val.getInt(m_cfg.ikdMean));
            }
        }
    }

    // Default digraph timings for common pairs
    if (m_digraphTimings.empty()) {
        m_digraphTimings = {
            {"th", 70}, {"he", 75}, {"in", 80}, {"er", 85}, {"an", 80},
            {"re", 85}, {"on", 80}, {"at", 90}, {"en", 80}, {"nd", 85},
            {"ti", 90}, {"es", 85}, {"or", 90}, {"te", 85}, {"of", 95},
            {"ed", 90}, {"is", 80}, {"it", 85}, {"al", 90}, {"ar", 95},
            {"st", 75}, {"to", 85}, {"nt", 80}, {"ng", 85}, {"se", 90},
            {"ha", 85}, {"as", 90}, {"ou", 95}, {"io", 100}, {"le", 90},
            {"ve", 95}, {"co", 100}, {"me", 90}, {"de", 95}, {"hi", 90},
            {"ri", 95}, {"ro", 100}, {"ic", 95}, {"ne", 90}, {"ea", 95},
            {"qz", 200}, {"zx", 190}, {"xq", 195}, {"qw", 150}, {"zp", 180},
            {"bv", 170}, {"jk", 160}, {"yp", 155},
        };
    }

    logInfo("KeystrokeEmulator initialized (IKD mean=%d, std=%d, errorRate=%.2f)",
            m_cfg.ikdMean, m_cfg.ikdStd, m_cfg.errorRate);
}

int KeystrokeEmulator::getKeyHoldDuration() {
    int hold = static_cast<int>(normalRandom(
        (m_cfg.keyHoldMin + m_cfg.keyHoldMax) / 2.0,
        (m_cfg.keyHoldMax - m_cfg.keyHoldMin) / 4.0
    ));
    return std::clamp(hold, m_cfg.keyHoldMin, m_cfg.keyHoldMax);
}

int KeystrokeEmulator::getIKDDuration(char prevKey, char currentKey) {
    std::string digraph;
    digraph += static_cast<char>(std::tolower(prevKey));
    digraph += static_cast<char>(std::tolower(currentKey));

    int baseTiming = m_cfg.ikdMean;
    auto it = m_digraphTimings.find(digraph);
    if (it != m_digraphTimings.end()) {
        baseTiming = it->second;
    }

    int ikd = static_cast<int>(normalRandom(baseTiming, m_cfg.ikdStd * 0.5));
    return std::max(20, ikd);
}

bool KeystrokeEmulator::shouldInjectError() {
    return randomDouble(0.0, 1.0) < m_cfg.errorRate;
}

char KeystrokeEmulator::getAdjacentKey(char key) {
    // QWERTY adjacency map
    static const std::map<char, std::string> adjacency = {
        {'q', "wa"}, {'w', "qeas"}, {'e', "wrds"}, {'r', "etdf"},
        {'t', "ryfg"}, {'y', "tugh"}, {'u', "yijh"}, {'i', "uojk"},
        {'o', "iplk"}, {'p', "ol"},
        {'a', "qwsz"}, {'s', "wedxza"}, {'d', "erfcxs"}, {'f', "rtgvcd"},
        {'g', "tyhbvf"}, {'h', "yujnbg"}, {'j', "uikmnh"}, {'k', "iolmj"},
        {'l', "opk"},
        {'z', "asx"}, {'x', "zsdc"}, {'c', "xdfv"}, {'v', "cfgb"},
        {'b', "vghn"}, {'n', "bhjm"}, {'m', "njk"},
    };

    char lower = static_cast<char>(std::tolower(key));
    auto it = adjacency.find(lower);
    if (it != adjacency.end() && !it->second.empty()) {
        char adj = it->second[randomInt(0, static_cast<int>(it->second.size()) - 1)];
        return std::isupper(key) ? static_cast<char>(std::toupper(adj)) : adj;
    }
    return key;
}

void KeystrokeEmulator::injectTypo(const std::string& text, size_t& pos) {
    if (pos >= text.size()) return;

    char wrongKey = getAdjacentKey(text[pos]);
    int holdMs = getKeyHoldDuration();

    // Type wrong character
    WinAPI::sendCharDown(wrongKey);
    WinAPI::sleepMs(holdMs);
    WinAPI::sendCharUp(wrongKey);

    // Pause as if noticing the error
    int noticeDelay = randomInt(200, 400);
    WinAPI::sleepMs(noticeDelay);

    // Backspace to delete wrong character
    int bsHold = getKeyHoldDuration();
    WinAPI::sendKeyDown(VK_BACK);
    WinAPI::sleepMs(bsHold);
    WinAPI::sendKeyUp(VK_BACK);

    // Small pause before retyping
    WinAPI::sleepMs(randomInt(50, 150));

    // Type correct character
    holdMs = getKeyHoldDuration();
    WinAPI::sendCharDown(text[pos]);
    WinAPI::sleepMs(holdMs);
    WinAPI::sendCharUp(text[pos]);

    m_totalErrors++;
    logDebug("Typo injected: '%c' -> '%c' -> corrected at pos %zu", text[pos], wrongKey, pos);
}

bool KeystrokeEmulator::isPunctuation(char ch) {
    return ch == ',' || ch == ';' || ch == ':' || ch == '-';
}

bool KeystrokeEmulator::isEndOfSentence(char ch) {
    return ch == '.' || ch == '!' || ch == '?';
}

void KeystrokeEmulator::pressKey(char key, int durationMs) {
    if (durationMs < 0) durationMs = getKeyHoldDuration();
    WinAPI::sendCharDown(key);
    WinAPI::sleepMs(durationMs);
    WinAPI::sendCharUp(key);
    m_totalKeysTyped++;
}

void KeystrokeEmulator::typeString(const std::string& text, bool includeErrors) {
    if (text.empty()) return;

    bool errorInCurrentWord = false;
    char prevChar = 0;

    for (size_t i = 0; i < text.size(); ++i) {
        char ch = text[i];

        // Inter-key delay
        if (prevChar != 0) {
            int ikd = getIKDDuration(prevChar, ch);
            m_totalIKD += ikd;
            WinAPI::sleepMs(ikd);
        }

        // Word boundary pauses
        if (ch == ' ') {
            errorInCurrentWord = false;
            int pause = randomInt(m_cfg.wordPauseMin, m_cfg.wordPauseMax);
            WinAPI::sleepMs(pause);
            pressKey(ch);
            prevChar = ch;
            continue;
        }

        // Sentence end pauses
        if (prevChar != 0 && isEndOfSentence(prevChar) && ch == ' ') {
            int pause = randomInt(m_cfg.sentencePauseMin, m_cfg.sentencePauseMax);
            WinAPI::sleepMs(pause);
        }

        // Paragraph/newline thinking pause
        if (ch == '\n') {
            int thinkPause = randomInt(1000, 3000);
            WinAPI::sleepMs(thinkPause);
            WinAPI::sendKeyPress(VK_RETURN, getKeyHoldDuration());
            prevChar = ch;
            m_totalKeysTyped++;
            continue;
        }

        // Inject typo
        if (includeErrors && !errorInCurrentWord && std::isalpha(ch) && shouldInjectError()) {
            injectTypo(text, i);
            errorInCurrentWord = true;
            prevChar = ch;
            m_totalKeysTyped++;
            continue;
        }

        // Normal key press
        pressKey(ch);

        // Add longer pause after punctuation
        if (isEndOfSentence(ch)) {
            WinAPI::sleepMs(randomInt(m_cfg.sentencePauseMin, m_cfg.sentencePauseMax));
        } else if (isPunctuation(ch)) {
            WinAPI::sleepMs(randomInt(30, 100));
        }

        prevChar = ch;
    }

    logDebug("Typed %zu characters, errors=%d", text.size(), m_totalErrors);
}

KeystrokeEmulator::TypingStats KeystrokeEmulator::getStatistics() const {
    TypingStats stats;
    stats.totalKeysTyped = m_totalKeysTyped;
    stats.totalErrors = m_totalErrors;
    stats.avgIKD = (m_totalKeysTyped > 1) ? m_totalIKD / (m_totalKeysTyped - 1) : 0.0;
    stats.errorRate = (m_totalKeysTyped > 0) ?
        static_cast<double>(m_totalErrors) / m_totalKeysTyped : 0.0;
    return stats;
}

} // namespace vmh
