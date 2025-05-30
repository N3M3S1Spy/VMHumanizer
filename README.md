# 🧠 VMHuminazor

**VMHuminazor** ist ein C++-basiertes Tool, das virtuelle Maschinen so manipuliert, dass sie wie ein real genutzter Windows-PC wirken. Ziel ist es, forensische Artefakte, Nutzerverhalten und Systemnutzung zu simulieren – für Sandbox-Evasion, Honeypot-Täuschung oder Red Team-Simulationen.

---

## 🎯 Ziele

- Erzeugung glaubwürdiger Nutzungs-Spuren
- Manipulation typischer Anti-VM-Evasion-Indikatoren
- Anpassbare Profile (z. B. „Privatnutzer“, „Entwickler“, „Büroangestellter“)
- Keine externen Abhängigkeiten (reines WinAPI/C++)

---

## 📦 Kernfunktionen (ToDo & Status)

| Funktion                                           | Beschreibung | Status       | Priorität |
|----------------------------------------------------|--------------|--------------|-----------|
| 🔧 Fake User Profile                               | Benutzername, Ordnerstruktur, Profilbild ändern | ✅ Implementiert | Essential |
| 📁 Realistische Dateistruktur                      | Fake-Dokumente (.docx, .jpg etc. mit Timestamps) | ⏳ In Planung    | Essential |
| 🧾 Windows-Eventlog-Faker                          | Boot-, Login-, Software-Events erzeugen | ⏳ Offen         | Advanced  |
| 🌐 Edge-Browser-Verlauf                            | Einträge in SQLite `History` einfügen | ✅ Implementiert | Essential |
| 🌍 Bookmarks & Downloads                           | Bookmarks in Edge, Fake-Downloads in Datei | ⏳ Offen         | Essential |
| 🧠 Registry MRU Simulation                         | Word, RunMRU, TypedURLs, UserAssist-Einträge | ⏳ Offen         | Essential |
| 🕑 Dateisystem-Artefakte                           | LNK-Dateien, Prefetch-Dateien, RecentDocs | ⏳ Offen         | Advanced  |
| 🖨️ Fake Druckaufträge                              | Historische Einträge im Spooler simulieren | ⏳ Offen         | Optional  |
| 📧 Fake Outlook-E-Mail-Artefakte                   | OST-Dateien, E-Mail-Indikatoren | ⏳ Offen         | Optional  |
| 🖼️ User-Hintergrund, Taskbar-Pins, Themes         | Optisches User-Profil abrunden | ⏳ Offen         | Optional  |
| 🛡️ Anti-VM-Hardening (Name, MAC, Registry)         | VBox/QEMU-Spuren entfernen | ⏳ Offen         | Essential |
| ⚙️ Konfigurierbares Nutzerprofil (JSON)            | Profilauswahl: „Student“, „Entwickler“ etc. | ⏳ Offen         | Advanced  |
| 💾 Snapshot-Funktion (Zurücksetzen)                | Vorher-/Nachher-Vergleich ermöglichen | ⏳ Offen         | Optional  |

---

## 📁 Verzeichnisstruktur (Beispiel)

```plaintext
VMHuminazor/
├── src/
│   ├── main.cpp
│   ├── edge_history.cpp
│   ├── file_generator.cpp
│   ├── registry_simulator.cpp
├── data/
│   ├── profiles/
│   │   ├── developer.json
│   │   └── office_worker.json
│   └── templates/
│       ├── dummy.docx
│       └── dummy.jpg
├── README.md
├── LICENSE