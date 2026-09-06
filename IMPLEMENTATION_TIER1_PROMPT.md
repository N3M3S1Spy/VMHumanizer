# 🔧 TIER 1 Implementation Prompt (For Claude Opus)

**Projekt:** VMHumanizer - Core Behavior Humanization Engine  
**Tier:** Tier 1 (Essential Core Features)  
**Tech-Stack:** Pure C++ (C++17/20) + Windows API (WINAPI)  
**Target Platform:** Windows 10/11  

---

## 📋 Features to Implement (Tier 1)

### Feature #1: Behavioral Mouse Movement Simulator
### Feature #2: Realistic Keystroke Timing & Behavior Emulation
### Feature #3: Advanced Registry MRU Manipulation
### Feature #4: Fake Event Log Injection (Windows Event Viewer)

---

## 🏗️ Project Structure (Expected)

After implementation, the project should have this structure:

```
VMHumanizer/
├── src/
│   ├── main.cpp                          # Entry point & orchestration
│   ├── mouse_behavior.cpp                # Feature #1
│   ├── mouse_behavior.h
│   ├── keystroke_emulator.cpp            # Feature #2
│   ├── keystroke_emulator.h
│   ├── registry_mru_engine.cpp           # Feature #3
│   ├── registry_mru_engine.h
│   ├── event_log_injector.cpp            # Feature #4
│   ├── event_log_injector.h
│   ├── common/
│   │   ├── profile_loader.cpp            # JSON profile loading
│   │   ├── profile_loader.h
│   │   ├── utils.cpp                     # Logging, file ops
│   │   ├── utils.h
│   │   └── types.h                       # Shared data structures
│   └── windows/
│       ├── winapi_wrapper.cpp            # WinAPI convenience wrappers
│       └── winapi_wrapper.h
├── data/
│   ├── profiles/
│   │   ├── developer.json
│   │   ├── office_worker.json
│   │   └── student.json
│   └── templates/
│       ├── eventlog_templates.json
│       └── registry_templates.json
├── FEATURE_ROADMAP.md                    # Already created
├── README.md
├── CMakeLists.txt                        # Build configuration (NEW)
└── .gitignore
```

---

## 🎯 Detailed Feature Specifications

### Feature #1: Behavioral Mouse Movement Simulator

**Objective:** Simulate realistic, non-linear mouse movements with human-like characteristics.

**Requirements:**

1. **Bezier Curve Interpolation**
   - Implement quadratic or cubic Bezier curves between mouse target points
   - Avoid linear interpolation (too mechanical, easily detected)
   - Control points should be randomized for variation
   - Smoothness parameter configurable (0.0 = more jagged, 1.0 = ultra-smooth)

2. **Micro-Jitter Simulation**
   - Add fine random oscillations (±2-5px) simulating hand tremor
   - Jitter amplitude should vary based on movement distance
   - Short movements = smaller jitter, long movements = can have more jitter

3. **Variable Speed Profiles**
   - Speed should NOT be constant during movement
   - Implement: "slow_start" (acceleration), "fast_then_slow" (deceleration), "uniform"
   - Speed variation realistic to human hand movement

4. **Intelligent Pauses**
   - When moving towards UI elements (buttons, text input), add micro-pauses (100-500ms)
   - Pause duration inversely correlated with movement distance
   - Can be configured as "aggressive" vs "relaxed" pause behavior

5. **Configuration via JSON**
   ```json
   {
     "mouse": {
       "curve_type": "quadratic_bezier",
       "smoothness": 0.85,
       "jitter_amplitude": [2, 5],
       "pause_on_target": true,
       "pause_duration_range": [100, 500],
       "speed_profile": "fast_then_slow"
     }
   }
   ```

**Function Signatures (C++):**
```cpp
class MouseBehavior {
  public:
    // Initialize with JSON config
    MouseBehavior(const json& config);
    
    // Move mouse from current position to target with simulated behavior
    void moveTo(int targetX, int targetY, int durationMs = 500);
    
    // Get current mouse position
    std::pair<int, int> getCurrentPosition();
    
  private:
    // Bezier interpolation
    std::pair<double, double> bezierPoint(double t);
    
    // Add jitter noise
    std::pair<double, double> applyJitter(double x, double y);
    
    // Calculate speed profile (returns multiplier 0.0-2.0)
    double getSpeedMultiplier(double progress);
};
```

**Testing Criteria:**
- Recorded mouse movements should have non-linear patterns
- Jitter should be visible in high-speed recordings but subtle
- Forensic analysis tools should not flag movements as "bot-like"

---

### Feature #2: Realistic Keystroke Timing & Behavior Emulation

**Objective:** Simulate authentic typing patterns with human-like timing and occasional errors.

**Requirements:**

1. **Inter-Key Delay (IKD) Simulation**
   - Implement realistic delays between Key-Down and Key-Up events
   - Delays should be normally distributed (mean ~100-150ms, std ~30-50ms)
   - Duration varies per user profile ("fast typist": lower mean, "slow typist": higher mean)

2. **Digraph-Based Timing**
   - Common letter pairs should have consistent timing patterns
   - Examples: "th" is faster (~80ms), "qz" is slower (~200ms)
   - Use a digraph lookup table or Markov chain
   - Configuration via JSON with digraph timing pairs

3. **Typing Error Simulation**
   - Randomly introduce typos with configurable error rate (default 5-10%)
   - Simulate automatic correction:
     - Type wrong letter
     - Pause (200-400ms, as if noticing error)
     - Backspace and retype correct letter
   - Error rate should be lower for common words, higher for rare combinations

4. **Pause Injection Between Words/Phrases**
   - After finishing a word, pause before starting next (50-300ms)
   - Longer pauses after punctuation or sentence end (200-800ms)
   - Occasional longer "thinking pauses" between paragraphs (1-3 seconds)

5. **Configuration Example:**
   ```json
   {
     "keystroke": {
       "key_hold_duration": [40, 200],
       "inter_key_delay_mean": 120,
       "inter_key_delay_std": 40,
       "error_rate": 0.08,
       "word_pause_range": [50, 300],
       "sentence_pause_range": [200, 800],
       "digraph_timings": {
         "th": 80,
         "er": 90,
         "qz": 200
       }
     }
   }
   ```

**Function Signatures (C++):**
```cpp
class KeystrokeEmulator {
  public:
    KeystrokeEmulator(const json& config);
    
    // Type a string with all realistic patterns
    void typeString(const std::string& text, bool includeErrors = true);
    
    // Simulate single key press with timing
    void pressKey(char key, int durationMs = -1);
    
    // Get statistics about typing patterns (for debugging)
    struct TypingStats { double avgIKD; double errorRate; };
    TypingStats getStatistics();
    
  private:
    int getIKDDuration(char prevKey, char currentKey);
    bool shouldInjectError();
    void injectTypo(const std::string& text, size_t& pos);
};
```

**Testing Criteria:**
- Typing should not be perfectly consistent (visually noticeable variation)
- Error rate should match configuration
- Digraph timing should be observable in keystroke logs
- No multiple errors in same word (realistic)

---

### Feature #3: Advanced Registry MRU Manipulation

**Objective:** Populate Windows Registry with realistic Most-Recently-Used (MRU) entries.

**Requirements:**

1. **TypedURLs & TypedURLMRU**
   - Path: `HKCU\Software\Microsoft\Internet Explorer\TypedURLs`
   - Create realistic URL entries with timestamps
   - Mix of: tech sites, office sites, search results, social media
   - Max 25 entries (realistic limit)

2. **UserAssist Entries (Execution History)**
   - Path: `HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\UserAssist\{GUID}\Count`
   - ROT-13 encoded values (must implement decoder)
   - Entries for:
     - Executable launches (notepad.exe, calc.exe, etc.)
     - Document opens (Word, Excel, PDFs)
     - Website visits (via shell items)
   - Timestamps should reflect realistic usage patterns

3. **Office MRU (Word, Excel, PowerPoint)**
   - Paths: `HKCU\Software\Microsoft\Office\16.0\{Word|Excel|PowerPoint}\FileMRU`
   - Recent documents with file paths & open times
   - Mix of local and network paths
   - Realistic file names (not "test.docx")

4. **Shell Items & Recent Docs**
   - Path: `HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\RunMRU`
   - Store recent commands typed in Run dialog

5. **Registry Value Structure**
   - Correct data types (REG_SZ, REG_DWORD, REG_BINARY)
   - Proper timestamp encoding (Windows FILETIME format)
   - Realistic registry timestamps

6. **Automatic Cleanup**
   - Periodically remove oldest entries (simulate Windows cleanup)
   - Never exceed max entries per key
   - Maintain realistic "last access" times

7. **Configuration:**
   ```json
   {
     "registry_mru": {
       "enable_typed_urls": true,
       "enable_user_assist": true,
       "enable_office_mru": true,
       "max_urls": 20,
       "max_office_entries": 15,
       "include_programs": ["notepad.exe", "calc.exe", "explorer.exe"],
       "include_documents": ["document1.docx", "spreadsheet.xlsx"]
     }
   }
   ```

**Function Signatures (C++):**
```cpp
class RegistryMRUEngine {
  public:
    RegistryMRUEngine(const json& config);
    
    // Populate all MRU entries based on profile
    void populateAllMRU();
    
    // Add specific entries
    void addTypedURL(const std::wstring& url);
    void addUserAssistEntry(const std::wstring& path);
    void addOfficeMRU(const std::wstring& filePath, const std::string& office_app);
    
    // Cleanup old entries
    void cleanupOldEntries();
    
    // Clear all MRU (for testing)
    void clearAllMRU();
    
  private:
    void rotEncode(const std::wstring& input, std::wstring& output);
    FILETIME getCurrentFileTime();
    void cleanupRegistry(const std::wstring& hkey_path, int maxEntries);
};
```

**Testing Criteria:**
- Registry entries should be readable in Regedit.exe
- Timestamps should be in correct Windows FILETIME format
- UserAssist values should be properly ROT-13 encoded
- No duplicate entries across different MRU locations

---

### Feature #4: Fake Event Log Injection (Windows Event Viewer)

**Objective:** Inject realistic Windows Event Log entries to simulate system activities.

**Requirements:**

1. **Event Categories to Support**
   - **Security Logs:** Process creation (4688), Account logon (4624), User account created (4720)
   - **System Logs:** Boot (6005), Shutdown (6006), Service start/stop, Device insertion
   - **Application Logs:** Generic errors, warnings (from Office, Visual Studio, etc.)
   - **PowerShell Logs:** Script execution records, pipeline execution

2. **Event Structure**
   - Correct Event ID, Source, Type (Information/Warning/Error)
   - Realistic Message strings (not generic)
   - Computer name matches system
   - User/SID fields correctly filled
   - Timestamp in correct format (realistic order, not all same time)

3. **Temporal Realism**
   - Events should be spread realistically across time
   - Boot events should be far apart (not every minute)
   - Clustered events make sense (e.g., multiple "process creation" events during batch operation)
   - No events in future or too old (within last 30 days)

4. **Event Templates**
   - Use JSON template system for event generation:
   ```json
   {
     "events": [
       {
         "event_id": 4688,
         "source": "Security",
         "type": "Information",
         "message_template": "New process created: {process_name} by {user}",
         "frequency": "occasional",
         "process_examples": ["notepad.exe", "calc.exe", "cmd.exe"]
       }
     ]
   }
   ```

5. **Automatic Event Generation**
   - Generate plausible events based on profile
   - "Office Worker" gets Outlook, Excel, Word process events
   - "Developer" gets cmd.exe, Visual Studio, git.exe events
   - Time events realistically (no 100 events per second)

6. **Cleanup & Limits**
   - Don't exceed 10,000 events per log (realistic limit)
   - Remove oldest events when limit reached
   - Preserve boot-related events (important for timeline)

**Function Signatures (C++):**
```cpp
class EventLogInjector {
  public:
    EventLogInjector(const json& config);
    
    // Inject single event
    void injectEvent(const std::string& logName, 
                     DWORD eventID, 
                     const std::wstring& message);
    
    // Generate and inject realistic events for profile
    void populateEventsForProfile(const std::string& profile_name, int count = 50);
    
    // Inject boot/shutdown sequence
    void injectBootSequence(SYSTEMTIME bootTime);
    void injectShutdownSequence(SYSTEMTIME shutdownTime);
    
    // Clear all injected events
    void clearAllEvents();
    
  private:
    void openEventLog(const std::wstring& logName);
    DWORD buildEventRecord(const std::wstring& message, WORD type);
};
```

**Testing Criteria:**
- Events visible in Windows Event Viewer
- Event properties (ID, Source, Type, Time) are correct
- Message text is readable and contextually appropriate
- No duplicate event IDs in same second
- Timestamps form logical sequence (no time travel)

---

## 📦 Implementation Requirements

### Code Quality Standards
- **Language:** C++17/20
- **Compiler:** MSVC (Visual Studio 2022+) or MinGW-w64
- **No External Dependencies:** Only Windows API + nlohmann/json (header-only)
- **Code Style:**
  - Class names: PascalCase
  - Method names: camelCase
  - Member variables: m_prefixed for private
  - Constants: UPPER_SNAKE_CASE
  - Use `std::wstring` for paths/registry (Unicode safety)

### Header & Implementation Split
- Each feature gets `.h` and `.cpp` file
- Headers include guard + comprehensive comments
- Keep implementations in .cpp (avoid template bloat in headers)

### Error Handling
- Use `HRESULT` for Windows API calls
- Custom exception class for non-API errors
- Graceful fallback when operations fail (log & continue)
- No crashes on invalid user input

### Logging (Use utils.h)
```cpp
logInfo("Feature initialized: %s", featureName);
logWarning("Potential issue: %s", description);
logError("Failed to inject event: 0x%08X", hresult);
```

### JSON Configuration
- All features must be configurable via JSON profiles
- Load from `data/profiles/{profile_name}.json`
- Sensible defaults if values missing
- Validate config on load (fail fast with clear messages)

---

## 🧪 Testing Strategy

### Unit Testing (Manual)
Each feature should have a simple test in main():

```cpp
int main() {
    // Initialize
    json config = loadConfigFromFile("data/profiles/developer.json");
    
    // Test Feature #1: Mouse
    MouseBehavior mouseBehavior(config);
    mouseBehavior.moveTo(500, 300);
    
    // Test Feature #2: Keystroke
    KeystrokeEmulator keystrokeEmu(config);
    keystrokeEmu.typeString("Hello, World!");
    
    // Test Feature #3: Registry
    RegistryMRUEngine regEngine(config);
    regEngine.populateAllMRU();
    
    // Test Feature #4: Event Log
    EventLogInjector eventLog(config);
    eventLog.populateEventsForProfile("developer", 20);
    
    return 0;
}
```

### Validation Checklist
- [ ] Mouse movements visible and non-linear (use screen recorder)
- [ ] Keystroke timing varies (measure inter-key delays)
- [ ] Registry entries readable in Regedit with correct values
- [ ] Event Log entries visible in Event Viewer with correct properties
- [ ] No system crashes or hangs
- [ ] Memory usage < 100MB
- [ ] CPU usage < 10% at idle

---

## 📝 Deliverables

After implementing Tier 1, provide:

1. **Complete source code** with all 4 features
2. **CMakeLists.txt** for easy compilation
3. **Sample profiles** (developer.json, office_worker.json, student.json)
4. **README with usage examples** (how to use each feature)
5. **Brief technical documentation** of key algorithms
6. **Quick testing checklist** output

---

## ⚠️ Critical Constraints

- **NO external libraries** except Windows API + nlohmann/json (header-only)
- **NO Python, .NET, Node.js, or other runtimes**
- **Single executable:** All features compiled into one .exe
- **Unicode support:** Proper handling of German characters & international paths
- **Backward compatible:** With existing main.cpp (integrate, don't replace)
- **Performance:** Features should complete initialization in < 2 seconds

---

## 🚀 Success Criteria

- ✅ All 4 Tier 1 features fully implemented
- ✅ Code compiles without warnings
- ✅ Can be executed on Windows 10/11 without admin privileges (where possible)
- ✅ Generates realistic artifacts (testable in Event Viewer, Regedit, etc.)
- ✅ Configurable via JSON profiles
- ✅ Clean, maintainable code structure for future Tier 2/4 additions

---

**Ready to implement? Start with Feature #1 (Mouse) or Feature #2 (Keystroke) — whichever feels most natural.**

