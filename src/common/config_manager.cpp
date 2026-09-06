#include "config_manager.h"
#include "utils.h"
#include <filesystem>

namespace vmh {

ConfigManager& ConfigManager::getInstance() {
    static ConfigManager instance;
    return instance;
}

bool ConfigManager::loadConfig(const std::string& profileName) {
    std::string path = getDataDirectory() + "\\profiles\\" + profileName + ".json";
    if (loadFromFile(path)) {
        m_activeProfile = profileName;
        return true;
    }
    return false;
}

bool ConfigManager::loadFromFile(const std::string& path) {
    std::lock_guard<std::mutex> lock(m_mutex);
    try {
        if (!std::filesystem::exists(path)) {
            logWarning("Config file not found: %s", path.c_str());
            return false;
        }
        m_config = json::loadFromFile(path);
        logInfo("Config loaded from: %s", path.c_str());
        return true;
    } catch (const std::exception& e) {
        logError("Failed to load config: %s", e.what());
        return false;
    }
}

const json& ConfigManager::getSection(const std::string& section) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_overrides.contains(section)) return m_overrides[section];
    return m_config[section];
}

void ConfigManager::setOverride(const std::string& key, const json& value) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_overrides[key] = value;
}

void ConfigManager::clearOverrides() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_overrides = json::object();
}

std::string ConfigManager::getActiveProfileName() const {
    return m_activeProfile;
}

} // namespace vmh
