#pragma once
#include "common/json.hpp"
#include <string>
#include <vector>

namespace vmh {

class ProfileEngine {
public:
    ProfileEngine(const json& defaultConfig);

    bool loadProfile(const std::string& profilePath);
    std::string createNewProfile(const std::string& profileType, const std::string& locale);
    bool applyProfile();
    bool rollbackProfile();

    json getProfileData() const;
    void saveProfile(const std::string& path);
    std::vector<std::string> listProfiles();

    struct GeneratedData {
        std::string fullName;
        std::string email;
        std::string phone;
        std::string address;
        std::string company;
        std::string occupation;
    };
    GeneratedData generateFakeData(const std::string& locale = "de_DE");

private:
    std::string generateName(const std::string& locale);
    std::string generateLastName(const std::string& locale);
    std::string generateEmail(const std::string& name, const std::string& company);
    std::string generatePhone(const std::string& locale);
    std::string generateAddress(const std::string& locale);
    std::string generateCompany(const std::string& locale);
    void validateProfile();
    std::string toLower(const std::string& str);
    std::string removeSpecialChars(const std::string& str);

    json m_config;
    json m_previousConfig;
    bool m_hasRollback = false;
    std::string m_currentProfilePath;
};

} // namespace vmh
