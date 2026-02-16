/**
 * @file ScancodeMapper.cpp
 * @brief Modifies the Scancode Map registry value (admin-only helper)
 *
 * GitHub: https://github.com/SevenKeyboard/scancode-mapper
 * Author: SevenKeyboard Ltd. (2026)
 * License: MIT License
 */
#include <vector>
#include <cwchar>

#include <windows.h>
#include <shellapi.h>
#include <winreg.h>

static bool IsCurrentProcessElevated();
static bool ParseHexRawToBytes(LPCWSTR hex, std::vector<BYTE>& outBytes);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    PWSTR lpCmdLine, int nCmdShow)
{
    int argc = 0;
    LPWSTR* argv = nullptr;
    LPCWSTR command = nullptr;
    LPCWSTR scancodeMapHex = L"";

    argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (!argv)
        return 0;
    if (!IsCurrentProcessElevated() || argc < 2)
    {
        MessageBoxW(
            nullptr,
            LR"(This program is not intended for general users. It is designed to modify the Scancode Map value in:
HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Keyboard Layout
using a separate administrator privilege, independent of the main program. The program will automatically terminate upon execution.)",
            L"ScancodeMapper.exe",
            MB_OK | MB_ICONERROR        
        );
        goto cleanup;
    }

    switch (argc)
    {
        case 2:
            command = argv[1];
            scancodeMapHex = L"";
            break;
        case 3:
            command = argv[1];
            scancodeMapHex = argv[2];
            break;
        default:
            goto cleanup;
    }
    if (lstrcmpW(command, L"RegWrite") == 0)
    {
        if (!scancodeMapHex || scancodeMapHex[0] == L'\0')
            goto cleanup;
        std::vector<BYTE> data;
        if (!ParseHexRawToBytes(scancodeMapHex, data))
            goto cleanup;
        HKEY hKey = nullptr;
        LSTATUS lResult = RegOpenKeyExW(
            HKEY_LOCAL_MACHINE,
            L"SYSTEM\\CurrentControlSet\\Control\\Keyboard Layout",
            0,
            KEY_SET_VALUE,
            &hKey);
        if (lResult == ERROR_SUCCESS)
        {
            RegSetValueExW(
                hKey,
                L"Scancode Map",
                0,
                REG_BINARY,
                data.data(),
                static_cast<DWORD>(data.size())
            );
            RegCloseKey(hKey);
            hKey = nullptr;
        }
    }
    else if (lstrcmpW(command, L"RegDelete") == 0)
    {
        HKEY hKey = nullptr;
        LSTATUS lResult = RegOpenKeyExW(
            HKEY_LOCAL_MACHINE,
            L"SYSTEM\\CurrentControlSet\\Control\\Keyboard Layout",
            0,
            KEY_SET_VALUE,
            &hKey);
        if (lResult == ERROR_SUCCESS)
        {
            RegDeleteValueW(hKey, L"Scancode Map");
            RegCloseKey(hKey);
            hKey = nullptr;
        }
    }
cleanup:
    if (argv)
        LocalFree(argv);
    return 0;
}

static bool IsCurrentProcessElevated()
{
    HANDLE hToken = nullptr;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
        return false;
    TOKEN_ELEVATION tokenElevation{};
    DWORD cbSize = 0;
    const BOOL bResult = GetTokenInformation(hToken, TokenElevation, &tokenElevation, sizeof(tokenElevation), &cbSize);
    CloseHandle(hToken);
    return bResult && (tokenElevation.TokenIsElevated != 0);
}

static bool ParseHexRawToBytes(LPCWSTR hex, std::vector<BYTE>& outBytes)
{
    auto hexNibbleFromChar = [](wchar_t ch, BYTE& outNibble) -> bool
    {
        if (L'0' <= ch && ch <= L'9') { outNibble = (BYTE)(ch - L'0'); return true; }
        if (L'A' <= ch && ch <= L'F') { outNibble = (BYTE)(ch - L'A' + 10); return true; }
        if (L'a' <= ch && ch <= L'f') { outNibble = (BYTE)(ch - L'a' + 10); return true; }
        return false;
    };
    outBytes.clear();
    if (!hex) return false;
    const size_t len = wcslen(hex);
    if (len == 0) return false;
    if ((len % 2) != 0) return false;
    outBytes.reserve(len / 2);
    for (size_t i = 0; i < len; i += 2)
    {
        BYTE hi = 0, lo = 0;
        if (!hexNibbleFromChar(hex[i], hi)) return false;
        if (!hexNibbleFromChar(hex[i + 1], lo)) return false;
        outBytes.push_back((BYTE)((hi << 4) | lo));
    }
    return true;
}