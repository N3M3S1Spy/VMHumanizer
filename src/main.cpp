#include <iostream>
#include <string>
#include <chrono>
#include <windows.h>
#include "common/json.hpp"
#include "common/utils.h"
#include "common/profile_loader.h"
#include "common/json_utils.h"
#include "mouse_behavior.h"
#include "keystroke_emulator.h"
#include "registry_mru_engine.h"
#include "event_log_injector.h"
#include "activity_engine.h"
#include "prefetch_generator.h"
#include "lnk_file_generator.h"
#include "profile_engine.h"
#include "network_simulator.h"
#include "logger.h"
#include "telemetry_pipeline.h"
#include "debug_ui.h"
#include "anomaly_detector.h"
#include "anomaly_corrector.h"

using namespace vmh;

void printBanner() {
    std::cout << R"(
 __     ____  __ _   _                             _
 \ \   / /  \/  | | | |_   _ _ __ ___   __ _ _ __ (_)_______ _ __
  \ \ / /| |\/| | |_| | | | | '_ ` _ \ / _` | '_ \| |_  / _ \ '__|
   \ V / | |  | |  _  | |_| | | | | | | (_| | | | | |/ /  __/ |
    \_/  |_|  |_|_| |_|\__,_|_| |_| |_|\__,_|_| |_|_/___\___|_|

    VMHumanizer v2.0 - VM Behavior Humanization Engine
    )" << std::endl;
}

void printUsage() {
    std::cout << "Usage: VMHumanizer.exe [options]\n\n"
              << "Options:\n"
              << "  --profile <name>    Profile to load (developer, office_worker, student, gamer)\n"
              << "\n  Tier 1 Features:\n"
              << "  --mouse             Run mouse behavior simulation\n"
              << "  --keystroke         Run keystroke emulation\n"
              << "  --registry          Populate registry MRU entries\n"
              << "  --eventlog          Inject event log entries\n"
              << "\n  Tier 2 Features:\n"
              << "  --activity          Start activity cycling engine\n"
              << "  --prefetch          Generate prefetch file artifacts\n"
              << "  --lnk               Generate LNK file artifacts\n"
              << "  --network           Run network traffic simulation\n"
              << "  --genprofile        Generate a new random profile\n"
              << "\n  Tier 4 Features:\n"
              << "  --dashboard         Launch real-time debug UI dashboard\n"
              << "  --anomaly-check     Run anomaly detection on recent logs\n"
              << "\n  General:\n"
              << "  --all               Run all features\n"
              << "  --demo              Run a quick demo of all features\n"
              << "  --help              Show this help\n"
              << std::endl;
}

void runMouseDemo(MouseBehavior& mouse) {
    Logger::getInstance().logInfo("mouse_behavior", "Starting mouse demo");
    std::cout << "\n[*] Mouse Behavior Demo\n";
    std::cout << "    Moving mouse in a pattern...\n";

    auto [startX, startY] = mouse.getCurrentPosition();
    std::cout << "    Current position: (" << startX << ", " << startY << ")\n";

    mouse.moveTo(500, 300, 800);
    std::cout << "    Moved to (500, 300)\n";

    mouse.moveTo(800, 500, 600);
    std::cout << "    Moved to (800, 500)\n";

    mouse.moveTo(300, 200, 700);
    std::cout << "    Moved to (300, 200)\n";

    mouse.moveTo(startX, startY, 500);
    std::cout << "    Returned to start position\n";
    std::cout << "[+] Mouse demo complete\n";
    Logger::getInstance().logInfo("mouse_behavior", "Mouse demo complete");
}

void runKeystrokeDemo(KeystrokeEmulator& keystroke) {
    Logger::getInstance().logInfo("keystroke_emulator", "Starting keystroke demo");
    std::cout << "\n[*] Keystroke Emulation Demo\n";
    std::cout << "    Will type a sample text (focus a text editor!)...\n";
    std::cout << "    Starting in 3 seconds...\n";
    WinAPI::sleepMs(3000);

    keystroke.typeString("Hello World! This is a test of the VMHumanizer keystroke emulator.", true);

    auto stats = keystroke.getStatistics();
    std::cout << "\n    Typing stats:\n";
    std::cout << "      Keys typed: " << stats.totalKeysTyped << "\n";
    std::cout << "      Avg IKD: " << stats.avgIKD << "ms\n";
    std::cout << "      Error rate: " << (stats.errorRate * 100.0) << "%\n";
    std::cout << "[+] Keystroke demo complete\n";
    Logger::getInstance().logInfo("keystroke_emulator", "Keystroke demo complete");
}

void runRegistryDemo(RegistryMRUEngine& registry) {
    Logger::getInstance().logInfo("registry_mru", "Starting registry MRU population");
    std::cout << "\n[*] Registry MRU Population\n";
    std::cout << "    Populating MRU entries...\n";

    registry.populateAllMRU();

    std::cout << "[+] Registry MRU population complete\n";
    std::cout << "    Check with: regedit -> HKCU\\Software\\Microsoft\\Internet Explorer\\TypedURLs\n";
    Logger::getInstance().logInfo("registry_mru", "Registry MRU population complete");
}

void runEventLogDemo(EventLogInjector& eventLog, const std::string& profile) {
    Logger::getInstance().logInfo("event_log_injector", "Starting event log injection for profile: " + profile);
    std::cout << "\n[*] Event Log Injection\n";
    std::cout << "    Injecting events for profile '" << profile << "'...\n";

    eventLog.populateEventsForProfile(profile, 20);

    std::cout << "[+] Event log injection complete\n";
    std::cout << "    Check with: Event Viewer -> Windows Logs\n";
    Logger::getInstance().logInfo("event_log_injector", "Event log injection complete");
}

void runActivityDemo(ActivityEngine& engine) {
    Logger::getInstance().logInfo("activity_engine", "Starting activity engine demo");
    std::cout << "\n[*] Activity Engine Demo\n";
    std::cout << "    Starting activity engine for 30 seconds...\n";

    engine.start();
    WinAPI::sleepMs(30000);

    auto stats = engine.getStats();
    std::cout << "    Current state: " << engine.getStateString() << "\n";
    std::cout << "    Activities completed: " << stats.totalActivities << "\n";
    std::cout << "    Breaks taken: " << stats.totalBreaks << "\n";

    engine.stop();
    std::cout << "[+] Activity engine demo complete\n";
    Logger::getInstance().logInfo("activity_engine", "Activity engine demo complete");
}

void runPrefetchDemo(PrefetchGenerator& prefetch) {
    Logger::getInstance().logInfo("prefetch_generator", "Starting prefetch generation");
    std::cout << "\n[*] Prefetch File Generation\n";
    std::cout << "    Generating prefetch artifacts...\n";

    prefetch.generateAllPrefetches();

    std::cout << "[+] Prefetch generation complete\n";
    std::cout << "    Check: C:\\Windows\\Prefetch\\ (requires admin)\n";
    Logger::getInstance().logInfo("prefetch_generator", "Prefetch generation complete");
}

void runLNKDemo(LNKFileGenerator& lnk) {
    Logger::getInstance().logInfo("lnk_generator", "Starting LNK file generation");
    std::cout << "\n[*] LNK File Generation\n";
    std::cout << "    Creating recent document shortcuts...\n";

    lnk.populateRecentDocuments();
    lnk.populateDesktopShortcuts();

    std::cout << "[+] LNK file generation complete\n";
    std::cout << "    Check: Recent Documents & Desktop\n";
    Logger::getInstance().logInfo("lnk_generator", "LNK file generation complete");
}

void runNetworkDemo(NetworkSimulator& network) {
    Logger::getInstance().logInfo("network_simulator", "Starting network simulation");
    std::cout << "\n[*] Network Traffic Simulation\n";
    std::cout << "    Running simulation cycle...\n";

    network.runSimulationCycle();

    auto stats = network.getStats();
    std::cout << "    DNS lookups: " << stats.dnsLookupsToday << "\n";
    std::cout << "    Firewall events: " << stats.firewallEvents << "\n";
    std::cout << "    Connections: " << stats.activeConnections << "\n";
    std::cout << "[+] Network simulation complete\n";
    Logger::getInstance().logInfo("network_simulator", "Network simulation complete");
}

void runProfileGenDemo(ProfileEngine& profileEng, const std::string& profileType) {
    Logger::getInstance().logInfo("profile_engine", "Starting profile generation");
    std::cout << "\n[*] Profile Generation\n";

    auto data = profileEng.generateFakeData("de_DE");
    std::cout << "    Generated identity:\n";
    std::cout << "      Name:    " << data.fullName << "\n";
    std::cout << "      Email:   " << data.email << "\n";
    std::cout << "      Phone:   " << data.phone << "\n";
    std::cout << "      Company: " << data.company << "\n";
    std::cout << "      Address: " << data.address << "\n";

    std::string path = profileEng.createNewProfile(profileType, "de_DE");
    std::cout << "    Profile saved: " << path << "\n";
    std::cout << "[+] Profile generation complete\n";
    Logger::getInstance().logInfo("profile_engine", "Profile generation complete: " + path);
}

void runAnomalyCheck(const json& config) {
    Logger::getInstance().logInfo("anomaly_detector", "Starting anomaly check on recent logs");
    std::cout << "\n[*] Anomaly Detection Check\n";

    AnomalyDetector detector(config);
    AnomalyCorrector corrector(config);

    auto logEntries = JsonUtils::readJsonLines(Logger::getInstance().getActivitiesLogPath(), 100);

    if (logEntries.empty()) {
        std::cout << "    No log entries found to analyze.\n";
        std::cout << "    Run some features first to generate log data.\n";
        return;
    }

    std::cout << "    Analyzing " << logEntries.size() << " log entries...\n";

    json eventsArray = json::array();
    for (const auto& e : logEntries) eventsArray.push_back(e);

    auto anomalies = detector.detectAnomalies(eventsArray);

    if (anomalies.empty()) {
        std::cout << "    No anomalies detected. Behavior looks realistic.\n";
    } else {
        std::cout << "    Detected " << anomalies.size() << " anomalies:\n\n";
        for (const auto& a : anomalies) {
            const char* sevLabel = a.severity > 0.7 ? "SEVERE" :
                                  (a.severity > 0.3 ? "MODERATE" : "MINOR");
            std::cout << "    [" << sevLabel << "] " << a.type << ": " << a.description << "\n";
            std::cout << "      Severity: " << a.severity << "  Suggestion: " << a.suggestion << "\n\n";
        }

        auto corrections = corrector.getRecommendedCorrections(anomalies);
        std::cout << "    Recommended corrections:\n";
        for (const auto& c : corrections) {
            std::cout << "      Strategy: " << c.strategy
                      << "  Param: " << c.parameter
                      << "  Affects: " << c.affectedActivities << " activities\n";
        }

        corrector.correctAnomalies(anomalies);
        std::cout << "\n    Auto-corrections applied.\n";
    }

    std::cout << "[+] Anomaly check complete\n";
    Logger::getInstance().logInfo("anomaly_detector", "Anomaly check complete, found " +
                                  std::to_string(anomalies.size()) + " anomalies");
}

void runDashboard(const json& config, const std::string& profileName) {
    Logger::getInstance().logInfo("debug_ui", "Launching dashboard");

    DebugUI ui(config);
    TelemetryPipeline telemetry;
    telemetry.initialize(config);
    AnomalyDetector detector(config);
    AnomalyCorrector corrector(config);

    ui.setProfileName(profileName);
    ui.setStatus("Running", 0);

    auto startTime = std::chrono::steady_clock::now();

    Logger::getInstance().setEventCallback([&ui](const json& event) {
        std::string ts = event.value<std::string>("timestamp", "");
        if (ts.size() > 11) ts = ts.substr(11, 8);
        std::string msg = event.value<std::string>("message", "");
        if (msg.empty()) msg = event.value<std::string>("event_type", "event");

        std::string level = event.value<std::string>("level", "INFO");
        ConsoleColor color = COLOR_WHITE;
        if (level == "ERROR") color = COLOR_RED;
        else if (level == "WARN") color = COLOR_YELLOW;
        else if (level == "DEBUG") color = COLOR_GRAY;
        else color = COLOR_CYAN;

        ui.addEvent(ts, msg, color);
    });

    telemetry.startMetricsCollection();

    ui.setCommandCallback([&](char key) {
        if (key == 'd' || key == 'D') {
            runAnomalyCheck(config);
        }
    });

    std::thread statsUpdater([&]() {
        while (ui.isRunning()) {
            auto stats = telemetry.getStats();
            ui.updateMetrics(stats);

            auto elapsed = std::chrono::steady_clock::now() - startTime;
            int secs = static_cast<int>(std::chrono::duration_cast<std::chrono::seconds>(elapsed).count());
            ui.setUptimeSeconds(secs);

            auto logStats = Logger::getInstance().getStats();
            ui.setTotalActivities(logStats.totalEvents);
            ui.setErrorCount(logStats.errors);

            Sleep(1000);
        }
    });

    ui.run();

    telemetry.stopMetricsCollection();
    if (statsUpdater.joinable()) statsUpdater.join();

    Logger::getInstance().logInfo("debug_ui", "Dashboard closed");
}

int main(int argc, char* argv[]) {
    printBanner();

    std::string profileName = "developer";
    bool runMouse = false, runKeystroke = false, runRegistry = false, runEventLog = false;
    bool runActivity = false, runPrefetch = false, runLNK = false, runNetwork = false;
    bool runGenProfile = false;
    bool launchDashboard = false, doAnomalyCheck = false;
    bool runAll = false, runDemo = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--profile" && i + 1 < argc) {
            profileName = argv[++i];
        } else if (arg == "--mouse") { runMouse = true; }
        else if (arg == "--keystroke") { runKeystroke = true; }
        else if (arg == "--registry") { runRegistry = true; }
        else if (arg == "--eventlog") { runEventLog = true; }
        else if (arg == "--activity") { runActivity = true; }
        else if (arg == "--prefetch") { runPrefetch = true; }
        else if (arg == "--lnk") { runLNK = true; }
        else if (arg == "--network") { runNetwork = true; }
        else if (arg == "--genprofile") { runGenProfile = true; }
        else if (arg == "--dashboard") { launchDashboard = true; }
        else if (arg == "--anomaly-check") { doAnomalyCheck = true; }
        else if (arg == "--all") { runAll = true; }
        else if (arg == "--demo") { runDemo = true; }
        else if (arg == "--help") { printUsage(); return 0; }
    }

    bool anySelected = runMouse || runKeystroke || runRegistry || runEventLog ||
                       runActivity || runPrefetch || runLNK || runNetwork ||
                       runGenProfile || launchDashboard || doAnomalyCheck ||
                       runAll || runDemo;

    if (!anySelected) {
        printUsage();
        return 0;
    }

    logInit("data\\logs\\vmhumanizer.log");

    // Initialize structured logging (Tier 4)
    json loggingConfig = json::object();
    json loggingSection = json::object();
    loggingSection["log_level"] = "INFO";
    loggingSection["async_writes"] = true;
    loggingSection["queue_size"] = 1000;
    json files = json::object();
    files["activities"] = "data/logs/activities.log";
    files["metrics"] = "data/logs/metrics.log";
    files["anomalies"] = "data/logs/anomalies.log";
    loggingSection["files"] = files;
    json rotation = json::object();
    rotation["max_file_size_mb"] = 100;
    rotation["retention_days"] = 30;
    loggingSection["rotation"] = rotation;
    loggingConfig["logging"] = loggingSection;

    Logger::getInstance().initialize(loggingConfig);
    Logger::getInstance().logInfo("main", "VMHumanizer starting with profile: " + profileName);

    ProfileLoader loader;
    if (!loader.loadProfile(profileName)) {
        logWarning("Could not load profile '%s', using defaults", profileName.c_str());
        Logger::getInstance().logWarn("main", "Could not load profile, using defaults");
    }

    const json& config = loader.getConfig();
    std::cout << "[*] Profile loaded: " << profileName << "\n";

    try {
        // Dashboard mode (blocks until user quits)
        if (launchDashboard) {
            runDashboard(config, profileName);
            Logger::getInstance().shutdown();
            return 0;
        }

        // Anomaly check mode
        if (doAnomalyCheck) {
            runAnomalyCheck(config);
            Logger::getInstance().shutdown();
            return 0;
        }

        // Tier 1 Features
        if (runAll || runDemo || runMouse) {
            MouseBehavior mouse(config);
            runMouseDemo(mouse);
        }

        if (runAll || runDemo || runKeystroke) {
            KeystrokeEmulator keystroke(config);
            runKeystrokeDemo(keystroke);
        }

        if (runAll || runDemo || runRegistry) {
            RegistryMRUEngine registry(config);
            runRegistryDemo(registry);
        }

        if (runAll || runDemo || runEventLog) {
            EventLogInjector eventLog(config);
            runEventLogDemo(eventLog, profileName);
        }

        // Tier 2 Features
        if (runAll || runDemo || runActivity) {
            MouseBehavior mouse(config);
            KeystrokeEmulator keystroke(config);
            ActivityEngine activity(config);
            activity.setMouseBehavior(&mouse);
            activity.setKeystrokeEmulator(&keystroke);
            runActivityDemo(activity);
        }

        if (runAll || runDemo || runPrefetch) {
            PrefetchGenerator prefetch(config);
            runPrefetchDemo(prefetch);
        }

        if (runAll || runDemo || runLNK) {
            LNKFileGenerator lnk(config);
            runLNKDemo(lnk);
        }

        if (runAll || runDemo || runNetwork) {
            NetworkSimulator network(config);
            runNetworkDemo(network);
        }

        if (runGenProfile) {
            ProfileEngine profileEng(config);
            runProfileGenDemo(profileEng, profileName);
        }

    } catch (const VMHException& e) {
        Logger::getInstance().logException("VMHException", e.what(), "main");
        logError("VMHumanizer error: %s", e.what());
        std::cerr << "[!] Error: " << e.what() << std::endl;
        Logger::getInstance().shutdown();
        return 1;
    } catch (const std::exception& e) {
        Logger::getInstance().logException("std::exception", e.what(), "main");
        logError("Unexpected error: %s", e.what());
        std::cerr << "[!] Unexpected error: " << e.what() << std::endl;
        Logger::getInstance().shutdown();
        return 1;
    }

    std::cout << "\n[+] VMHumanizer complete.\n";
    Logger::getInstance().logInfo("main", "VMHumanizer finished successfully");
    Logger::getInstance().shutdown();
    logInfo("VMHumanizer finished successfully");
    return 0;
}
