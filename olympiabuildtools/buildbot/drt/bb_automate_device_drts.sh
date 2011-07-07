#!/bin/bash

#Default configurable options
DEVICE_BOOT_TIME=100
OUTPUT=/tmp/bb_automate_device_drts.log
P4_BASE_DIR="/c/p4/device_only/dev/athens/master/build/arm"
MAX_DRT_TEST_TIME=2000
DEVICE_DRIVE="/e"
[ -z $SDCARD_PATH ] && SDCARD_PATH=/tmp/sdcard

NUM_TRY_MOUNT=10
SLEEP_BETWEEN_MOUNT_TRIES=15
CONTINUE=0
DEBUGGER_START_TIME=60 #Time to wait for debugger to start
DRT_TEST_TIME=$MAX_DRT_TEST_TIME
FOUND_DONE="FALSE"
INITIALIZE=0
INITIALIZED=1      #Assume device initilized (unless started with -init
TRY_MOUNT_REBOOT=1 #Try a reboot if mount doesn't work
reboot_device="cfp -vd USBMS -s"


function mount_sdcard
{

    if [ $INITIALIZED -le 0 ]; then
        echo "INFO: Initializing device..." | tee -a $OUTPUT
        javaws /c/bb/incr/olympiabuildtools/buildbot/bbjam/launch.jnlp -open '-script c:\bb\incr\olympiabuildtools\buildbot\bbjam\initialize.groovy'
        INITIALIZED=1

        #Wait for initialization process to finish
        countdown 200
    fi


    ls /e > /dev/null 2>&1
    sdcard_available=$?
    [ $sdcard_available -ne 0 ] && javaws /c/bb/incr/olympiabuildtools/buildbot/bbjam/launch.jnlp -open '-script c:\bb\incr\olympiabuildtools\buildbot\bbjam\click.groovy' && countdown 20

    CNT=0
    while [ $CNT -lt $NUM_TRY_MOUNT ] && [ $sdcard_available -ne 0 ]; do
        sleep $SLEEP_BETWEEN_MOUNT_TRIES
        ls /e > /dev/null 2>&1
        sdcard_available=$?
        let CNT=$CNT+1
        printf "\r   Try: $CNT/$NUM_TRY_MOUNT"
    done
    echo

    if [ $CNT -ge $NUM_TRY_MOUNT ]; then #Reboot and try one more time.
        if [ $TRY_MOUNT_REBOOT -gt 0 ]; then
            $reboot_device | tee -a $OUTPUT
            wait_for_boot

            TRY_MOUNT_REBOOT=0
            mount_sdcard
            sdcard_available=$?
        fi
    fi
    TRY_MOUNT_REBOOT=1
    return $sdcard_available
}

function wait_for_boot
{
    echo "INFO: Waiting for device to boot... (press 'C' to skip timeout)" | tee -a $OUTPUT
    CNT=$DEVICE_BOOT_TIME
    IN=""
    while [ $CNT -ge 0 ] && [ "$IN" != "C" ]; do
        printf "\r   $CNT...   "
        let CNT=$CNT-1
        read -sn 1 -t 1 IN #Read 1 char and timeout after 1 sec.
    done
    echo
}

function countdown
{
    CNT=$1
    while [ $CNT -ge 0 ]; do
        printf "\r   $CNT...   "
        let CNT=$CNT-1
        sleep 1
    done
}


function usage
{
echo "Usage: `basename $0` [OPTIONS]
Automate DRT run on blackberry device

 -c          Continue a drt run on device
             (used if device has already been initialized)

 -i          Initialize device (copy binaries from P4.  See -p)

 -b <secs>   Device boot time in seconds (default [$DEVICE_BOOT_TIME])

 -d <drive>  Drive the device is mounted on (default [$DEVICE_DRIVE])

 -p <dir>    P4 Directory where device binaries are located
             (default: [$P4_BASE_DIR])

 -m <secs>   Time in secs to allow DRTs to run (default [$MAX_DRT_TEST_TIME])

 -o <file>   Output file (output is written to this fila and STDOUT with [tee])
"
}

#===========================
#INPUT
#===========================
while getopts cib:d:p:m:o: opt  2>/dev/null; do
case $opt in
    c)  echo "INFO: Continuing Session..." | tee -a $OUTPUT;
        CONTINUE=1;;
    i) echo "INFO: Initializing and Starting DRT Tests..." | tee -a $OUTPUT;
        INITIALIZE=1;;
    b) DEVICE_BOOT_TIME=$OPTARG;;
    b) DEVICE_DRIVE="$OPTARG";;
    p) P4_BASE_DIR=$OPTARG;;
    m) MAX_DRT_TEST_TIME=$OPTARG;;
    o) OUTPUT=$OPTARG;;
    *) usage; exit 1;;
  esac
done

#===========================
#ERROR CHECKS
#===========================
if [ ! -f "$OUTPUT" ]; then
    echo "ERROR: Output file must exist [$OUTPUT]"
    exit 1
fi

if [ "`echo $DEVICE_BOOT_TIME | grep '^[0-9][0-9]*$'`" = "" ]; then
    echo "ERROR: Device boot time not integer: [$DEVICE_BOOT_TIME]" | tee $OUTPUT
    exit 1
fi

if [ ! -d "$P4_BASE_DIR" ]; then
    echo "ERROR: P4 base directory doesn't exist: [$P4_BASE_DIR]" | tee $OUTPUT
    exit 1
fi

if [ "`echo $MAX_DRT_TEST_TIME | grep '^[0-9][0-9]*$'`" = "" ]; then
    echo "ERROR: Device test time not integer: [$MAX_DRT_TEST_TIME]" | tee $OUTPUT
    exit 1
fi

if [ ! $INITIALIZE ] && [ ! $CONTINUE ];then
    echo "WARNING: Executing full DRT tests withought device SW installation." | tee $OUTPUT
fi

if [ $INITIALIZE -gt 0 ] && [  $CONTINUE -gt 0 ];then
    echo "ERROR: Cannot use -i and -c simultaneously." | tee $OUTPUT
    exit 1;
fi

if [ ! -f $SDCARD_PATH/index.drt ] && [ $CONTINUE -le 0 ]; then
    echo "ERROR: No index.drt found in \$SDCARD_PATH: [$SDCARD_PATH]"
    exit 1;
fi

#===========================
#INFO
#===========================
echo "`basename $0` : Automate DRT run on blackberry device
#===========================

This tool will run all DRT tests found in the [\$SDCARD_PATH/index.drt] on the device physically connected to the pc.

Multiple devices support is not yet implemented.  (current limitation of automated bbjam)
Tests will be interrupted every [$MAX_DRT_TEST_TIME] seconds to check for crashed or hanging tests and then continued again.

Please do not disconnect the device or mount the sd card during the tests....

OUTPUT FILE:        [$OUTPUT]
MAX_DRT_TEST_TIME:  [$MAX_DRT_TEST_TIME]
DEVICE_BOOT_TIME:   [$DEVICE_BOOT_TIME]
P4_BASE_DIR:        [$P4_BASE_DIR]
DEVICE_DRIVE:       [$DEVICE_DRIVE]
#===========================
" | tee $OUTPUT

#Start of tests...
date | tee -a $OUTPUT

#New sessions will start off with a copy of $SDCARD_PATH/index.drt
if [ $CONTINUE -le 0 ]; then

    #Warn about removing data
    if [ $INITIALIZE -gt 0 ]; then
        echo "WARNING !!!: About to wipe device!!!" | tee -a $OUTPUT
    fi
    echo "WARNING !!!: All results will be overwitten. (CTRL-C to abort)" | tee -a $OUTPUT
    countdown 30


    #Mount sd and copy index over.
    echo "INFO: Mounting SD for initial index.drt setup..." | tee -a $OUTPUT
    mount_sdcard
    if [ $? -ne 0 ];then
        echo "ERROR: [Unable to mount sdcard]" | tee -a $OUTPUT
        exit 1
    fi

    echo "INFO: Removing results from device..." | tee -a $OUTPUT
    cp $SDCARD_PATH/index.drt "$DEVICE_DRIVE/"
    rm "$DEVICE_DRIVE/done" 2> /dev/null
    rm "$DEVICE_DRIVE/Skipped" 2> /dev/null
    rm `find "$DEVICE_DRIVE/results" -type f` 2> /dev/null
fi

if [ $INITIALIZE -ge 1 ]; then
    INITIALIZED=0   #Need to initialize device (accept license agreement)
    echo "INFO: Initializing Device with Newest P4 revisions..." | tee -a $OUTPUT
    export P4CLIENT=device_only
    p4 sync \#head

    cfp -u1 wipe | tee -a $OUTPUT
    cfp -u1 load $P4_BASE_DIR/rim* | tee -a $OUTPU
    cfp -u1 load $P4_BASE_DIR/OlympiaWebkit.sfi | tee -a $OUTPUT
    cfp -u1 load $P4_BASE_DIR/FSLinked.dmp | tee -a $OUTPUT

    echo "INFO: Device SW installed.  Please wait for the process to complete..." | tee -a $OUTPUT
    countdown 60;
fi

while [ "$FOUND_DONE" != "TRUE" ]; do
    #Reboot and sleep DRT_TEST_TIME
    $reboot_device | tee -a $OUTPUT
    wait_for_boot

    echo "INFO: Waiting for tests to complete... (press 'C' to skip timeout)" | tee -a $OUTPUT
    CNT=$DRT_TEST_TIME;
    IN=""
    while [ $CNT -ge 0 ] && [ "$IN" != "C" ]; do
        printf "\r   $CNT...   \r"
        let CNT=$CNT-1
        read -sn 1 -t 1 IN #Read 1 char and timeout after 1 sec.
    done
    echo "INFO: Tests hanging or finished. (waited ${CNT}sec)" | tee -a $OUTPUT

    echo "INFO: Killing Debugger..." | tee -a $OUTPUT
    $OLYMPIAJAVA/Lynx/bin/JavaLoader.exe erase -f OlympiaDebugger.cod 2>&1
    wait_for_boot


    #Mount sd
    echo "INFO: Mounting SDCard..." | tee -a $OUTPUT
    mount_sdcard
    if [ $? -ne 0 ];then
        echo "ERROR: [Unable to mount sdcard]" | tee -a $OUTPUT
        exit 1
    fi

    #Check for done
    if [ -e "$DEVICE_DRIVE/done" ]; then
        echo "INFO: Done file found.  DRT TESTS FINISHED!!!" | tee -a $OUTPUT
        date | tee -a $OUTPUT
        FOUND_DONE="TRUE"

        #Reload the debugger
        echo "INFO: Restarting Debugger..." | tee -a $OUTPUT
        $OLYMPIAJAVA/Lynx/bin/JavaLoader.exe load $OLYMPIAJAVA/Lynx/Debug/OlympiaDebugger.cod

        exit 0
    fi

    #Remove all tests up-to and including test in current.drt
    current=`cat "$DEVICE_DRIVE/current.drt"`
    echo "$current" >> "$DEVICE_DRIVE/Skipped"
    echo "INFO: Last executed test: [$current]" | tee -a $OUTPUT

    perl -n -i.bak -e "chomp; print \"\$_\\n\" if \$PRINT; \$PRINT = 1 if \"\$_\" eq \"$current\";" "$DEVICE_DRIVE/index.drt"
    echo "INFO: Refactored index.drt.  Number of tests remaining: [`cat "$DEVICE_DRIVE/index.drt" | wc -l`]" | tee -a $OUTPUT
    echo "INFO: Number of results: [`find $DEVICE_DRIVE/results -type f | wc -l`]" | tee -a $OUTPUT

    #Recalculate sleep time
    num_test=`cat /tmp/sdcard/index.drt | wc -l | sed 's/ //g'`
    let DRT_TEST_TIME=$num_test
    [ $DRT_TEST_TIME -gt $MAX_DRT_TEST_TIME ] && DRT_TEST_TIME=$MAX_DRT_TEST_TIME

    #Reload the debugger
    echo "INFO: Restarting Debugger..." | tee -a $OUTPUT
    $OLYMPIAJAVA/Lynx/bin/JavaLoader.exe load $OLYMPIAJAVA/Lynx/Debug/OlympiaDebugger.cod

done

