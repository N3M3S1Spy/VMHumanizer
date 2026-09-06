# 📊 TIER 4 Implementation Prompt (For Claude Opus)

**Projekt:** VMHumanizer - Observability, Telemetry & Debugging Infrastructure  
**Tier:** Tier 4 (Observability & Debugging)  
**Tech-Stack:** Pure C++ (C++17/20) + Windows API (WINAPI)  
**Prerequisites:** TIER 1 & TIER 2 should be completed first  
**Target Platform:** Windows 10/11  

---

## 📋 Features to Implement (Tier 4)

### Feature #11: Structured Logging & Activity Telemetry Pipeline
### Feature #12: Visual Activity Debugger (Inline Reporting)
### Feature #13: Behavioral Anomaly Detection & Auto-Correction

---

## 🏗️ Project Structure Extension

Tier 4 builds on Tier 1 & 2. New additions:

```
VMHumanizer/
├── src/
│   ├── [All Tier 1 & Tier 2 modules already exist]
│   ├── logger.cpp                        # Feature #11
│   ├── logger.h
│   ├── telemetry_pipeline.cpp            # Feature #11
│   ├── telemetry_pipeline.h
│   ├── debug_ui.cpp                      # Feature #12
│   ├── debug_ui.h
│   ├── anomaly_detector.cpp              # Feature #13
│   ├── anomaly_detector.h
│   ├── anomaly_corrector.cpp             # Feature #13 (part 2)
│   ├── anomaly_corrector.h
│   └── common/
│       ├── metrics.cpp                   # NEW - metrics collection
│       ├── metrics.h
│       ├── json_utils.cpp                # NEW - JSON helpers
│       └── json_utils.h
├── data/
│   ├── logs/
│   │   ├── activities.log                # Generated activity log (JSON Lines)
│   │   ├── metrics.log                   # System metrics log
│   │   └── anomalies.log                 # Detected anomalies log
│   ├── reports/
│   │   └── activity_report.json          # Generated daily reports
│   └── templates/
│       └── anomaly_rules.json             # NEW - anomaly detection rules
├── CMakeLists.txt                        # Updated
└── [other files]
```

---

## 🎯 Detailed Feature Specifications

### Feature #11: Structured Logging & Activity Telemetry Pipeline

**Objective:** Comprehensive logging system for all VMHumanizer activities with structured JSON output.

**Requirements:**

1. **Structured JSON Logging**
   - Every activity logged as JSON object (one per line - JSONL format)
   - Common fields in all logs:
     - `timestamp`: ISO 8601 format (2026-09-02T14:23:45.123Z)
     - `log_level`: DEBUG, INFO, WARN, ERROR
     - `component`: Which module logged this (mouse_behavior, keystroke, etc.)
     - `message`: Human-readable description
   - Activity-specific fields vary by type

2. **Log Entry Examples**
   ```json
   {"timestamp":"2026-09-02T14:23:45.123Z","level":"INFO","component":"activity_engine","event_type":"activity_started","activity_name":"Morning Routine","duration_ms":3600000}
   {"timestamp":"2026-09-02T14:23:50.456Z","level":"DEBUG","component":"mouse_behavior","event_type":"mouse_move","from":[100,200],"to":[500,300],"duration_ms":250,"jitter_amplitude":3.2}
   {"timestamp":"2026-09-02T14:24:15.789Z","level":"INFO","component":"keystroke_emulator","event_type":"text_input","text_length":42,"ikd_mean":125,"error_injections":2}
   {"timestamp":"2026-09-02T14:25:00.000Z","level":"WARN","component":"registry_mru","event_type":"registry_modification","key":"HKCU\\...","value_count":15}
   {"timestamp":"2026-09-02T14:26:30.111Z","level":"ERROR","component":"event_log_injector","event_type":"injection_failed","error_code":"0x00000005","description":"Access denied"}
   ```

3. **Log Levels**
   - **DEBUG:** Detailed internal operations (mouse jitter values, keystroke delays)
   - **INFO:** High-level events (activity started, feature initialized)
   - **WARN:** Potential issues (feature taking longer than expected, default values used)
   - **ERROR:** Failed operations (registry write failed, event log injection failed)

4. **Log File Management**
   - Main log: `data/logs/activities.log`
   - Metrics log: `data/logs/metrics.log` (performance data)
   - Anomalies log: `data/logs/anomalies.log` (anomaly detections)
   - **Log Rotation:**
     - New log file daily or when size exceeds 100MB
     - Old logs archived to `data/logs/archive/activities_2026-09-02.log.gz`
     - Keep last 30 days of logs
   - **Async Writing:** Non-blocking log writes (queue-based)

5. **Performance Metrics Logging**
   - Every 60 seconds, log system metrics:
   ```json
   {"timestamp":"2026-09-02T14:30:00.000Z","level":"INFO","component":"metrics","event_type":"system_metrics","cpu_percent":5.2,"memory_mb":85,"disk_mb_used":150,"active_threads":4}
   ```
   - Track: CPU %, Memory, Disk I/O, Thread count, Activity queue length
   - Use for performance bottleneck identification

6. **Activity Chain Tracking**
   - Each activity gets unique `activity_chain_id` (UUID)
   - All sub-operations logged with same chain ID
   - Enables tracking complete workflow through logs
   ```json
   {"...", "activity_chain_id": "chain_abc123", "step": 1, ...}
   {"...", "activity_chain_id": "chain_abc123", "step": 2, ...}
   {"...", "activity_chain_id": "chain_abc123", "step": 3, ...}
   ```

7. **Error & Exception Tracking**
   - Full stack trace for unhandled exceptions
   - Error context (what was the system doing when error occurred)
   - Correlation with other logs for root cause analysis
   ```json
   {"timestamp":"...", "level":"ERROR", "exception_type":"RegistryException", "message":"Failed to write to registry", "stack_trace":"...", "context": {"key": "HKCU\\...", "operation": "SetValue"}}
   ```

8. **Audit Trail for System Changes**
   - Every registry modification logged
   - File creation/modification logged
   - Process launch logged
   - Format includes: user, timestamp, operation, target, result

9. **Configuration:**
   ```json
   {
     "logging": {
       "enabled": true,
       "log_level": "INFO",
       "output_format": "jsonl",
       "files": {
         "activities": "data/logs/activities.log",
         "metrics": "data/logs/metrics.log",
         "anomalies": "data/logs/anomalies.log"
       },
       "rotation": {
         "max_file_size_mb": 100,
         "retention_days": 30
       },
       "async_writes": true,
       "queue_size": 1000
     }
   }
   ```

**Function Signatures (C++):**
```cpp
class Logger {
  public:
    // Singleton pattern
    static Logger& getInstance();
    
    // Initialize logging system
    void initialize(const json& config);
    
    // Log at different levels (variadic, printf-style)
    void logDebug(const std::string& component, const std::string& message);
    void logInfo(const std::string& component, const std::string& message);
    void logWarn(const std::string& component, const std::string& message);
    void logError(const std::string& component, const std::string& message);
    
    // Log structured JSON event
    void logEvent(const json& eventData);
    
    // Log exception with stack trace
    void logException(const std::exception& e, const std::string& context);
    
    // Flush pending logs
    void flush();
    
    // Get statistics
    struct LogStats { 
      int totalEvents; 
      int errors; 
      int warnings;
      int queuedEvents;
    };
    LogStats getStats();
    
  private:
    void writeToFile(const std::string& filename, const std::string& data);
    void rotateLogIfNeeded(const std::string& filename);
};

class TelemetryPipeline {
  public:
    TelemetryPipeline();
    
    // Start collecting metrics
    void startMetricsCollection();
    void stopMetricsCollection();
    
    // Log performance metrics every N seconds
    void recordSystemMetrics();
    
    // Activity lifecycle tracking
    void recordActivityStart(const std::string& activityName, const std::string& activityChainId);
    void recordActivityEnd(const std::string& activityName, const std::string& activityChainId, bool success);
    
    // Get telemetry summary
    struct TelemetryStats {
      int activitiesCompleted;
      int activitiesFailed;
      double avgActivityDurationMs;
      double cpuUsagePercent;
      int memoryUsageMb;
    };
    TelemetryStats getStats();
    
  private:
    void metricsThread();
};
```

**Testing Criteria:**
- Log files created and populated
- JSON is valid (can be parsed)
- Performance overhead < 2% CPU
- Logs readable in text editor
- Rotation works correctly (file rolling)
- Can parse logs for analysis

---

### Feature #12: Visual Activity Debugger (Inline Reporting)

**Objective:** Real-time console-based dashboard for monitoring VMHumanizer activities.

**Requirements:**

1. **Console-Based Dashboard**
   - No external UI framework (pure Windows Console API)
   - Colored output (use Windows Console color API)
   - Layout:
     ```
     ╔═══════════════════════════════════════════════════════════════════╗
     ║                   VMHumanizer Activity Monitor                    ║
     ╠═══════════════════════════════════════════════════════════════════╣
     ║ STATUS: Running [████████░░] 80%                                  ║
     ║ Current Activity: Coding Session                                  ║
     ║ Activity Duration: 45min 23s                                      ║
     ║ Profile: developer_001                                            ║
     ╠═══════════════════════════════════════════════════════════════════╣
     ║ RECENT EVENTS:                                                    ║
     ║ [14:30:45] Mouse moved to (500, 300)                             ║
     ║ [14:30:48] Typed 42 characters                                   ║
     ║ [14:31:00] Registered MRU: project.cpp                           ║
     ║ [14:31:15] Event log injection: Success                          ║
     ╠═══════════════════════════════════════════════════════════════════╣
     ║ PERFORMANCE:                                                      ║
     ║ CPU: 5.2% | Memory: 85MB | Threads: 4                           ║
     ║ Activity Queue: 3 pending                                         ║
     ╠═══════════════════════════════════════════════════════════════════╣
     ║ ACTIVE TASKS:                                                     ║
     ║ [████████░░] Code Review - 45 of 60 seconds                      ║
     ║ [██████████] GitHub Browse - Complete                           ║
     ╠═══════════════════════════════════════════════════════════════════╣
     ║ STATS:                                                            ║
     ║ Total Activities: 12 | Errors: 0 | Uptime: 2h 15m               ║
     ║ Avg Activity Duration: 8m 32s                                    ║
     ╠═══════════════════════════════════════════════════════════════════╣
     ║ [P]ause  [R]esume  [S]kip Activity  [D]etailed Logs  [Q]uit     ║
     ╚═══════════════════════════════════════════════════════════════════╝
     ```

2. **Display Sections**
   - **Status Bar:** Current overall status + progress bar
   - **Current Activity:** What is happening right now
   - **Recent Events Feed:** Last 5-10 events (scrollable)
   - **Performance Metrics:** Real-time CPU/Memory/Thread count
   - **Active Tasks:** Progress bars for concurrent tasks
   - **Statistics:** Cumulative stats (total activities, errors, uptime)
   - **Control Bar:** Keyboard shortcuts for interaction

3. **Color Coding**
   - Green: Successful operations
   - Yellow: Warnings
   - Red: Errors
   - Cyan: Info/Status updates
   - White: Normal text

4. **Keyboard Controls**
   - **P:** Pause activity engine
   - **R:** Resume activity engine
   - **S:** Skip current activity
   - **D:** Show detailed logs (open log file or display more details)
   - **Q:** Quit program gracefully
   - **↑/↓:** Scroll event feed
   - **+/-:** Adjust refresh rate

5. **Real-Time Updates**
   - Refresh every 500ms (configurable)
   - Non-blocking input (doesn't freeze dashboard)
   - Smooth animations (progress bar filling)
   - No console flicker (clear & redraw optimized)

6. **Data Sources**
   - Reads from Logger and TelemetryPipeline
   - Aggregates metrics in real-time
   - Queries current state of all engines

7. **Configuration:**
   ```json
   {
     "debug_ui": {
       "enabled": true,
       "refresh_rate_ms": 500,
       "show_detailed_stats": false,
       "max_event_feed_size": 10,
       "color_scheme": "default",
       "auto_scroll_feed": true
     }
   }
   ```

**Function Signatures (C++):**
```cpp
class DebugUI {
  public:
    DebugUI(const json& config);
    
    // Start UI loop (blocking, runs until user quits)
    void run();
    
    // Non-blocking update (call periodically)
    void update();
    
    // Post event to display
    void addEvent(const std::string& timestamp, const std::string& event);
    
    // Update status
    void setStatus(const std::string& status, int progressPercent);
    
    // Update metrics
    void updateMetrics(const TelemetryPipeline::TelemetryStats& stats);
    
  private:
    // Drawing functions
    void drawHeader();
    void drawStatusBar();
    void drawEventFeed();
    void drawMetrics();
    void drawActiveTasks();
    void drawStats();
    void drawControlBar();
    
    // Input handling
    void handleKeyInput(char key);
    
    // Screen management
    void clearScreen();
    void setCursorPosition(int x, int y);
    void setTextColor(int color);
};

// Enums for color
enum ConsoleColor {
    BLACK = 0,
    RED = 12,
    GREEN = 10,
    YELLOW = 14,
    CYAN = 11,
    WHITE = 15
};
```

**Testing Criteria:**
- UI renders without flicker
- Keyboard input responsive
- Colors display correctly
- Progress bars animate smoothly
- Memory usage minimal (< 10MB overhead)
- No blocking on I/O operations

---

### Feature #13: Behavioral Anomaly Detection & Auto-Correction

**Objective:** Automatically detect and correct unrealistic behavior patterns.

**Requirements:**

1. **Statistical Anomaly Detection**
   - **Keystroke Timing Anomalies:**
     - Mean IKD deviates > 3σ from historical (ultra-fast or ultra-slow typing suddenly)
     - Error rate suddenly spikes or drops to zero
   - **Mouse Movement Anomalies:**
     - Movement speed suddenly increases/decreases dramatically
     - Jitter amplitude inconsistent
     - Movement pattern changes from curved to linear
   - **Activity Timing Anomalies:**
     - Activity completes in wrong timeframe (task expected 30min done in 2min)
     - Time between activities inconsistent
     - No pauses when pauses expected

2. **Pattern Validation**
   - Consistency checks:
     - Digraph timing consistent (same pairs should have similar delays)
     - Activity chains logically connected (reading → typing makes sense, gaming → coding unlikely)
     - Time-of-day patterns match profile (dev doesn't code at 3am like morning)
   - Plausibility checks:
     - Multiple simultaneous activities flagged (user can't be in 2 places)
     - Impossible transitions (cmd.exe opens, closes, opens again in same millisecond)

3. **Resource Usage Validation**
   - CPU/Memory consistency:
     - Light activity shouldn't spike CPU to 80%
     - Heavy task (coding) should have baseline CPU > 5%
   - Process validation:
     - Task claims to launch app, verify app actually running
     - Check file modifications match declared activities

4. **Anomaly Scoring**
   - Each anomaly gets severity score (0.0-1.0):
     - 0.0-0.3: Minor (can be ignored)
     - 0.3-0.7: Moderate (should be corrected)
     - 0.7-1.0: Severe (must fix immediately)
   - Aggregate score determines action

5. **Auto-Correction Strategies**
   - **Slow Down:** If activities too fast, artificially slow next activities
   - **Speed Up:** If activities too slow, increase pace
   - **Inject Pauses:** Add pauses between activities if too dense
   - **Adjust Timing:** Stretch/compress activity durations to match expectations
   - **Vary Parameters:** Increase keystroke variation if too uniform
   - **Restart Activity:** If anomaly severe, cancel and restart activity

   Example:
   ```
   Anomaly detected: Typing 200 WPM (expected 60-80)
   Severity: 0.85 (severe)
   Action: Slow down next 5 keystroke activities by 40%
   ```

6. **Anomaly Learning**
   - Track corrections made
   - Learn from corrections (adjust future predictions)
   - Build per-profile baseline over time
   - Export learned baselines

7. **Configuration:**
   ```json
   {
     "anomaly_detection": {
       "enabled": true,
       "sensitivity": 0.7,
       "auto_correct": true,
       "correction_strategies": [
         "slow_down",
         "speed_up",
         "inject_pauses",
         "adjust_timing",
         "vary_parameters"
       ],
       "anomaly_rules": "data/templates/anomaly_rules.json",
       "learning_enabled": true,
       "log_anomalies": true
     }
   }
   ```

8. **Anomaly Rules File Example**
   ```json
   {
     "rules": [
       {
         "name": "keystroke_too_fast",
         "type": "keystroke_timing",
         "condition": "ikd_mean < 50",
         "severity": 0.7,
         "correction": "slow_down",
         "correction_amount": 50
       },
       {
         "name": "mouse_jerky",
         "type": "mouse_movement",
         "condition": "jitter_variance > 2.0",
         "severity": 0.6,
         "correction": "adjust_parameters",
         "target_smoothness": 0.9
       },
       {
         "name": "activity_too_fast",
         "type": "activity_timing",
         "condition": "actual_duration < expected * 0.5",
         "severity": 0.8,
         "correction": "stretch_duration"
       }
     ]
   }
   ```

**Function Signatures (C++):**
```cpp
class AnomalyDetector {
  public:
    AnomalyDetector(const json& config);
    
    // Analyze data for anomalies
    struct Anomaly {
      std::string type;          // keystroke, mouse, activity, timing
      std::string description;
      double severity;           // 0.0-1.0
      std::string suggestion;
    };
    
    std::vector<Anomaly> detectAnomalies(const json& recentEvents);
    
    // Check single activity for anomalies
    std::vector<Anomaly> validateActivity(const json& activity);
    
    // Get baseline statistics for profile
    struct Statistics {
      double avgIKD;
      double avgIKDStdDev;
      double avgMouseSpeed;
      double avgActivityDuration;
    };
    Statistics getBaselineStatistics();
    
    // Update baseline based on observations
    void updateBaseline(const json& observation);
    
  private:
    bool isOutlier(double value, double mean, double stdDev);
    json loadAnomalyRules(const std::string& rulesFile);
};

class AnomalyCorrector {
  public:
    AnomalyCorrector(const json& config);
    
    // Apply corrections based on anomalies
    void correctAnomalies(const std::vector<AnomalyDetector::Anomaly>& anomalies);
    
    // Get recommended corrections
    struct Correction {
      std::string strategy;       // slow_down, speed_up, inject_pauses, etc.
      int parameter;              // amount of slowdown %, pause ms, etc.
      int affectedActivities;     // how many activities to apply to
    };
    std::vector<Correction> getRecommendedCorrections(
        const std::vector<AnomalyDetector::Anomaly>& anomalies);
    
    // Apply specific correction
    void applyCorrectionStrategy(const std::string& strategy, 
                                 int parameter);
    
    // Get correction history
    struct CorrectionRecord {
      std::string timestamp;
      std::string anomalyType;
      std::string strategyApplied;
      bool wasSuccessful;
    };
    std::vector<CorrectionRecord> getCorrectionHistory();
    
  private:
    void strategySlowDown(int percentSlower);
    void strategySpeedUp(int percentFaster);
    void strategyInjectPauses(int pauseMs);
    void strategyAdjustTiming(double timeFactor);
    void strategyVaryParameters();
};
```

**Testing Criteria:**
- Anomalies detected with correct severity
- Corrections applied and logged
- Baseline statistics accurate
- Learning improves over time
- No false positives on normal behavior
- Auto-corrections restore normality

---

## 📦 Integration Strategy

### How Tier 4 Integrates with Tier 1 & 2

```
┌─────────────────────────────────────────┐
│     All Tier 1 & Tier 2 Components      │
│  (Mouse, Keystroke, Registry, Activity) │
└──────────────┬──────────────────────────┘
               │ Events & Metrics
               ▼
     ┌─────────────────────────┐
     │     Logger & Telemetry  │
     │   (Feature #11)          │
     └──────────┬──────────────┘
                │ Log Data
                ▼
    ┌──────────────────────────┐
    │   Anomaly Detector       │
    │   (Feature #13)           │
    └────────┬─────────────────┘
             │ Anomalies
             ▼
    ┌──────────────────────────┐
    │  Anomaly Corrector       │
    │  (Feature #13)            │
    └────────┬─────────────────┘
             │ Corrections
             ▼
    ┌──────────────────────────┐
    │    Debug UI              │
    │    (Feature #12)          │
    └──────────────────────────┘
```

- Logger captures ALL events
- Telemetry aggregates metrics
- Anomaly Detector analyzes logs in real-time
- Anomaly Corrector feeds corrections back to Tier 1 & 2 engines
- Debug UI displays everything

### Code Dependencies
- Logger: Standalone (no dependencies on other modules)
- TelemetryPipeline: Depends on Logger
- AnomalyDetector: Reads from Logger output, depends on config
- AnomalyCorrector: Gets input from AnomalyDetector, calls back to Tier 1 engines
- DebugUI: Reads from Logger & TelemetryPipeline, handles user input

---

## 📦 Implementation Requirements

### Code Quality Standards
- **Language:** C++17/20
- **No New External Dependencies:** Use only what Tier 1 & 2 already have
- **Logging Performance:** Async writes, < 1ms per log call
- **UI Responsiveness:** Console drawing < 50ms
- **Memory Bounded:** Logs rotated to prevent unbounded growth

### Threading Model
- Logger runs in separate thread (non-blocking writes)
- Telemetry collection in separate thread (metrics every 60s)
- Debug UI blocks main thread (or separate thread with async input)
- Anomaly detection can run continuously in background

### Error Handling
- Failed log writes don't crash program
- UI errors gracefully fall back to simple output
- Anomaly detection errors logged but don't stop main flow

### Performance Targets
- Logging overhead: < 2% CPU
- TelemetryPipeline: < 1% CPU
- AnomalyDetection: < 5% CPU (runs continuously)
- DebugUI: < 10% CPU (only when active)

---

## 🧪 Testing Strategy

### Unit Tests
```cpp
// Test Logger
Logger& logger = Logger::getInstance();
logger.initialize(config);
logger.logInfo("test", "This is a test message");
// Verify log file created and contains valid JSON

// Test AnomalyDetector
json activity = {{"type", "keystroke"}, {"ikd_mean", 30}};  // Too fast
auto anomalies = detector.detectAnomalies({activity});
assert(anomalies.size() > 0);
assert(anomalies[0].severity > 0.7);

// Test AnomalyCorrector
corrector.applyCorrectionStrategy("slow_down", 30);
// Verify correction logged
// Verify next keystroke activity is slower
```

### Integration Test
1. Run full VMHumanizer with Tier 1 & 2 for 1 hour
2. Monitor Tier 4 logs and UI
3. Verify:
   - All events logged
   - Metrics collected accurately
   - Anomalies detected and corrected
   - UI displays correctly
   - No performance degradation

---

## 📝 Deliverables

After implementing Tier 4, provide:

1. **Complete source code** for all 3 features (8 modules total)
2. **Anomaly rules file** (anomaly_rules.json) with comprehensive patterns
3. **Example logs** (activities.log, metrics.log, anomalies.log) showing output
4. **UI screenshots** (recorded or documented)
5. **Performance analysis** showing overhead percentages
6. **Integration tests** demonstrating Tier 1 + 2 + 4 working together
7. **Log analysis guide** - how to parse and analyze generated logs
8. **Debugging guide** - how to use Debug UI and logs for troubleshooting

---

## ⚠️ Critical Constraints

- **Performance:** Tier 4 overhead must not exceed 15% total CPU
- **Memory:** Logs must be bounded (rotation required)
- **Blocking:** Logger & Telemetry must be non-blocking
- **Compatibility:** Must work with Tier 1 & 2 without modifications
- **Observability:** All features must be loggable and debuggable

---

## 🚀 Success Criteria

- ✅ All 3 Tier 4 features fully implemented
- ✅ Code compiles with Tier 1 & 2
- ✅ Logging captures all meaningful events
- ✅ Debug UI renders and handles input correctly
- ✅ Anomaly detection accurate (< 5% false positives)
- ✅ Auto-corrections effective (restore behavior to normal)
- ✅ Performance overhead < 15% CPU
- ✅ Logs useful for debugging and analysis

---

## 🔍 Example End-to-End Flow

```
1. Activity Engine starts "Coding Session"
   → Logger logs: {"event_type": "activity_started", "activity": "Coding"}
   
2. During activity, Keystroke Emulator types
   → Logger logs: {"event_type": "keystroke", "ikd_mean": 125, ...}
   
3. Anomaly Detector reads logs periodically
   → Detects: "Typing suddenly 40 WPM faster than baseline"
   → Severity: 0.75 (moderate-high)
   
4. Anomaly Corrector acts
   → Applies strategy: "slow_down by 30%"
   → Logs: {"event_type": "correction_applied", "strategy": "slow_down"}
   
5. Debug UI shows all of this in real-time
   → User sees warning about typing speed
   → Sees correction being applied
   → Notices next keystroke activities are slower

6. User can press [D] to see detailed logs
   → Opens log file showing all events and corrections
   → Can analyze patterns and learn anomaly detector behavior
```

---

**Ready to complete the observation stack? Start with Feature #11 (Logging) or Feature #12 (UI) — they support everything else.**

