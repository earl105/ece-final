#Requires AutoHotkey v2.0

; === Hotkey: Ctrl+Alt+T ===
^!t::
{
    
    psCommand := "kasa --alias tall_lamp toggle"

    Run('powershell.exe -NoProfile -WindowStyle Hidden -Command "' psCommand '"',, "Hide")
    
    ;MsgBox("The hotkey is working!")
}