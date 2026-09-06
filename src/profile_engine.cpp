#include "profile_engine.h"
#include "common/utils.h"
#include <filesystem>
#include <algorithm>
#include <cctype>

namespace vmh {

static const std::vector<std::string> DE_FIRST_NAMES = {
    "Max", "Lukas", "Leon", "Finn", "Jonas", "Noah", "Elias", "Paul",
    "Ben", "Felix", "Alexander", "Julian", "Moritz", "David", "Niklas",
    "Anna", "Marie", "Sophie", "Lena", "Laura", "Sarah", "Lisa",
    "Julia", "Katharina", "Hannah", "Mia", "Emma", "Lea", "Clara", "Nina",
};

static const std::vector<std::string> EN_FIRST_NAMES = {
    "James", "John", "Robert", "Michael", "William", "David", "Richard",
    "Joseph", "Thomas", "Charles", "Daniel", "Matthew", "Anthony",
    "Mary", "Patricia", "Jennifer", "Linda", "Barbara", "Elizabeth",
    "Jessica", "Sarah", "Karen", "Nancy", "Lisa", "Betty", "Emily",
};

static const std::vector<std::string> DE_LAST_NAMES = {
    "Mueller", "Schmidt", "Schneider", "Fischer", "Weber", "Meyer",
    "Wagner", "Becker", "Schulz", "Hoffmann", "Koch", "Richter",
    "Klein", "Wolf", "Schroeder", "Neumann", "Schwarz", "Braun",
    "Zimmermann", "Krueger", "Hartmann", "Lange", "Werner", "Lehmann",
};

static const std::vector<std::string> EN_LAST_NAMES = {
    "Smith", "Johnson", "Williams", "Brown", "Jones", "Garcia",
    "Miller", "Davis", "Rodriguez", "Martinez", "Anderson", "Taylor",
    "Thomas", "Moore", "Jackson", "Martin", "Lee", "Thompson",
    "White", "Harris", "Clark", "Lewis", "Robinson", "Walker",
};

static const std::vector<std::string> DE_COMPANIES = {
    "TechVision GmbH", "DataFlow AG", "CloudNine Solutions", "ByteForge UG",
    "NetWorks GmbH", "CodeCraft AG", "DigitalPioniere GmbH", "SmartSystems AG",
    "InnoTech Solutions", "CyberGuard GmbH", "DataMind AG", "WebWerkstatt UG",
};

static const std::vector<std::string> EN_COMPANIES = {
    "TechVentures Inc", "DataCore Solutions", "CloudBridge Corp",
    "ByteStream Technologies", "NetPulse Systems", "CodeForge LLC",
    "DigitalEdge Inc", "SmartPath Corp", "InnoWave Technologies",
};

static const std::vector<std::string> DE_STREETS = {
    "Hauptstrasse", "Bahnhofstrasse", "Gartenstrasse", "Schulstrasse",
    "Kirchstrasse", "Berliner Strasse", "Muenchner Strasse", "Lindenstrasse",
};

static const std::vector<std::string> DE_CITIES = {
    "Berlin", "Hamburg", "Muenchen", "Koeln", "Frankfurt am Main",
    "Stuttgart", "Duesseldorf", "Leipzig", "Dortmund", "Essen",
};

static const std::vector<std::string> EN_STREETS = {
    "Main Street", "Oak Avenue", "Elm Street", "Park Road",
    "Cedar Lane", "Maple Drive", "Pine Street", "Washington Blvd",
};

static const std::vector<std::string> EN_CITIES = {
    "New York", "Los Angeles", "Chicago", "Houston", "Phoenix",
    "San Antonio", "San Diego", "Dallas", "San Jose", "Austin",
};

ProfileEngine::ProfileEngine(const json& defaultConfig) : m_config(defaultConfig) {
    logInfo("ProfileEngine initialized");
}

bool ProfileEngine::loadProfile(const std::string& profilePath) {
    try {
        if (!std::filesystem::exists(profilePath)) {
            logError("Profile not found: %s", profilePath.c_str());
            return false;
        }
        m_previousConfig = m_config;
        m_hasRollback = true;
        m_config = json::loadFromFile(profilePath);
        m_currentProfilePath = profilePath;
        validateProfile();
        logInfo("Profile loaded: %s", profilePath.c_str());
        return true;
    } catch (const std::exception& e) {
        logError("Failed to load profile: %s", e.what());
        return false;
    }
}

std::string ProfileEngine::generateName(const std::string& locale) {
    const auto& names = (locale == "de_DE") ? DE_FIRST_NAMES : EN_FIRST_NAMES;
    return names[randomInt(0, static_cast<int>(names.size()) - 1)];
}

std::string ProfileEngine::generateLastName(const std::string& locale) {
    const auto& names = (locale == "de_DE") ? DE_LAST_NAMES : EN_LAST_NAMES;
    return names[randomInt(0, static_cast<int>(names.size()) - 1)];
}

std::string ProfileEngine::generateCompany(const std::string& locale) {
    const auto& companies = (locale == "de_DE") ? DE_COMPANIES : EN_COMPANIES;
    return companies[randomInt(0, static_cast<int>(companies.size()) - 1)];
}

std::string ProfileEngine::toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](char c) { return static_cast<char>(std::tolower(c)); });
    return result;
}

std::string ProfileEngine::removeSpecialChars(const std::string& str) {
    std::string result;
    for (char c : str) {
        if (std::isalnum(c) || c == '.' || c == '-' || c == '_') {
            result += c;
        }
    }
    return result;
}

std::string ProfileEngine::generateEmail(const std::string& name, const std::string& company) {
    std::string cleanName = removeSpecialChars(toLower(name));
    std::string domain = removeSpecialChars(toLower(company));
    // Remove GmbH/AG/etc from domain
    for (auto& suffix : {"gmbh", "ag", "ug", "inc", "corp", "llc"}) {
        size_t pos = domain.find(suffix);
        if (pos != std::string::npos) domain = domain.substr(0, pos);
    }
    // Remove trailing whitespace
    while (!domain.empty() && (domain.back() == ' ' || domain.back() == '.'))
        domain.pop_back();

    std::vector<std::string> patterns = {
        cleanName + "@" + domain + ".de",
        cleanName + "@" + domain + ".com",
        cleanName + "." + std::to_string(randomInt(1, 99)) + "@" + domain + ".de",
    };
    return patterns[randomInt(0, static_cast<int>(patterns.size()) - 1)];
}

std::string ProfileEngine::generatePhone(const std::string& locale) {
    if (locale == "de_DE") {
        return "+49 " + std::to_string(randomInt(151, 179)) + " " +
               std::to_string(randomInt(1000000, 9999999));
    }
    return "+1 " + std::to_string(randomInt(200, 999)) + " " +
           std::to_string(randomInt(100, 999)) + " " +
           std::to_string(randomInt(1000, 9999));
}

std::string ProfileEngine::generateAddress(const std::string& locale) {
    if (locale == "de_DE") {
        const auto& street = DE_STREETS[randomInt(0, static_cast<int>(DE_STREETS.size()) - 1)];
        const auto& city = DE_CITIES[randomInt(0, static_cast<int>(DE_CITIES.size()) - 1)];
        return street + " " + std::to_string(randomInt(1, 200)) + ", " +
               std::to_string(randomInt(10000, 99999)) + " " + city;
    }
    const auto& street = EN_STREETS[randomInt(0, static_cast<int>(EN_STREETS.size()) - 1)];
    const auto& city = EN_CITIES[randomInt(0, static_cast<int>(EN_CITIES.size()) - 1)];
    return std::to_string(randomInt(100, 9999)) + " " + street + ", " + city;
}

ProfileEngine::GeneratedData ProfileEngine::generateFakeData(const std::string& locale) {
    GeneratedData data;
    std::string firstName = generateName(locale);
    std::string lastName = generateLastName(locale);
    data.fullName = firstName + " " + lastName;
    data.company = generateCompany(locale);
    data.email = generateEmail(firstName + "." + lastName, data.company);
    data.phone = generatePhone(locale);
    data.address = generateAddress(locale);

    std::vector<std::string> occupations = {
        "Software Developer", "Project Manager", "Data Analyst",
        "System Administrator", "UX Designer", "DevOps Engineer",
        "IT Consultant", "Product Owner", "QA Engineer", "Architect",
    };
    data.occupation = occupations[randomInt(0, static_cast<int>(occupations.size()) - 1)];

    return data;
}

std::string ProfileEngine::createNewProfile(const std::string& profileType,
                                             const std::string& locale) {
    auto data = generateFakeData(locale);

    auto profile = json::object();
    profile["profile_name"] = profileType + "_" + std::to_string(randomInt(100, 999));
    profile["profile_type"] = profileType;

    auto attrs = json::object();
    attrs["full_name"] = data.fullName;
    attrs["email"] = data.email;
    attrs["phone"] = data.phone;
    attrs["address"] = data.address;
    attrs["company"] = data.company;
    attrs["occupation"] = data.occupation;
    attrs["locale"] = locale;
    profile["user_attributes"] = attrs;

    // Copy existing feature configs or create defaults
    if (m_config.contains("mouse")) profile["mouse"] = m_config["mouse"];
    if (m_config.contains("keystroke")) profile["keystroke"] = m_config["keystroke"];
    if (m_config.contains("registry_mru")) profile["registry_mru"] = m_config["registry_mru"];
    if (m_config.contains("event_log")) profile["event_log"] = m_config["event_log"];

    std::string filename = profile["profile_name"].getString() + ".json";
    std::string path = getDataDirectory() + "\\profiles\\" + filename;

    m_previousConfig = m_config;
    m_hasRollback = true;
    m_config = profile;
    m_currentProfilePath = path;

    saveProfile(path);
    logInfo("New profile created: %s (%s)", filename.c_str(), data.fullName.c_str());
    return path;
}

bool ProfileEngine::applyProfile() {
    validateProfile();
    logInfo("Profile applied: %s", m_config.value<std::string>("profile_name", "unknown").c_str());
    return true;
}

bool ProfileEngine::rollbackProfile() {
    if (!m_hasRollback) {
        logWarning("No rollback state available");
        return false;
    }
    m_config = m_previousConfig;
    m_hasRollback = false;
    logInfo("Profile rolled back");
    return true;
}

json ProfileEngine::getProfileData() const {
    return m_config;
}

void ProfileEngine::saveProfile(const std::string& path) {
    try {
        std::filesystem::create_directories(std::filesystem::path(path).parent_path());
        m_config.saveToFile(path);
        logInfo("Profile saved: %s", path.c_str());
    } catch (const std::exception& e) {
        logError("Failed to save profile: %s", e.what());
    }
}

std::vector<std::string> ProfileEngine::listProfiles() {
    std::vector<std::string> profiles;
    std::string dir = getDataDirectory() + "\\profiles";
    try {
        for (auto& entry : std::filesystem::directory_iterator(dir)) {
            if (entry.path().extension() == ".json") {
                profiles.push_back(entry.path().stem().string());
            }
        }
    } catch (...) {}
    return profiles;
}

void ProfileEngine::validateProfile() {
    if (!m_config.contains("profile_name")) {
        logWarning("Profile missing 'profile_name', using default");
        m_config["profile_name"] = "unnamed_profile";
    }
    if (!m_config.contains("profile_type")) {
        logWarning("Profile missing 'profile_type', using 'developer'");
        m_config["profile_type"] = "developer";
    }
}

} // namespace vmh
