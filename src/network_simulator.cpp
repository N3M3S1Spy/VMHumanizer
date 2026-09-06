#include "network_simulator.h"
#include "common/utils.h"
#include "windows/winapi_wrapper.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windns.h>
#include <iphlpapi.h>
#include <cstdio>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "dnsapi.lib")
#pragma comment(lib, "iphlpapi.lib")

namespace vmh {

NetworkSimulator::NetworkSimulator(const json& config) : m_config(config) {
    if (config.contains("network_simulator")) {
        const auto& ns = config["network_simulator"];
        m_enabled = ns.value<bool>("enabled", true);
        m_simulateDNS = ns.value<bool>("simulate_dns", true);
        m_simulateFirewall = ns.value<bool>("simulate_firewall", true);
        m_simulateShares = ns.value<bool>("simulate_network_shares", true);
        m_firewallBlockRate = ns.value<double>("firewall_block_rate", 0.05);

        if (ns.contains("dns_targets") && ns["dns_targets"].isArray()) {
            for (auto& t : ns["dns_targets"].elements()) {
                m_dnsTargets.push_back(t.getString());
            }
        }
        if (ns.contains("network_shares") && ns["network_shares"].isArray()) {
            for (auto& s : ns["network_shares"].elements()) {
                m_networkShares.push_back(s.getString());
            }
        }
    }

    if (m_dnsTargets.empty()) {
        m_dnsTargets = {
            "www.google.com", "www.microsoft.com", "github.com",
            "stackoverflow.com", "www.youtube.com", "outlook.office365.com",
            "teams.microsoft.com", "www.linkedin.com", "www.amazon.de",
            "www.heise.de", "news.ycombinator.com", "mail.google.com",
            "docs.microsoft.com", "www.wikipedia.org", "portal.azure.com",
        };
    }

    if (m_networkShares.empty()) {
        m_networkShares = {
            "\\\\fileserver\\shared",
            "\\\\nas\\documents",
            "\\\\printserver\\printers",
        };
    }

    // Initialize Winsock
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    logInfo("NetworkSimulator initialized (DNS=%s, Firewall=%s, Shares=%s)",
            m_simulateDNS ? "on" : "off",
            m_simulateFirewall ? "on" : "off",
            m_simulateShares ? "on" : "off");
}

void NetworkSimulator::simulateDNSLookup(const std::string& domain) {
    if (!m_enabled || !m_simulateDNS) return;

    DNS_RECORD* pDnsRecord = nullptr;
    std::wstring wDomain = utf8ToWide(domain);

    DNS_STATUS status = DnsQuery_W(wDomain.c_str(), DNS_TYPE_A,
                                    DNS_QUERY_STANDARD, nullptr,
                                    &pDnsRecord, nullptr);

    if (status == 0 && pDnsRecord) {
        char ipStr[64];
        IN_ADDR addr;
        addr.S_un.S_addr = pDnsRecord->Data.A.IpAddress;
        inet_ntop(AF_INET, &addr, ipStr, sizeof(ipStr));

        logDebug("DNS lookup: %s -> %s", domain.c_str(), ipStr);
        logNetworkEvent("dns_lookup", domain + " -> " + ipStr);

        DnsRecordListFree(pDnsRecord, DnsFreeRecordList);
    } else {
        logDebug("DNS lookup failed for %s (status=%ld)", domain.c_str(), status);
        logNetworkEvent("dns_lookup_failed", domain);
    }

    m_dnsLookupCount++;
}

void NetworkSimulator::simulateNetworkConnection(const std::string& destination,
                                                   int durationSeconds) {
    if (!m_enabled) return;

    logDebug("Simulating connection to %s for %ds", destination.c_str(), durationSeconds);
    logNetworkEvent("connection_established", destination);

    // Create a real TCP connection attempt (non-blocking, just to generate traffic)
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock != INVALID_SOCKET) {
        // Set non-blocking
        u_long nonBlocking = 1;
        ioctlsocket(sock, FIONBIO, &nonBlocking);

        struct addrinfo hints = {}, *result = nullptr;
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;

        int status = getaddrinfo(destination.c_str(), "443", &hints, &result);
        if (status == 0 && result) {
            connect(sock, result->ai_addr, static_cast<int>(result->ai_addrlen));
            // We don't need the connection to succeed — the attempt generates network events
            freeaddrinfo(result);
        }

        WinAPI::sleepMs(randomInt(100, 500));
        closesocket(sock);
    }

    m_connectionCount++;
    logNetworkEvent("connection_closed", destination);
}

void NetworkSimulator::injectFirewallEvent(bool allowed, const std::string& app, int port) {
    if (!m_enabled || !m_simulateFirewall) return;

    std::string action = allowed ? "ALLOW" : "DROP";
    std::string protocol = (port == 80 || port == 443) ? "TCP" : "UDP";

    std::wstring msg = utf8ToWide(
        "Windows Firewall " + action + ": " + app +
        " (" + protocol + " port " + std::to_string(port) + ")");

    WinAPI::writeEventLog(L"VMHumanizer",
                          allowed ? EVENTLOG_INFORMATION_TYPE : EVENTLOG_WARNING_TYPE,
                          allowed ? 2004u : 2003u,
                          msg);

    m_firewallEventCount++;
    logDebug("Firewall event: %s %s port %d (%s)", action.c_str(), app.c_str(), port, protocol.c_str());
}

std::string NetworkSimulator::generateMACAddress() {
    char mac[18];
    snprintf(mac, sizeof(mac), "%02x:%02x:%02x:%02x:%02x:%02x",
             randomInt(0, 255) & 0xFE, // Unicast
             randomInt(0, 255), randomInt(0, 255),
             randomInt(0, 255), randomInt(0, 255), randomInt(0, 255));
    return mac;
}

void NetworkSimulator::addARPEntry(const std::string& ip, const std::string& mac) {
    if (!m_enabled) return;
    // ARP manipulation requires admin — log instead
    logInfo("ARP entry: %s -> %s (requires admin for arp -s)", ip.c_str(), mac.c_str());
    logNetworkEvent("arp_entry", ip + " -> " + mac);
}

void NetworkSimulator::accessNetworkShare(const std::wstring& uncPath) {
    if (!m_enabled || !m_simulateShares) return;

    // Attempt to list the share (will fail if it doesn't exist, but generates network events)
    WIN32_FIND_DATAW findData;
    std::wstring searchPath = uncPath + L"\\*";
    HANDLE hFind = FindFirstFileW(searchPath.c_str(), &findData);

    if (hFind != INVALID_HANDLE_VALUE) {
        logDebug("Network share accessed: %s", wideToUtf8(uncPath).c_str());
        FindClose(hFind);
    } else {
        logDebug("Network share access attempt: %s (not available)", wideToUtf8(uncPath).c_str());
    }

    logNetworkEvent("share_access", wideToUtf8(uncPath));
}

void NetworkSimulator::simulateDNSBatch() {
    int count = randomInt(3, 8);
    for (int i = 0; i < count; ++i) {
        const auto& domain = m_dnsTargets[randomInt(0, static_cast<int>(m_dnsTargets.size()) - 1)];
        simulateDNSLookup(domain);
        WinAPI::sleepMs(randomInt(500, 3000));
    }
}

void NetworkSimulator::simulateFirewallBatch() {
    int count = randomInt(2, 5);
    std::vector<std::pair<std::string, int>> apps = {
        {"msedge.exe", 443}, {"chrome.exe", 443}, {"outlook.exe", 993},
        {"teams.exe", 443}, {"svchost.exe", 53}, {"explorer.exe", 445},
    };

    for (int i = 0; i < count; ++i) {
        auto& [app, port] = apps[randomInt(0, static_cast<int>(apps.size()) - 1)];
        bool allowed = randomDouble(0, 1) > m_firewallBlockRate;
        injectFirewallEvent(allowed, app, port);
        WinAPI::sleepMs(randomInt(200, 1000));
    }
}

void NetworkSimulator::runSimulationCycle() {
    if (!m_enabled) return;

    logInfo("Running network simulation cycle...");

    if (m_simulateDNS) simulateDNSBatch();
    if (m_simulateFirewall) simulateFirewallBatch();

    // Simulate some connections
    int connCount = randomInt(2, 5);
    for (int i = 0; i < connCount; ++i) {
        const auto& domain = m_dnsTargets[randomInt(0, static_cast<int>(m_dnsTargets.size()) - 1)];
        simulateNetworkConnection(domain, randomInt(5, 60));
        WinAPI::sleepMs(randomInt(1000, 5000));
    }

    // Try network shares
    if (m_simulateShares && !m_networkShares.empty()) {
        const auto& share = m_networkShares[randomInt(0, static_cast<int>(m_networkShares.size()) - 1)];
        accessNetworkShare(utf8ToWide(share));
    }

    // Add some ARP entries
    addARPEntry("192.168.1.1", generateMACAddress());
    addARPEntry("192.168.1." + std::to_string(randomInt(2, 254)), generateMACAddress());

    logInfo("Network simulation cycle complete (DNS=%d, FW=%d, Conn=%d)",
            m_dnsLookupCount, m_firewallEventCount, m_connectionCount);
}

void NetworkSimulator::logNetworkEvent(const std::string& eventType, const std::string& details) {
    logDebug("Network event [%s]: %s", eventType.c_str(), details.c_str());
}

NetworkSimulator::NetworkStats NetworkSimulator::getStats() const {
    return {
        m_connectionCount,
        m_dnsLookupCount,
        m_firewallEventCount,
        {"192.168.1.0/24"}
    };
}

} // namespace vmh
