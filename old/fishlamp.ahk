#Requires AutoHotkey v2.0

; === Hotkey: Ctrl+Alt+T ===
^!f::
{
    ;yes my lamp is in the shape of a fish what about it
    psCommand := "kasa --alias fish_lamp toggle"

    Run('powershell.exe -NoProfile -WindowStyle Hidden -Command "' psCommand '"',, "Hide")
    
    ;MsgBox("The hotkey is working!")
}