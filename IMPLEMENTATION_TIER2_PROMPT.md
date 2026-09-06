# 🚀 TIER 2 Implementation Prompt (For Claude Opus)

**Projekt:** VMHumanizer - Advanced Automation & Behavior Engine  
**Tier:** Tier 2 (Advanced Automation)  
**Tech-Stack:** Pure C++ (C++17/20) + Windows API (WINAPI)  
**Prerequisites:** TIER 1 must be completed first  
**Target Platform:** Windows 10/11  

---

## 📋 Features to Implement (Tier 2)

### Feature #5: Intelligent Task-Switching & Activity Cycling Engine
### Feature #6: Prefetch & LNK File Artefact Generator
### Feature #7: Configurable User Profile Generator with Dynamic Data Population
### Feature #8: Network Traffic & Connection Pattern Simulator

---

## 🏗️ Project Structure Extension

Tier 2 builds on Tier 1 structure. New additions:

```
VMHumanizer/
├── src/
│   ├── [Tier 1 modules already exist]
│   ├── activity_engine.cpp               # Feature #5
│   ├── activity_engine.h
│   ├── prefetch_generator.cpp            # Feature #6
│   ├── prefetch_generator.h
│   ├── lnk_file_generator.cpp            # Feature #6 (part 2)
│   ├── lnk_file_generator.h
│   ├── profile_engine.cpp                # Feature #7
│   ├── profile_engine.h
│   ├── network_simulator.cpp             # Feature #8
│   ├── network_simulator.h
│   └── common/
│       ├── config_manager.cpp            # NEW - centralized config
│       ├── config_manager.h
│       ├── timer_utils.cpp               # NEW - timing utilities
│       └── timer_utils.h
├── data/
│   ├── profiles/
│   │   ├── developer.json                # Enhanced with activity patterns
│   │   ├── office_worker.json
│   │   ├── student.json
│   │   └── gamer.json                    # NEW profile example
│   ├── templates/
│   │   ├── activity_templates.json       # NEW - task definitions
│   │   ├── prefetch_template.bin         # NEW - prefetch structure
│   │   └── lnk_template.bin              # NEW - LNK structure
│   └── network/
│       └── network_patterns.json         # NEW - network activity templates
├── CMakeLists.txt                        # Updated for new modules
└── [other files from Tier 1]
```

---

## 🎯 Detailed Feature Specifications

### Feature #5: Intelligent Task-Switching & Activity Cycling Engine

**Objective:** Automate realistic, context-aware user activity simulation that runs continuously.

**Requirements:**

1. **Activity Profile System**
   - Define typical activity sequences for user types
   - Example profile (JSON):
   ```json
   {
     "profile": "developer",
     "activities": [
       {
         "name": "Morning Routine",
         "time_range": [8, 10],
         "tasks": [
           {"app": "edge.exe", "action": "browse_github", "duration": [300, 600]},
           {"app": "cmd.exe", "action": "git_pull", "duration": [120, 180]},
           {"app": "vscode.exe", "action": "code_review", "duration": [1800, 3600]}
         ],
         "break_after": 3600
       },
       {
         "name": "Afternoon Coding",
         "time_range": [13, 17],
         "tasks": [
           {"app": "vscode.exe", "action": "development", "duration": [3600, 7200]},
           {"app": "edge.exe", "action": "stackoverflow_search", "duration": [300, 900]}
         ],
         "break_after": 3600
       }
     ]
   }
   ```

2. **State Machine Implementation**
   - States: IDLE, ACTIVE_TASK, SWITCHING, BREAK, WAITING
   - Transitions based on:
     - Time of day
     - Activity duration
     - Random "distraction" events
     - System resource usage

3. **Task Execution Engine**
   - Launch applications via ShellExecute()
   - Simulate actions within apps (mouse moves, clicks, typing)
   - Track activity duration + add natural variation
   - Log all activities for audit trail

4. **Time-Aware Behavior**
   - **Morning (6-12):** Higher intensity, faster typing, frequent task switching
   - **Afternoon (12-18):** Moderate intensity, occasional breaks, slower pace after lunch
   - **Evening (18-23):** Lower intensity, more browsing/entertainment
   - **Night (23-6):** Minimal activity (occasional idle), maybe some late-night work
   - **Weekends:** Different pattern entirely (more gaming/entertainment, less work)

5. **Break Injection**
   - Coffee breaks (5-15 min) - computer idle
   - Lunch breaks (30-60 min) - system in low-power state
   - "Bio breaks" (2-5 min) - quick absence
   - Distraction breaks (user "leaves to get something")

6. **Realistic Task Durations**
   - Tasks don't always complete perfectly (sometimes interrupted)
   - Some tasks take longer than expected (user debugging)
   - Add variation: mean ± 20-30%
   - Never exact same duration twice

7. **Activity Logging**
   - Each activity logged with: timestamp, app, action, actual duration, issues
   - Enable event/audit trail review
   - Export to JSON for analysis

8. **Configuration:**
   ```json
   {
     "activity_engine": {
       "enabled": true,
       "profile": "developer",
       "timezone_offset": 1,
       "auto_start_on_boot": false,
       "break_frequency_hours": 1.5,
       "distraction_probability": 0.15,
       "activity_intensity": 0.8,
       "log_activities": true
     }
   }
   ```

**Function Signatures (C++):**
```cpp
class ActivityEngine {
  public:
    ActivityEngine(const json& config);
    
    // Start the activity loop (runs continuously)
    void start();
    
    // Stop the activity loop gracefully
    void stop();
    
    // Current state
    enum State { IDLE, ACTIVE, SWITCHING, BREAK, WAITING };
    State getCurrentState();
    
    // Manual activity injection (for testing)
    void executeActivity(const std::string& activityName);
    
    // Get current activity stats
    struct ActivityStats {
      std::string currentApp;
      int elapsedSeconds;
      std::string currentAction;
    };
    ActivityStats getStats();
    
  private:
    void stateLoop();
    void selectNextActivity();
    void executeTask(const json& task);
    void injectBreak(const std::string& breakType);
    json getActivitiesForTimeWindow(int currentHour);
};
```

**Testing Criteria:**
- Engine continuously runs without crashes (8+ hour simulation)
- Activities logged correctly with realistic timing
- Time-of-day patterns observable in activity log
- Break injection working (apps close, mouse stops)
- CPU/Memory stable over long run

---

### Feature #6: Prefetch & LNK File Artefact Generator

**Objective:** Generate authentic Windows forensic artifacts (Prefetch & LNK files).

#### Part A: Prefetch File Generator

**Requirements:**

1. **Prefetch File Format (PF)**
   - Path: `C:\Windows\Prefetch\{AppName}.exe-{GUID}.pf`
   - Binary format version 17 (Windows 10+)
   - Structure:
     - **Header:** Magic (0x414353..), version, file size
     - **Metrics:** Last run time, run count, first run time
     - **File Info:** List of DLLs/files loaded
     - **Trace Info:** CPU usage, I/O operations
   - Timestamps must be in FILETIME format
   - Run count should vary (1-50 depending on app)

2. **Realistic File Dependencies**
   - System DLLs (kernel32.dll, ntdll.dll, etc.)
   - Framework DLLs (if applicable)
   - Application-specific libraries
   - Config files loaded
   - Do NOT include files that don't make sense for the app

3. **Multiple Prefetch Entries**
   - Create PF files for common apps (notepad.exe, calc.exe, explorer.exe, etc.)
   - Different apps per profile (Developer: vscode.exe, git.exe, cmd.exe)
   - Run counts reflect realistic usage

4. **Timestamp Realism**
   - Last run time should be recent (within last 7 days for active apps)
   - Run counts correlate with usage pattern (notepad: 50+, rare_app: 1)
   - First run time older than last run (within 30 days)
   - No future timestamps

5. **Automatic Prefetch Cleanup**
   - Remove oldest prefetch entries when cache exceeds limit (limit ~128 files)
   - Preserve recent application prefetches
   - Realistic aging pattern

6. **Configuration:**
   ```json
   {
     "prefetch": {
       "enabled": true,
       "apps_to_generate": [
         {"name": "notepad.exe", "run_count": [50, 150]},
         {"name": "calc.exe", "run_count": [20, 80]},
         {"name": "cmd.exe", "run_count": [30, 200]},
         {"name": "vscode.exe", "run_count": [50, 300]}
       ],
       "max_prefetch_files": 128,
       "include_last_access_time": true
     }
   }
   ```

**Function Signatures (C++):**
```cpp
class PrefetchGenerator {
  public:
    PrefetchGenerator(const json& config);
    
    // Generate all prefetch files
    void generateAllPrefetches();
    
    // Generate for specific app
    void generatePrefetchForApp(const std::wstring& appName, int runCount);
    
    // Add file dependency to prefetch
    void addFileDependency(const std::wstring& prefetchName, 
                           const std::wstring& filePath);
    
    // Cleanup old prefetch entries
    void cleanupOldPrefetches();
    
    // Clear all generated prefetches (testing)
    void clearAllPrefetches();
    
  private:
    struct PrefetchHeader { /* binary structure */ };
    void writePrefetchFile(const std::wstring& filename, 
                           const PrefetchHeader& header,
                           const std::vector<std::wstring>& files);
};
```

#### Part B: LNK File Generator

**Requirements:**

1. **LNK File Format (Shell Link)**
   - Path: `%AppData%\Microsoft\Windows\Recent\` or `Desktop\`
   - Binary format with proper structure:
     - **File Header:** Magic (0x4C), GUID, flags
     - **LinkTarget Specification:** Target path, arguments, icon
     - **Shell Item Identifier List:** Target location in file system
     - **Extra Data Block:** MachineID, NetBIOS name, metadata
   - Timestamps in Windows FILETIME format

2. **Realistic LNK Properties**
   - Target: Actual document/executable paths
   - Arguments: Realistic command-line args
   - Working Directory: Sensible defaults
   - Icon: Correct icon index for file type
   - Description: User-friendly description

3. **LNK Placement Strategy**
   - **Recent Documents:** `%AppData%\Microsoft\Windows\Recent\`
   - **Recent Tasks:** `%AppData%\Microsoft\Windows\Recent\AutomaticDestinations\`
   - **Shortcuts on Desktop:** `%UserProfile%\Desktop\`
   - Different LNK files per profile type

4. **Example LNK Entries by Profile:**
   - **Developer:** project_folder.lnk, github_repo.lnk, vscode_projects.lnk
   - **Office Worker:** report_templates.lnk, client_files.lnk, shared_drive.lnk
   - **Student:** assignments.lnk, lecture_notes.lnk

5. **Configuration:**
   ```json
   {
     "lnk_files": {
       "enabled": true,
       "recent_docs": [
         {"target": "C:\\Users\\User\\Documents\\report.docx", "desc": "Quarterly Report"},
         {"target": "C:\\Users\\User\\Desktop\\projects", "desc": "My Projects"}
       ],
       "desktop_shortcuts": [
         {"target": "C:\\Program Files\\Visual Studio Code", "name": "VS Code"}
       ]
     }
   }
   ```

**Function Signatures (C++):**
```cpp
class LNKFileGenerator {
  public:
    LNKFileGenerator(const json& config);
    
    // Generate LNK file at path
    void createLNKFile(const std::wstring& targetPath,
                       const std::wstring& lnkPath,
                       const std::wstring& description);
    
    // Generate recent documents
    void populateRecentDocuments();
    
    // Generate desktop shortcuts
    void populateDesktopShortcuts();
    
    // Clear all LNK files (testing)
    void clearAllLNKFiles();
    
  private:
    struct LNKHeader { /* binary structure */ };
    void writeLNKFile(const std::wstring& path, const LNKHeader& header);
};
```

**Testing Criteria:**
- Prefetch files readable in `C:\Windows\Prefetch\` (requires admin)
- LNK files accessible in Recent Documents
- File properties accessible via right-click → Properties
- No corrupted/unreadable binary files
- Timestamps appear logical in file explorer

---

### Feature #7: Configurable User Profile Generator with Dynamic Data Population

**Objective:** Create flexible, reusable user profiles with automatic data generation.

**Requirements:**

1. **Master Profile Schema (JSON)**
   ```json
   {
     "profile_id": "developer_001",
     "profile_name": "John Developer",
     "profile_type": "software_developer",
     "user_attributes": {
       "full_name": "John Doe",
       "occupation": "Software Developer",
       "interests": ["GitHub", "Stack Overflow", "Tech Blogs", "Open Source"],
       "activity_intensity": 0.85,
       "typing_speed": "fast",
       "mouse_precision": "high",
       "technical_level": "expert"
     },
     "file_patterns": {
       "document_count": [50, 100],
       "code_projects": 5,
       "technical_docs": true
     },
     "registry_patterns": { ... },
     "browser_patterns": { ... },
     "locale": "de_DE"
   }
   ```

2. **Dynamic Data Generation**
   - Generate realistic names (mix first/last name databases)
   - Random company names (tech companies, startups, etc.)
   - Email addresses derived from name + company
   - Phone numbers (realistic format)
   - Address generation (fake but plausible)
   - Profile pictures (via placeholder APIs or local templates)

3. **Locale-Specific Customization**
   - **German (de_DE):** File names in German, German websites in browser history, etc.
   - **English (en_US):** English file names, US-based sites
   - **French (fr_FR):** French accents in names, French websites
   - Character encoding: proper Unicode handling

4. **Profile Validation**
   - Consistency checks (interests match occupation, etc.)
   - Ensure all required fields present
   - Warn about unrealistic combinations
   - Auto-fix minor issues

5. **Multi-Profile Management**
   - Create multiple profiles for same VM (switch between them)
   - Profile versioning/history (rollback capability)
   - Import/Export profiles as .json files
   - Merge profiles (combine attributes from multiple profiles)

6. **Profile Application**
   - Apply profile → all other Tier 1 & Tier 2 features use the profile data
   - Atomic application (all-or-nothing, no partial applies)
   - Rollback capability if application fails

7. **Configuration:**
   ```json
   {
     "profile_engine": {
       "enabled": true,
       "current_profile": "developer_001",
       "auto_generate_data": true,
       "data_generation_rules": {
         "generate_names": true,
         "generate_emails": true,
         "generate_documents": true,
         "include_photos": true
       },
       "locale": "de_DE",
       "timezone": "Europe/Berlin"
     }
   }
   ```

**Function Signatures (C++):**
```cpp
class ProfileEngine {
  public:
    ProfileEngine(const json& defaultConfig);
    
    // Load profile from file
    bool loadProfile(const std::string& profilePath);
    
    // Create new profile with auto-generation
    std::string createNewProfile(const std::string& profileType, 
                                  const std::string& locale);
    
    // Apply profile to all systems (integrate with other engines)
    bool applyProfile();
    
    // Get profile details
    json getProfileData();
    
    // Save current profile
    void saveProfile(const std::string& path);
    
    // List all available profiles
    std::vector<std::string> listProfiles();
    
    // Generate fake data
    struct GeneratedData {
      std::string fullName;
      std::string email;
      std::string phone;
      std::string address;
      std::string company;
    };
    GeneratedData generateFakeData();
    
  private:
    std::string generateName(const std::string& locale);
    std::string generateEmail(const std::string& name, const std::string& company);
    void validateProfile();
};
```

**Testing Criteria:**
- Profiles load without errors
- Generated data is realistic and locale-appropriate
- Auto-generated names/emails are pronounceable
- Profile application doesn't break existing features
- Rollback restores previous state

---

### Feature #8: Network Traffic & Connection Pattern Simulator

**Objective:** Simulate realistic network activity at OS level.

**Requirements:**

1. **DNS Request Simulation**
   - Simulate DNS lookups for "visited" websites
   - Pattern: sporadic lookups throughout day
   - Include:
     - Search engine lookups (google.com, bing.com)
     - Tech sites (stack overflow, github, etc.)
     - News sites (depending on profile)
   - Log in Event Viewer if possible

2. **Network Connection Events**
   - Inject logs in Windows Event Viewer (Network subcategory)
   - Event types: "Connection established", "Connection closed", "Connection timeout"
   - Realistic timing (connections last 1-60 minutes)
   - Mix of:
     - HTTP(S) connections to websites
     - Database connections (if dev profile)
     - Network share access (if office profile)

3. **Firewall Activity Logging**
   - Windows Firewall blocked/allowed connections
   - Occasional blocked attempts (realistic security events)
   - Profile-appropriate blocked ports (RDP on dev machine, etc.)
   - Log to: `%ProgramData%\Microsoft\Windows\Firewall\Logs\`

4. **ARP Cache Manipulation**
   - Inject ARP entries for "seen" network devices
   - Gateway, DNS servers, network printers, etc.
   - Use `arp.exe -s` to add entries (requires admin)
   - Realistic MAC addresses

5. **Network Share Access**
   - Simulate accessing network shares (UNC paths)
   - Create entry in Network Locations history
   - Add to MRU for file access
   - Different shares per profile (Office: \\company\share, Dev: \\git-server)

6. **Optional: VPN Simulation**
   - Show signs of VPN usage (registry entries)
   - Log connection to VPN servers
   - Modify network adapter list to show VPN adapter

7. **Configuration:**
   ```json
   {
     "network_simulator": {
       "enabled": true,
       "simulate_dns": true,
       "dns_targets": ["google.com", "stackoverflow.com", "github.com"],
       "simulate_firewall": true,
       "firewall_block_rate": 0.05,
       "simulate_network_shares": true,
       "network_shares": ["\\\\company\\share", "\\\\projects"],
       "simulate_vpn": false
     }
   }
   ```

**Function Signatures (C++):**
```cpp
class NetworkSimulator {
  public:
    NetworkSimulator(const json& config);
    
    // Simulate DNS lookup
    void simulateDNSLookup(const std::string& domain);
    
    // Simulate network connection
    void simulateNetworkConnection(const std::string& destination, 
                                    int durationSeconds);
    
    // Inject firewall events
    void injectFirewallEvent(bool allowed, const std::string& app, 
                            int port);
    
    // Add ARP entry (requires admin)
    void addARPEntry(const std::string& ip, const std::string& mac);
    
    // Access network share
    void accessNetworkShare(const std::wstring& uncPath);
    
    // Get current network stats
    struct NetworkStats {
      int activeConnections;
      int DNSLookupsToday;
      std::vector<std::string> seenNetworks;
    };
    NetworkStats getStats();
    
  private:
    void logNetworkEvent(const std::string& eventType, 
                         const std::string& details);
};
```

**Testing Criteria:**
- Network connections logged (check Event Viewer → Windows Logs → System)
- Firewall entries visible in Windows Firewall with Advanced Security
- ARP entries visible (requires admin, check via `arp -a`)
- Network shares accessible from File Explorer
- No system errors or blocked operations

---

## 📦 Implementation Requirements

### Dependencies on Tier 1
- All Tier 2 features integrate with Tier 1 modules
- Use Tier 1 class instances (MouseBehavior, KeystrokeEmulator, etc.)
- Activity Engine uses these to simulate user actions
- Profile Engine loads config for all systems

### Code Architecture
- **Activity Engine** should be orchestrator (calls Tier 1 classes)
- **Profile Engine** is configuration provider (feeds data to all others)
- **Prefetch/LNK** generators are independent (no interaction with Activity/Network)
- **Network Simulator** can run independently or alongside Activity Engine

### Threading Model
- Activity Engine runs in separate thread (continuous loop)
- Use `std::thread` for activity loop
- Proper synchronization (mutex for state changes)
- Graceful shutdown without crashes

### Error Handling
- Activities fail gracefully (skip to next if app launch fails)
- Missing profiles handled with defaults
- Network operations non-blocking (fire and forget)
- Log all errors for debugging

### Performance Targets
- Activity Engine should not exceed 15% CPU
- Memory footprint < 200MB even with continuous operation
- Profile loading < 1 second
- LNK/Prefetch generation < 5 seconds

---

## 🧪 Testing Strategy

### Integration Testing
1. Start with Feature #7 (Profile Engine) - ensures good base data
2. Test Feature #5 (Activity Engine) with profiles
3. Test Feature #6 (Artifacts) - verify files created correctly
4. Test Feature #8 (Network) - check event logs

### Long-Running Test (8+ hours)
- Start Activity Engine
- Monitor CPU/Memory every 30 minutes
- Verify no crashes or hangs
- Check activity logs for realistic patterns

### Manual Validation
```cpp
// Test in main()
ProfileEngine profiles(config);
profiles.createNewProfile("developer", "de_DE");
profiles.applyProfile();

ActivityEngine activityEngine(config);
activityEngine.start();  // Run for 1 hour, observe

PrefetchGenerator prefetch(config);
prefetch.generateAllPrefetches();
// Check C:\Windows\Prefetch\ directory

LNKFileGenerator lnks(config);
lnks.populateRecentDocuments();
// Check %AppData%\Microsoft\Windows\Recent\

NetworkSimulator network(config);
network.simulateDNSLookup("github.com");
// Check Event Viewer → Windows Logs → System
```

---

## 📝 Deliverables

After implementing Tier 2, provide:

1. **Complete source code** for all 4 features (8 modules total)
2. **Enhanced CMakeLists.txt** with new dependencies
3. **Sample profiles** (developer.json, office_worker.json, student.json, gamer.json)
4. **Activity templates** (activity_templates.json) with example sequences
5. **Integration tests** showing Tier 1 + Tier 2 working together
6. **8-hour stability test results**
7. **Technical documentation** of key algorithms (state machine, file format parsers)

---

## ⚠️ Critical Constraints

- **NO external runtime** dependencies (C++ only)
- **Single-threaded compatibility** (Activity Engine must support --single-threaded flag)
- **Admin privileges:** Some features (Firewall, ARP) may require admin
- **Reversible operations:** Profile application must be rollback-able
- **Backward compatibility:** Must integrate smoothly with Tier 1

---

## 🚀 Success Criteria

- ✅ All 4 Tier 2 features fully implemented
- ✅ Code compiles without warnings with Tier 1
- ✅ Activity Engine runs 8+ hours without crashes
- ✅ Generated artifacts pass forensic tool inspection
- ✅ Profile system supports multiple users/scenarios
- ✅ Network events visible in Windows Event Logs
- ✅ Clean architecture for future Tier 4 additions

---

**Ready to integrate with Tier 1? Start with Feature #7 (Profiles) or Feature #5 (Activity Engine) — they have strong synergy.**

