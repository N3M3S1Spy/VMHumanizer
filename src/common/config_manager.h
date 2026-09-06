#pragma once
#include "json.hpp"
#include <string>
#include <map>
#include <mutex>

namespace vmh {

class ConfigManager {
public:
    static ConfigManager& getInstance();

    bool loadConfig(const std::string& profileName);
    bool loadFromFile(const std::string& path);

    const json& getFullConfig() const { return m_config; }
    const json& getSection(const std::string& section) const;

    void setOverride(const std::string& key, const json& value);
    void clearOverrides();

    std::string getActiveProfileName() const;

private:
    ConfigManager() = default;
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

    json m_config;
    json m_overrides;
    std::string m_activeProfile;
    mutable std::mutex m_mutex;
};

} // namespace vmh
