#pragma once
#include "json.hpp"
#include "types.h"
#include <string>

namespace vmh {

class ProfileLoader {
public:
    ProfileLoader();

    bool loadProfile(const std::string& profileName);
    bool loadFromFile(const std::string& path);

    const json& getConfig() const { return m_config; }
    const json& getMouseConfig() const;
    const json& getKeystrokeConfig() const;
    const json& getRegistryMRUConfig() const;
    const json& getEventLogConfig() const;

    std::string getProfileName() const;
    std::string getProfileType() const;

    MouseConfig parseMouseConfig() const;
    KeystrokeConfig parseKeystrokeConfig() const;
    RegistryMRUConfig parseRegistryMRUConfig() const;
    EventLogConfig parseEventLogConfig() const;

    static json createDefaultConfig();

private:
    json m_config;
    bool m_loaded = false;
};

} // namespace vmh
