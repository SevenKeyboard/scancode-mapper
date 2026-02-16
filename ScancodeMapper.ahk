
#Requires AutoHotkey v2.0
#NoTrayIcon
#SingleInstance Off
;=============================================================
; ScancodeMapper — Modifies the Scancode Map registry value (admin-only helper)
;
; GitHub: https://github.com/SevenKeyboard/scancode-mapper
; Author: SevenKeyboard Ltd. (2026)
; License: MIT License
;=============================================================
if (!A_IsAdmin || !A_Args.Length)    {
    msgbox("
(LTrim0 RTrim0
This program is not intended for general users. It is designed to modify the Scancode Map value in:
HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Keyboard Layout
using a separate administrator privilege, independent of the main program. The program will automatically terminate upon execution.
)",, "Iconx")
    ExitApp
}
switch (A_Args.Length)
{
    case 1:     regFunc := A_Args[1], regValue := ""
    case 2:     regFunc := A_Args[1], regValue := A_Args[2]
    default:    ExitApp
}
regKeyName := "HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Keyboard Layout", regValueName := "Scancode Map"
switch regFunc, false
{
    case "RegWrite":
        if (regValue !== "")
            regWrite(regValue, "REG_BINARY", regKeyName, regValueName)
    case "RegDelete":
        regDelete(regKeyName, regValueName)
}