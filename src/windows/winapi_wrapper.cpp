#include "winapi_wrapper.h"
#include "../common/utils.h"
#include <sddl.h>
#include <memory>

#pragma comment(lib, "advapi32.lib")

namespace vmh {
namespace WinAPI {

bool setMousePosition(int x, int y) {
    return SetCursorPos(x, y) != 0;
}

std::pair<int, int> getMousePosition() {
    POINT pt;
    GetCursorPos(&pt);
    return { pt.x, pt.y };
}

void sendMouseMove(int x, int y) {
    INPUT input = {};
    input.type = INPUT_MOUSE;
    input.mi.dx = static_cast<LONG>(x * (65535.0 / GetSystemMetrics(SM_CXSCREEN)));
    input.mi.dy = static_cast<LONG>(y * (65535.0 / GetSystemMetrics(SM_CYSCREEN)));
    input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
    SendInput(1, &input, sizeof(INPUT));
}

void sendKeyDown(WORD vkCode) {
    INPUT input = {};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = vkCode;
    input.ki.dwFlags = 0;
    SendInput(1, &input, sizeof(INPUT));
}

void sendKeyUp(WORD vkCode) {
    INPUT input = {};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = vkCode;
    input.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &input, sizeof(INPUT));
}

void sendKeyPress(WORD vkCode, int holdMs) {
    sendKeyDown(vkCode);
    sleepMs(holdMs);
    sendKeyUp(vkCode);
}

void sendCharInput(char ch) {
    INPUT inputs[2] = {};
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wScan = static_cast<WORD>(ch);
    inputs[0].ki.dwFlags = KEYEVENTF_UNICODE;
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wScan = static_cast<WORD>(ch);
    inputs[1].ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
    SendInput(2, inputs, sizeof(INPUT));
}

void sendCharDown(char ch) {
    INPUT input = {};
    input.type = INPUT_KEYBOARD;
    input.ki.wScan = static_cast<WORD>(ch);
    input.ki.dwFlags = KEYEVENTF_UNICODE;
    SendInput(1, &input, sizeof(INPUT));
}

void sendCharUp(char ch) {
    INPUT input = {};
    input.type = INPUT_KEYBOARD;
    input.ki.wScan = static_cast<WORD>(ch);
    input.ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
    SendInput(1, &input, sizeof(INPUT));
}

LONG regCreateKey(HKEY hRoot, const std::wstring& subKey, HKEY& outKey) {
    DWORD disposition;
    return RegCreateKeyExW(hRoot, subKey.c_str(), 0, nullptr,
                           REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
                           nullptr, &outKey, &disposition);
}

LONG regSetString(HKEY hKey, const std::wstring& name, const std::wstring& value) {
    return RegSetValueExW(hKey, name.c_str(), 0, REG_SZ,
                          reinterpret_cast<const BYTE*>(value.c_str()),
                          static_cast<DWORD>((value.size() + 1) * sizeof(wchar_t)));
}

LONG regSetDword(HKEY hKey, const std::wstring& name, DWORD value) {
    return RegSetValueExW(hKey, name.c_str(), 0, REG_DWORD,
                          reinterpret_cast<const BYTE*>(&value), sizeof(DWORD));
}

LONG regSetBinary(HKEY hKey, const std::wstring& name, const std::vector<uint8_t>& data) {
    return RegSetValueExW(hKey, name.c_str(), 0, REG_BINARY,
                          data.data(), static_cast<DWORD>(data.size()));
}

LONG regDeleteValue(HKEY hKey, const std::wstring& name) {
    return RegDeleteValueW(hKey, name.c_str());
}

LONG regDeleteKey(HKEY hRoot, const std::wstring& subKey) {
    return RegDeleteKeyW(hRoot, subKey.c_str());
}

std::vector<std::wstring> regEnumValues(HKEY hRoot, const std::wstring& subKey) {
    std::vector<std::wstring> values;
    HKEY hKey;
    if (RegOpenKeyExW(hRoot, subKey.c_str(), 0, KEY_READ, &hKey) != ERROR_SUCCESS)
        return values;

    DWORD index = 0;
    wchar_t name[512];
    DWORD nameLen;
    while (true) {
        nameLen = 512;
        LONG result = RegEnumValueW(hKey, index, name, &nameLen,
                                     nullptr, nullptr, nullptr, nullptr);
        if (result != ERROR_SUCCESS) break;
        values.emplace_back(name, nameLen);
        ++index;
    }
    RegCloseKey(hKey);
    return values;
}

DWORD regGetValueCount(HKEY hRoot, const std::wstring& subKey) {
    HKEY hKey;
    if (RegOpenKeyExW(hRoot, subKey.c_str(), 0, KEY_READ, &hKey) != ERROR_SUCCESS)
        return 0;

    DWORD count = 0;
    RegQueryInfoKeyW(hKey, nullptr, nullptr, nullptr, nullptr, nullptr,
                     nullptr, &count, nullptr, nullptr, nullptr, nullptr);
    RegCloseKey(hKey);
    return count;
}

void regCloseKey(HKEY hKey) {
    if (hKey) RegCloseKey(hKey);
}

bool writeEventLog(const std::wstring& sourceName,
                   WORD eventType,
                   DWORD eventID,
                   const std::wstring& message) {
    HANDLE hEventLog = RegisterEventSourceW(nullptr, sourceName.c_str());
    if (!hEventLog) return false;

    const wchar_t* msgPtr = message.c_str();
    BOOL result = ReportEventW(hEventLog, eventType, 0, eventID,
                                nullptr, 1, 0, &msgPtr, nullptr);
    DeregisterEventSource(hEventLog);
    return result != 0;
}

bool registerEventSource(const std::wstring& sourceName) {
    std::wstring keyPath = L"SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\" + sourceName;
    HKEY hKey;
    LONG result = RegCreateKeyExW(HKEY_LOCAL_MACHINE, keyPath.c_str(), 0, nullptr,
                                   REG_OPTION_NON_VOLATILE, KEY_SET_VALUE,
                                   nullptr, &hKey, nullptr);
    if (result != ERROR_SUCCESS) return false;

    wchar_t modulePath[MAX_PATH];
    GetModuleFileNameW(nullptr, modulePath, MAX_PATH);

    RegSetValueExW(hKey, L"EventMessageFile", 0, REG_EXPAND_SZ,
                   reinterpret_cast<const BYTE*>(modulePath),
                   static_cast<DWORD>((wcslen(modulePath) + 1) * sizeof(wchar_t)));

    DWORD typesSupported = EVENTLOG_INFORMATION_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_ERROR_TYPE;
    RegSetValueExW(hKey, L"TypesSupported", 0, REG_DWORD,
                   reinterpret_cast<const BYTE*>(&typesSupported), sizeof(DWORD));

    RegCloseKey(hKey);
    return true;
}

std::wstring getComputerName() {
    wchar_t name[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = MAX_COMPUTERNAME_LENGTH + 1;
    if (GetComputerNameW(name, &size)) return std::wstring(name, size);
    return L"DESKTOP-VMH";
}

std::wstring getCurrentUserName() {
    wchar_t name[256];
    DWORD size = 256;
    if (GetUserNameW(name, &size)) return std::wstring(name, size - 1);
    return L"User";
}

std::wstring getCurrentUserSID() {
    HANDLE hToken;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
        return L"S-1-5-21-0-0-0-1000";

    DWORD tokenInfoLen = 0;
    GetTokenInformation(hToken, TokenUser, nullptr, 0, &tokenInfoLen);

    std::vector<BYTE> buffer(tokenInfoLen);
    if (!GetTokenInformation(hToken, TokenUser, buffer.data(), tokenInfoLen, &tokenInfoLen)) {
        CloseHandle(hToken);
        return L"S-1-5-21-0-0-0-1000";
    }

    TOKEN_USER* pTokenUser = reinterpret_cast<TOKEN_USER*>(buffer.data());
    LPWSTR sidStr = nullptr;
    ConvertSidToStringSidW(pTokenUser->User.Sid, &sidStr);
    std::wstring result = sidStr ? sidStr : L"S-1-5-21-0-0-0-1000";
    if (sidStr) LocalFree(sidStr);
    CloseHandle(hToken);
    return result;
}

void sleepMs(int ms) {
    Sleep(static_cast<DWORD>(ms));
}

} // namespace WinAPI
} // namespace vmh
