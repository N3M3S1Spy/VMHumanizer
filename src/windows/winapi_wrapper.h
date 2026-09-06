#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <cstdint>

namespace vmh {

namespace WinAPI {
    bool setMousePosition(int x, int y);
    std::pair<int, int> getMousePosition();
    void sendMouseMove(int x, int y);

    void sendKeyDown(WORD vkCode);
    void sendKeyUp(WORD vkCode);
    void sendKeyPress(WORD vkCode, int holdMs = 50);
    void sendCharInput(char ch);
    void sendCharDown(char ch);
    void sendCharUp(char ch);

    LONG regCreateKey(HKEY hRoot, const std::wstring& subKey, HKEY& outKey);
    LONG regSetString(HKEY hKey, const std::wstring& name, const std::wstring& value);
    LONG regSetDword(HKEY hKey, const std::wstring& name, DWORD value);
    LONG regSetBinary(HKEY hKey, const std::wstring& name, const std::vector<uint8_t>& data);
    LONG regDeleteValue(HKEY hKey, const std::wstring& name);
    LONG regDeleteKey(HKEY hRoot, const std::wstring& subKey);
    std::vector<std::wstring> regEnumValues(HKEY hRoot, const std::wstring& subKey);
    DWORD regGetValueCount(HKEY hRoot, const std::wstring& subKey);
    void regCloseKey(HKEY hKey);

    bool writeEventLog(const std::wstring& sourceName,
                       WORD eventType,
                       DWORD eventID,
                       const std::wstring& message);

    bool registerEventSource(const std::wstring& sourceName);

    std::wstring getComputerName();
    std::wstring getCurrentUserName();
    std::wstring getCurrentUserSID();

    void sleepMs(int ms);
}

} // namespace vmh
