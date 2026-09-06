#pragma once
#include "common/json.hpp"
#include <string>
#include <vector>
#include <windows.h>

namespace vmh {

class NetworkSimulator {
public:
    NetworkSimulator(const json& config);

    void simulateDNSLookup(const std::string& domain);
    void simulateNetworkConnection(const std::string& destination, int durationSeconds);
    void injectFirewallEvent(bool allowed, const std::string& app, int port);
    void addARPEntry(const std::string& ip, const std::string& mac);
    void accessNetworkShare(const std::wstring& uncPath);

    void runSimulationCycle();

    struct NetworkStats {
        int activeConnections;
        int dnsLookupsToday;
        int firewallEvents;
        std::vector<std::string> seenNetworks;
    };
    NetworkStats getStats() const;

private:
    void logNetworkEvent(const std::string& eventType, const std::string& details);
    std::string generateMACAddress();
    void simulateDNSBatch();
    void simulateFirewallBatch();

    json m_config;
    bool m_enabled = true;
    bool m_simulateDNS = true;
    bool m_simulateFirewall = true;
    bool m_simulateShares = true;
    double m_firewallBlockRate = 0.05;

    std::vector<std::string> m_dnsTargets;
    std::vector<std::string> m_networkShares;

    int m_dnsLookupCount = 0;
    int m_firewallEventCount = 0;
    int m_connectionCount = 0;
};

} // namespace vmh
