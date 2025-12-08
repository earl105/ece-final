#SingleInstance Force

; --- CONFIG: paths to your VBS scripts ---
scripts := Map(
    "Browsers", Map(
        "Up",   "\AppVolumeControl\browsers+5.vbs",
        "Down", "\AppVolumeControl\browsers-5.vbs"
    ),
    "Meetings", Map(
        "Up",   "\AppVolumeControl\meetings+5.vbs",
        "Down", "\AppVolumeControl\meetings-5.vbs"
    ),
    "Music", Map(
        "Up",   "\AppVolumeControl\music+5.vbs",
        "Down", "\AppVolumeControl\music-5.vbs"
    )
)

global CurrentGroup := ""

; ---------------------------
;     GROUP SELECTION KEYS
; ---------------------------

F13:: {
    global CurrentGroup := "Browsers"
    TrayTip "Group Selected", "Browsers", 1
}

F14:: {
    global CurrentGroup := "Meetings"
    TrayTip "Group Selected", "Meetings", 1
}

F15:: {
    global CurrentGroup := "Music"
    TrayTip "Group Selected", "Music", 1
}

; ---------------------------
;     ACTION KEYS
; ---------------------------

F16:: { ; Volume Down
    global CurrentGroup, scripts
    if !CurrentGroup {
        TrayTip "Error", "No group selected.", 1
        return
    }
    Run scripts[CurrentGroup]["Down"]
}

F17:: { ; Volume Up
    global CurrentGroup, scripts
    if !CurrentGroup {
        TrayTip "Error", "No group selected.", 1
        return
    }
    Run scripts[CurrentGroup]["Up"]
}
