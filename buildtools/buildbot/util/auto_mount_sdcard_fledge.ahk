;
; AutoHotKey Script to mount the sdcard in fledge
;

;Fledge window name
FledgeWindow := "BlackBerry Talladega Simulator"
SetTitleMatchMode 1

;This script's name (used in usage)
ScriptName := "auto_mount_sdcard_fledge.ahk"

;Maximum number of seconds to wait for fledge window to appear
TIMEOUT = 100

;Usage is $0 <path-to-sdcard>
if 0 < 1  
{
    MsgBox Usage:`n%ScriptName% <path-to-sdcard>`n`nPlease provide the path you want to have mounted in fledge.
    exit 1
}

;Param 1 is the path to the sdcard.
SDCARD_PATH = %1%
;MsgBox,,, INFO: SDCARD: [%SDCARD_PATH%],3
;Ensure path exists.
ifNotExist, %SDCARD_PATH%
{
    MsgBox,16,,Error!  Given Path does not exist: [%SDCARD_PATH%].`nThe program will be terminated.,5
    exit 1
}

Loop, %TIMEOUT%
{
    ;Wait for the fledge window
    IfWinExist, %FledgeWindow%
    {
        ;Close leftover windows  
        WinClose, SD Card
        WinClose, Browse For Folder

        ;Select the [Change SDCard...] from the Simulate menu item
        WinActivate, %FledgeWindow%
        Send !S
        Loop 10
            Send {DOWN}
        Send {ENTER}

        ;Open the directory to select the path
        WinWait, SD Card, , 30
        if ErrorLevel
        {
            ;Hm. we've selected the wrong thing I guess.
            ;just exit error
            exit 1
        }
        WinActivate
        ControlClick, Add Directory...

        ;Wait for directory browser window
        WinWait, Browse For Folder, , 30
        if ErrorLevel
        {
            ;Our expected window didn't pop-up. (???)
            WinClose Add SD Card Image - Fledge
            WinClose SD Card
            MsgBox,16,,Error!  Expecting [Browse For Folder] window.`nThe program will be terminated.,5
            exit 1
        }

        ;Enter the SDCARD_PATH
        WinActivate, Browse For Folder, , 30
        ControlClick, Edit1

        ;remove any present paths and replace with ours.
        sleep 1000
        Send {HOME}+{END}{DEL}
        Send %SDCARD_PATH%{ENTER}
        sleep 3000

        ;Close the SD Card window
        WinActivate, SD Card
        setKeyDelay, 100
        Send {TAB}{TAB}{ENTER}
        sleep 3000


        ;If the [SD Card] Window is still available, something went wrong.
        ifWinExist, SD Card
        {
            MsgBox,16,,Error!  [SD Card] Window still exists.`nSomething went wrong with the automation.`nThis program will be terminated.,5
            WinClose, SD Card
            exit 1
        }
        
        ;The directory is now mounted - our job is done.
        exit 0

    }else{
        sleep 1000
    }
}
exit 0
