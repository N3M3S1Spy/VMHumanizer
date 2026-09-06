#include "profile_loader.h"
#include "utils.h"
#include <filesystem>

namespace vmh {

ProfileLoader::ProfileLoader() {
    m_config = createDefaultConfig();
}

bool ProfileLoader::loadProfile(const std::string& profileName) {
    std::string path = getDataDirectory() + "\\profiles\\" + profileName + ".json";
    return loadFromFile(path);
}

bool ProfileLoader::loadFromFile(const std::string& path) {
    try {
        if (!std::filesystem::exists(path)) {
            logWarning("Profile file not found: %s, using defaults", path.c_str());
            return false;
        }
        json loaded = json::loadFromFile(path);
        // Merge loaded over defaults
        for (auto& [key, val] : loaded.items()) {
            m_config[key] = val;
        }
        m_loaded = true;
        logInfo("Profile loaded: %s", path.c_str());
        return true;
    } catch (const std::exception& e) {
        logError("Failed to load profile %s: %s", path.c_str(), e.what());
        return false;
    }
}

const json& ProfileLoader::getMouseConfig() const {
    return m_config["mouse"];
}

const json& ProfileLoader::getKeystrokeConfig() const {
    return m_config["keystroke"];
}

const json& ProfileLoader::getRegistryMRUConfig() const {
    return m_config["registry_mru"];
}

const json& ProfileLoader::getEventLogConfig() const {
    return m_config["event_log"];
}

std::string ProfileLoader::getProfileName() const {
    return m_config.value<std::string>("profile_name", "default");
}

std::string ProfileLoader::getProfileType() const {
    return m_config.value<std::string>("profile_type", "developer");
}

MouseConfig ProfileLoader::parseMouseConfig() const {
    MouseConfig cfg;
    const auto& m = getMouseConfig();
    if (m.isNull()) return cfg;

    std::string ct = m.value<std::string>("curve_type", "quadratic_bezier");
    if (ct == "cubic_bezier") cfg.curveType = CurveType::CUBIC_BEZIER;
    else cfg.curveType = CurveType::QUADRATIC_BEZIER;

    cfg.smoothness = m.value<double>("smoothness", 0.85);
    cfg.pauseOnTarget = m.value<bool>("pause_on_target", true);

    if (m.contains("jitter_amplitude") && m["jitter_amplitude"].isArray()) {
        cfg.jitterMin = static_cast<int>(m["jitter_amplitude"][0].getInt(2));
        cfg.jitterMax = static_cast<int>(m["jitter_amplitude"][1].getInt(5));
    }
    if (m.contains("pause_duration_range") && m["pause_duration_range"].isArray()) {
        cfg.pauseDurationMin = static_cast<int>(m["pause_duration_range"][0].getInt(100));
        cfg.pauseDurationMax = static_cast<int>(m["pause_duration_range"][1].getInt(500));
    }

    std::string sp = m.value<std::string>("speed_profile", "fast_then_slow");
    if (sp == "slow_start") cfg.speedProfile = SpeedProfile::SLOW_START;
    else if (sp == "uniform") cfg.speedProfile = SpeedProfile::UNIFORM;
    else cfg.speedProfile = SpeedProfile::FAST_THEN_SLOW;

    return cfg;
}

KeystrokeConfig ProfileLoader::parseKeystrokeConfig() const {
    KeystrokeConfig cfg;
    const auto& k = getKeystrokeConfig();
    if (k.isNull()) return cfg;

    if (k.contains("key_hold_duration") && k["key_hold_duration"].isArray()) {
        cfg.keyHoldMin = static_cast<int>(k["key_hold_duration"][0].getInt(40));
        cfg.keyHoldMax = static_cast<int>(k["key_hold_duration"][1].getInt(200));
    }
    cfg.ikdMean = k.value<int>("inter_key_delay_mean", 120);
    cfg.ikdStd = k.value<int>("inter_key_delay_std", 40);
    cfg.errorRate = k.value<double>("error_rate", 0.08);

    if (k.contains("word_pause_range") && k["word_pause_range"].isArray()) {
        cfg.wordPauseMin = static_cast<int>(k["word_pause_range"][0].getInt(50));
        cfg.wordPauseMax = static_cast<int>(k["word_pause_range"][1].getInt(300));
    }
    if (k.contains("sentence_pause_range") && k["sentence_pause_range"].isArray()) {
        cfg.sentencePauseMin = static_cast<int>(k["sentence_pause_range"][0].getInt(200));
        cfg.sentencePauseMax = static_cast<int>(k["sentence_pause_range"][1].getInt(800));
    }

    return cfg;
}

RegistryMRUConfig ProfileLoader::parseRegistryMRUConfig() const {
    RegistryMRUConfig cfg;
    const auto& r = getRegistryMRUConfig();
    if (r.isNull()) return cfg;

    cfg.enableTypedURLs = r.value<bool>("enable_typed_urls", true);
    cfg.enableUserAssist = r.value<bool>("enable_user_assist", true);
    cfg.enableOfficeMRU = r.value<bool>("enable_office_mru", true);
    cfg.maxURLs = r.value<int>("max_urls", 20);
    cfg.maxOfficeEntries = r.value<int>("max_office_entries", 15);

    if (r.contains("include_programs") && r["include_programs"].isArray()) {
        for (auto& el : r["include_programs"].elements()) {
            cfg.includePrograms.push_back(el.getString());
        }
    }
    if (r.contains("include_documents") && r["include_documents"].isArray()) {
        for (auto& el : r["include_documents"].elements()) {
            cfg.includeDocuments.push_back(el.getString());
        }
    }

    return cfg;
}

EventLogConfig ProfileLoader::parseEventLogConfig() const {
    EventLogConfig cfg;
    const auto& e = getEventLogConfig();
    if (e.isNull()) return cfg;

    cfg.enabled = e.value<bool>("enabled", true);
    cfg.maxEventsPerLog = e.value<int>("max_events_per_log", 10000);
    cfg.profileName = e.value<std::string>("profile_name", "developer");

    return cfg;
}

json ProfileLoader::createDefaultConfig() {
    auto cfg = json::object();

    auto mouse = json::object();
    mouse["curve_type"] = "quadratic_bezier";
    mouse["smoothness"] = 0.85;
    auto jitter = json::array();
    jitter.push_back(2);
    jitter.push_back(5);
    mouse["jitter_amplitude"] = jitter;
    mouse["pause_on_target"] = true;
    auto pauseRange = json::array();
    pauseRange.push_back(100);
    pauseRange.push_back(500);
    mouse["pause_duration_range"] = pauseRange;
    mouse["speed_profile"] = "fast_then_slow";
    cfg["mouse"] = mouse;

    auto keystroke = json::object();
    auto khd = json::array();
    khd.push_back(40);
    khd.push_back(200);
    keystroke["key_hold_duration"] = khd;
    keystroke["inter_key_delay_mean"] = 120;
    keystroke["inter_key_delay_std"] = 40;
    keystroke["error_rate"] = 0.08;
    auto wpr = json::array();
    wpr.push_back(50);
    wpr.push_back(300);
    keystroke["word_pause_range"] = wpr;
    auto spr = json::array();
    spr.push_back(200);
    spr.push_back(800);
    keystroke["sentence_pause_range"] = spr;
    cfg["keystroke"] = keystroke;

    auto regMru = json::object();
    regMru["enable_typed_urls"] = true;
    regMru["enable_user_assist"] = true;
    regMru["enable_office_mru"] = true;
    regMru["max_urls"] = 20;
    regMru["max_office_entries"] = 15;
    cfg["registry_mru"] = regMru;

    auto eventLog = json::object();
    eventLog["enabled"] = true;
    eventLog["max_events_per_log"] = 10000;
    eventLog["profile_name"] = "developer";
    cfg["event_log"] = eventLog;

    cfg["profile_name"] = "default";
    cfg["profile_type"] = "developer";

    return cfg;
}

} // namespace vmh
