;
; AutoHotKey Script to loop and kill BlackBerry/Fledge windows
;

;List of windows titles to kill
ListofWindows=Talladega|Fledge|BlackBerry|C++ Runtime|C++ Debug|wxWidgets Debug Alert
FledgeWindow := "BlackBerry Talladega Simulator"

SetTitleMatchMode 2
Loop
{
    Loop,parse,ListofWindows,|
    {
        WinWait, %A_LoopField%, ,10
        if ! ErrorLevel
        {
            WinGetTitle, Title

            ; if this is the shared memory error, select and exit
            IfInString, Title, Shared Memory
            {
                WinActivate %A_LoopField%
                Send !K
            }
            ; Don't kill the simulator otherwise close the window
            if Title != %FledgeWindow%
            {
                WinClose %A_LoopField%
                WinActivate %A_LoopField%
                if ! ErrorLevel
                    Send {ENTER}
            }
        }
    }
}

