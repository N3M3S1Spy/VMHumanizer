# VMHumanizer - Advanced Feature Roadmap

**Version:** 1.0  
**Zielplattform:** Windows 10/11 (Pure WinAPI/C++)  
**Zweck:** Authentische Simulation von Nutzerverhalten für forensische Artefakt-Generierung und VM-Evasion  

---

## Roadmap Übersicht (13 fortgeschrittene Features)

### ✅ Tier 1: Kern-Humanisierung (Essential)

#### 1. **Behavioral Mouse Movement Simulator**
**Kategorie:** Verhaltens-Simulation | **Komplexität:** Medium

**Beschreibung:**
Implementiere ein C++-Modul, das realistische, nicht-lineare Mausbewegungen mit Bezier-Kurven und stochastischen Mikrovibrationen generiert. Statt linearer Punkt-zu-Punkt-Bewegungen soll die Maus:
- Sanfte, quadratische Bezier-Interpolationen zwischen Zielpunkten verwenden
- Feine, zufällige Jitter-Schwankungen (±2-5px) simulieren, die typisch für menschliche Hand-Tremor sind
- Variable Geschwindigkeitsprofile implementieren (schneller Start, langsamer am Ziel, oder umgekehrt)
- Pausen und "Denkpausen" einfügen, wenn bestimmte UI-Elemente angevisiert werden (z.B. Knöpfe)

**Technische Implementierung:**
- `mouse_behavior.cpp` mit Bezier-Kurven-Engine
- LowLevelMouseProc Hook für realistische Input-Events
- Konfigurierbare Parameter via JSON-Profile (Z.B. `mouse_speed_range: [50, 200ms]`, `jitter_amplitude: [2, 5]`)

**Nutzen:**
- Deutlich realistischere Mausbewegungen als maschinelle perfekte Bewegungen
- Verhindert einfache behaviorale Erkennungsregeln durch Analyse-Tools
- Authentische VM-Humanisierung

**Abhängigkeiten:** WinAPI SetCursorPos(), GetCursorPos(), mouseMove-Events

---

#### 2. **Realistic Keystroke Timing & Behavior Emulation**
**Kategorie:** Verhaltens-Simulation | **Komplexität:** Medium-High

**Beschreibung:**
Implementiere eine C++-Komponente für authentische Tastatur-Eingabe-Simulation mit:
- Realistische **Inter-Key Delays** (IKD) - Zeit zwischen Key-Down und Key-Up, normalverteilte Verzögerungen (typisch 40-200ms pro Taste)
- **Digraph-basierte Timing-Muster** - unterschiedliche Eingabe-Geschwindigkeiten basierend auf häufigen Buchstabenkombinationen (z.B. "th", "er" sind schneller als "qz")
- Gelegentliche **Tippfehler-Simulation** mit automatischer Korrektur (Fehlerrate ~5-10%, wie bei echten Benutzern)
- **Pause-Muster** beim Umschalten zwischen Textblöcken oder beim "Nachdenken"
- Optional: Ergonomische Typing-Variationen (linke vs. rechte Hand für bestimmte Tasten)

**Technische Implementierung:**
- `keystroke_emulator.cpp` mit WinAPI SendInput()
- Markov-Chain oder Lookup-Table für Digraph-Delays
- Fehler-Injection-Engine mit Backspace-Simulation
- Konfigurierbare Nutzerprofile ("schneller Tipper", "langsamer, vorsichtiger Tipper")

**Nutzen:**
- Bypass von Keystroke-Timing-Analyse (forensische Erkennungsmethode)
- Realistische Benutzersimulation für Honeyports & Red-Team-Ops
- Adaptive Verhalten basierend auf Nutzertyp

---

#### 3. **Advanced Registry MRU (Most Recently Used) Manipulation**
**Kategorie:** System-Artefakte | **Komplexität:** High

**Beschreibung:**
Erweitere die Registry-Manipulation um komplexe, zeitabhängige MRU-Einträge:
- **TypedURLs & TypedURLMRU** - realistische Browser-URL-Historie in `HKCU\Software\Microsoft\Internet Explorer\TypedURLs`
- **UserAssist Entries** - kodierte Zugriffsmuster für executables & Dokumente in `HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\UserAssist\{GUID}\Count`
- **Office MRU** (Word, Excel, PowerPoint) - letzten Dateien mit richtigen Timestamps
- **ShellItems & Lnk-Einträge** - Recent Items Manipulation
- **App-Suchverlauf** (Windows Search, Cortana) für realistische Suchaktivitäten
- **Automatic & Manual Cleanup** - alte Einträge löschen, um ein echtes Verhalten zu simulieren (z.B. regelmäßige Cleanup wie echte Nutzer)

**Technische Implementierung:**
- `registry_mru_engine.cpp` mit WinAPI RegCreateKeyEx(), RegSetValueEx(), RegQueryValueEx()
- Unicode/Encoding-Engine für UserAssist ROT-13 Dekodierung
- Zeitstempel-Generator mit naturalistischen Abständen (nicht all auf einmal)
- JSON-basierte MRU-Datei als Vorlage für verschiedene Nutzerprofile

**Nutzen:**
- Täuscht forensische Anti-Forensik-Analysen
- Macht VM-Aktivitäten wie echte Benutzeraktivitäten zu wirken
- Wichtig für Incident Response & Honeypot-Szenarien

---

#### 4. **Fake Event Log Injection (Windows Event Viewer)**
**Kategorie:** System-Artefakte | **Komplexität:** High

**Beschreibung:**
Erstelle ein C++-Modul zur Erzeugung glaubwürdiger Windows Event Logs:
- **Security Event Logs** (Event ID 4688 - Process Creation, 4720 - User Account Created, etc.)
- **System Event Logs** (Event ID 6005 - Boot-Zeit, 6006 - Shutdown, etc.)
- **Application Event Logs** - typische Fehler & Warnungen von Office, Visual Studio, etc.
- **PowerShell Event Logs** - Execution Records mit realistischen Script-Aktivitäten
- **Authentische Timestamps** - zeitlich plausibel angeordnete Events, nicht alle zur gleichen Zeit
- **Korrekte Event-Struktur** - Quelle, EventID, Kategorie, Message-String in korrektem Format

**Technische Implementierung:**
- `event_log_injector.cpp` mit WinAPI ReportEventW()
- Event-Payload-Builder mit WEVT-Manifest-Parsing
- Zeitstempel-Randomisierung (realistische Verteilung)
- Vorlage-basierte Event-Library (verschiedene Szenarien)

**Nutzen:**
- Forensiker finden plausible "Aktivitätsbeweise" in Event Logs
- Komplettere Täuschung als nur Datei-Artefakte
- Critical für APT-Simulationen & Incident Response-Training

---

### 🚀 Tier 2: Erweiterte Automatisierung (Advanced)

#### 5. **Intelligent Task-Switching & Activity Cycling Engine**
**Kategorie:** Verhaltens-Automatisierung | **Komplexität:** High

**Beschreibung:**
Entwickle ein State-Machine-basiertes System, das typische menschliche Aktivitätsmuster simuliert:
- **Task-Profiles** - vordefinierte Aktivitätszyklen (z.B. "Office Worker": E-Mails → Spreadsheet-Bearbeitung → Break → Browser)
- **Context-Aware Timing** - Pausen-Dauer basierend auf:
  - Tageszeit (längere Pausen mittags, kürzere am Morgen)
  - Simulierter Workload (variable Aktivitätsmuster)
  - Wochenend-Muster (weniger Aktivität am Wochenende)
- **Application Switching** - realistische Wechsel zwischen Apps (Open Edge → Öffne Outlook → Zurück zu Excel)
- **Realistic Idle Times** - Pausen zwischen Aktivitäten mit variabler Länge
- **Event-Driven Triggers** - externe Trigger (z.B. "New Email" → Switch zu Outlook)
- **Activity Logging** - Tracking aller simulierten Aktivitäten für Audit/Review

**Technische Implementierung:**
- `activity_engine.cpp` mit Event-Dispatcher & State-Machine
- JSON-basierte Task-Profile (s. `data/profiles/office_worker.json`)
- Windows API ShellExecute() für App-Launches
- Timer-basierte State-Übergänge mit Zufallsvariabilität

**Nutzen:**
- Vollautomatische, realistische 24/7-Simulation ohne manuelle Intervention
- Täuscht Time-Series-Analysen (z.B. "Wann ist dieser PC aktiv?")
- Perfekt für Honeypots & kontinuierliche Evasion

---

#### 6. **Prefetch & LNK File Artefact Generator**
**Kategorie:** System-Artefakte | **Komplexität:** Medium-High

**Beschreibung:**
Generiere Windows-eigene Dateiartefakte, die forensische Analysen täuschen:
- **Prefetch Files** (`C:\Windows\Prefetch\*.pf`) - Binärdateien, die Windows beim Programm-Start erstellt, um zukünftige Starts zu beschleunigen
  - Korrekte Struktur (PF-Format Version 17/30)
  - Richtige Timestamps (First Run, Last Run)
  - Akkurate Datei-Dependencies & Module-Listeneinträge
- **LNK Files** (Shortcuts) - `.lnk` Dateien in Recent & Links mit:
  - Korrekte Shell Item Identifier List
  - Realistische Arguments & Target-Pfade
  - ExtraData Blocks mit MachineID & NetBIOS Name
- **Recent Documents** in `%AppData%\Microsoft\Windows\Recent\`
- **Automatic Cleanup** - alte Prefetch-Dateien regelmäßig löschen (wie Windows es tut)

**Technische Implementierung:**
- `prefetch_generator.cpp` & `lnk_file_generator.cpp`
- Binary-Format-Parser für PF & LNK
- WinAPI CreateFileW(), WriteFile() für korrektes Binary Writing
- Template-Dateien für Strukturen

**Nutzen:**
- Täuscht "Program Execution History" Forensik
- Authentisches Artefakt-Set für glaubwürdige VM-Nutzung
- Critical für Threat-Hunting-Evasion

---

#### 7. **Configurable User Profile Generator with Dynamic Data Population**
**Kategorie:** System-Konfiguration | **Komplexität:** Medium

**Beschreibung:**
Erweitere die Profilverwaltung zu einem vollständigen, konfigurierbaren System:
- **JSON-basierte Master-Profile** mit variablen Attributen:
  ```json
  {
    "profile_name": "software_developer",
    "user_attributes": {
      "full_name": "Max Mustermann",
      "occupation": "Software Developer",
      "interests": ["GitHub", "Stack Overflow", "Tech Blogs"],
      "activity_intensity": 8.5
    },
    "file_patterns": { ... },
    "registry_patterns": { ... },
    "browser_patterns": { ... }
  }
  ```
- **Dynamic Data Filling** - Zufallsgeneratoren für realistische Daten (Name, E-Mail, Telefon)
- **Locale-Specific Customization** - Profile für verschiedene Länder/Sprachen (deutsche vs. englische Dateinamen, etc.)
- **Multi-Profile Management** - Wechsel zwischen mehreren Profilen für verschiedene VMs
- **Export/Import-Funktionalität** - Profile zwischen Systemen portieren
- **Version-Control für Profile** - Track Änderungen & Rollback-Möglichkeiten

**Technische Implementierung:**
- `profile_engine.cpp` mit JSON-Parser (nlohmann/json oder WinAPI-basiert)
- `profile_manager.cpp` für CRUD-Operationen
- Faker-Library für Daten-Generierung (Namen, Adressen, etc.)
- Registry & File-System Staging vor Anwendung

**Nutzen:**
- Massive Zeitersparnis bei Multi-VM-Setups
- Konsistente, wiederholbare Simulationen
- Einfache Anpassung ohne Code-Änderungen

---

#### 8. **Network Traffic & Connection Pattern Simulator**
**Kategorie:** System-Verhalten | **Komplexität:** High

**Beschreibung:**
Simuliere authentische Netzwerk-Aktivitäten auf Betriebssystem-Ebene:
- **DNS-Requests** - regelmäßige DNS-Lookups für häufig besuchte Seiten
- **Network Connection Logging** - lokal in Windows Log einträge ("Connection established", "Connection closed")
- **Firewall Log Manipulation** - Windows Firewall Activity Logs mit simulierten Blocking-Events
- **Winsock-Events** - TCP/UDP Connection Events im Event Viewer
- **Cached ARP-Entries** - realistische ARP-Tabellen für "gesehene" Netzwerk-Geräte
- **Network Share Access Logging** - Spuren von SMB-Verbindungen zu Shared Drives
- **VPN-Tunnels simulation** (optional) - Zeichen von VPN-Nutzung in Netzwerk-Configs

**Technische Implementierung:**
- `network_simulator.cpp` mit WinAPI WSASocket()
- Event Log Injection für Network-Events
- Registry-Manipulation für Connection History
- Optional: WinDivert oder WinPcap für Paket-Level-Kontrolle (falls verfügbar)

**Nutzen:**
- Komplettere Täuschung von Netzwerk-basierten Erkennungen
- Realistische "Browsing-Spuren" im Netzwerk-Log
- Wichtig für Sandbox-Evasion (manche Sandboxes prüfen Netzwerk-Verhalten)

---

### 🔬 Tier 3: KI & Intelligente Automatisierung (Research-Level)

#### 9. **LLM-Driven Adaptive Behavior Engine**
**Kategorie:** AI-Integration | **Komplexität:** Research-Level

**Beschreibung:**
Integriere einen Large Language Model (Claude/GPT) für dynamische, kontextabhängige Verhaltensanpassung:
- **Context-Aware Decision Making** - LLM entscheidet basierend auf aktuellem System-State, was als nächste Aktivität sinnvoll ist
  - Input: "Es ist Freitagabend, der Benutzer ist 'Software Developer', die CPU-Last ist niedrig"
  - Output: LLM schlägt vor: "Browsen von Tech-News" oder "Arbeiten an persönlichem Projekt"
- **Dynamic Workflow Generation** - LLM generiert plausible Aktivitätsketten für verschiedene Profile
- **Natural Language Instruction Support** - Benutzer kann in Englisch/Deutsch beschreiben, welches Verhalten simuliert werden soll
  - "Simulate a typical Friday afternoon for a developer after finishing a project"
  - → Engine generiert Aktivitätskette mit Timing & Details
- **Anomaly Detection & Correction** - LLM erkennt unnatürliche Muster und korrigiert sie

**Technische Implementierung:**
- `llm_adapter.cpp` mit REST-API Calls zu Claude/OpenAI
- Prompt-Engineering für Verhaltens-Generierung
- Caching von LLM-Responses für Performance
- Fallback-Logik wenn API nicht erreichbar
- Optional: Lokales Inference-Modell (via ONNX Runtime)

**Nutzen:**
- Hochgradig adaptive, realistische Simulationen
- Weniger vordefinierte Regeln, mehr echte "Intelligenz"
- Potential für neue, unerwartete Verhaltensweisen (schwer zu erkennen)

---

#### 10. **Temporal Context-Aware Pause Injection**
**Kategorie:** Verhaltens-Simulation | **Komplexität:** Medium

**Beschreibung:**
Implementiere intelligente "Denkpausen" basierend auf realen Kontexten:
- **Cognitive Load Awareness** - längere Pausen bei komplexen Aktivitäten (z.B. Code-Review) vs. kurze bei repetitiven (z.B. E-Mail lesen)
- **System-Load Responsive** - Pausen basierend auf aktueller CPU/Memory-Last (wenn VM stark belastet ist, "denkt" der Nutzer länger)
- **Time-of-Day Patterns** - längere Pausen am Nachmittag ("Mittagstief"), kürzere am Morgen
- **Task-Switching Pauses** - kürzere Pausen zwischen eng verwandten Tasks (Word → Excel), längere zwischen nicht verwandten (Coding → Gaming)
- **Distraction Simulation** - gelegentliche Pausen/Abschweifungen (z.B. "User ist 2min abgelenkt")

**Technische Implementierung:**
- `pause_engine.cpp` mit Heuristik-basiertem Pause-Kalkulator
- System-Performance-Monitoring via WinAPI GetSystemTimes()
- Configurable Pause-Parameter in JSON-Profilen
- Optional: Integration mit Activity Engine für Context-Awareness

**Nutzen:**
- Realistic "Human Pacing" statt maschineller Konstanz
- Schwieriger für Behavioral Analysis zu erkennen
- Authentische Simulation ohne viel Konfiguration

---

### 📊 Tier 4: Observability & Telemetrie

#### 11. **Structured Logging & Activity Telemetry Pipeline**
**Kategorie:** Observability | **Komplexität:** Medium

**Beschreibung:**
Implementiere umfassendes Logging aller simulierten Aktivitäten für Debugging & Auditing:
- **Structured JSON Logs** - alle Aktivitäten werden in strukturiertem Format geloggt:
  ```json
  {
    "timestamp": "2026-09-02T14:23:45Z",
    "activity_type": "keystroke_input",
    "duration_ms": 125,
    "target_app": "notepad.exe",
    "confidence_score": 0.95,
    "activity_chain_id": "chain_001"
  }
  ```
- **Real-Time Metrics** - Performance-Metriken (CPU-Last, Memory-Footprint der VMH-Engine)
- **Activity Statistics** - Aggregierte Statistiken über Aktivitätsmuster
- **Error & Exception Logging** - detailliertes Logging von Ausfällen, mit Stack-Traces
- **Audit Trail** - Vollständige Nachverfolgung aller Registry/File-Modifikationen
- **Log Rotation & Archiving** - automatisches Archivieren alter Logs

**Technische Implementierung:**
- `logger.cpp` mit JSON-Output (nlohmann/json)
- Multi-threaded Logging für Performance
- Optional: ETW (Event Tracing for Windows) Integration
- Config-gesteuerte Log-Level (DEBUG, INFO, WARN, ERROR)

**Nutzen:**
- Einfaches Debugging komplexer Verhaltensszenarien
- Audit-Trail für Sicherheits-Validierung
- Performance-Monitoring der VMH-Engine selbst
- Exportierbare Reports für Analyse

---

#### 12. **Visual Activity Debugger (Inline Reporting)**
**Kategorie:** Observability | **Komplexität:** Medium-High

**Beschreibung:**
Erstelle ein Console-basiertes Real-Time-Dashboard zur Visualisierung laufender Aktivitäten:
- **Activity Feed** - scrollbare Liste der letzten simulierten Ereignisse
- **Performance Metrics** - Live-Anzeige von CPU/Memory/Disk-I/O der VMH-Engine
- **Active Tasks** - derzeit laufende Tasks mit Fortschrittsbalken
- **Event Timeline** - visuelle Darstellung der Aktivitätsmuster über Zeit
- **Profile Info** - aktuell geladenes Profil & Konfiguration
- **Error Alerts** - Warnung bei Fehlern oder unerwarteten Events

**Technische Implementierung:**
- `debug_ui.cpp` mit Windows Console API (für farbigen Text, Cursor-Positioning)
- Optional: NCurses-Style Library für bessere TUI-Unterstützung
- Async Event-Rendering (nicht-blockierend)
- Keyboard-Shortcuts zum Pausieren/Fortsetzen/Anpassen

**Nutzen:**
- Einfache Visualisierung ohne externe Tools
- Schnelle Diagnostik von Problemen
- Nützlich für Live-Demo & Testing

---

#### 13. **Behavioral Anomaly Detection & Auto-Correction**
**Kategorie:** Intelligente Fehlerbehandlung | **Komplexität:** High

**Beschreibung:**
Implementiere ein System zur Erkennung und automatischen Korrektur von unnatürlichen Mustern:
- **Statistical Outlier Detection** - erkennt ungewöhnliche Aktivitäten (z.B. 10x schnellere Keystrokes als normal)
- **Pattern Validation** - verifiziert, dass Aktivitätsketten logisch zusammenpassen
- **Timing Sanity Checks** - stellt sicher, dass Timestamps realistisch sind (nicht mehrere Events zur selben Millisekunde)
- **Resource Usage Validation** - prüft, dass Prozesse nicht mehr Ressourcen nutzen als möglich
- **Auto-Correction Strategies**:
  - Langsamer machen, wenn Aktivitäten zu schnell ablaufen
  - Pausen einfügen, wenn zu viele Events schnell hintereinander
  - Timing-Anpassungen bei unplausiblen Patterns
- **Detailed Anomaly Reports** - Logging was korrigiert wurde & warum

**Technische Implementierung:**
- `anomaly_detector.cpp` mit Statistical Analysis (mean, std-dev)
- Configurable Schwellenwerte in JSON
- Auto-Correction Engine mit verschiedenen Strategien
- Detailliertes Anomaly-Logging

**Nutzen:**
- Automatische Qualitätssicherung ohne manuelle Intervention
- Verhindert forensisch verdächtige Muster
- Continuous Improvement durch Anomaly-Learning

---

## Implementation Priorität & Abhängigkeiten

### Quick-Win Phase (Woche 1-2)
1. **#2 - Keystroke Timing Emulation** (Medium-High Komplexität, großer Impact)
2. **#3 - Advanced Registry MRU** (Fundation für weitere Registry-Features)

### Core Phase (Woche 3-6)
3. **#1 - Mouse Movement Simulator**
4. **#4 - Event Log Injection**
5. **#7 - Configurable Profile Generator**

### Advanced Phase (Woche 7-10)
6. **#5 - Task-Switching Engine**
7. **#6 - Prefetch & LNK Generator**
8. **#8 - Network Traffic Simulator**

### Research & Polish Phase (Woche 11+)
9. **#9 - LLM-Driven Behavior** (experimental)
10. **#10 - Temporal Context-Aware Pauses**
11. **#11 - Structured Logging**
12. **#12 - Visual Debugger**
13. **#13 - Anomaly Detection**

---

## Tech-Stack Compliance

✅ **Alle Features verwenden ausschließlich:**
- Pure C++ (C++17/20)
- Windows API (keine externe Libraries außer ggf. nlohmann/json für Profile)
- Native Windows-Komponenten (Registry, Event Logs, WinAPI)
- Keine Abhängigkeiten von Python, .NET, Node.js, etc.

---

## Success Metrics

- **Forensic Evasion Rate:** > 85% der generierten Artefakte halten forensischer Analyse Stand
- **Behavioral Authenticity:** LLM-basierte Evaluation gibt den Aktivitäten Score > 8/10
- **Performance:** VMH-Engine < 5% CPU-Overhead, < 50MB Memory-Footprint
- **Deployment:** Single EXE, keine separaten Dependencies, <15MB Größe

---

**Nächste Schritte:** Auswahl und Priorisierung durch den Nutzer.
