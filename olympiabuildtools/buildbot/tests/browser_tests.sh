#!/bin/bash
#==============================
#Execute a set of browser javascript 
#rendering tests via bbjam automation tool
#(http://go/bbjam)
#==============================

#=====================
# START FUNCTIONS
#=====================
function usage
{
    set_all_tests
    echo "Usage: `basename $0` [results_path]
OPTIONS:
    -f <file1,file2,filen>  #Flash the device with the given files
    -t <test1,test2,testn>  #Run the specified bbjam groovy test files
    -i                      #Initialize the device
    -q                      #Quiet
    -h                      #This help screen

Current available tests:
    $ALL_TESTS
"
}
function set_all_tests
{
    ALL_TESTS=""
    for test in `ls $BBJAM_DIR | grep browser_test | sed 's/.*_//' | sed 's/\.groovy$//'`; do
        ALL_TESTS="$ALL_TESTS $test"
    done
}
function wait_for_boot
{
    echo "INFO: Waiting for device to boot..."
    countdown $DEVICE_BOOT_TIME
}
function countdown
{
    CNT=$1
    while [ $CNT -ge 0 ]; do
        [ ! $QUIET ] && printf "\r   $CNT...   "
        let CNT=$CNT-1
        sleep 1
    done
    echo
}
function set_defaults
{
    #CONSTANTS:
    DEVICE_BOOT_TIME=100
    BBJAM_DIR="/c/olympia/olympiabuildtools/buildbot/bbjam"
    BBJAM_WDIR='c:\olympia\olympiabuildtools\buildbot\bbjam'
    BBJAM_EXECUTION_TIME=180   #Plenty of time to finish tests
    javaloader="$OLYMPIAJAVA/Lynx/bin/JavaLoader.exe"
    reboot_device="cfp -vd USBMS -s"
    
    #DEFAULTS:
    set_all_tests
    TESTS="$ALL_TESTS"
    NUM_FILES_FLASHED=0
    INITIALIZE=0

}
#=====================
# END FUNCTIONS
#=====================

set_defaults

#=====================
# PARSE INPUT
#=====================
while getopts ihf:t: opt  2>/dev/null; do
case $opt in
    f) FLASH_FILES=`echo $OPTARG |sed 's/,/ /g'`;;
    t) TESTS=`echo $OPTARG |sed 's/,/ /g'`;;
    i) INITIALIZE=1;;
    q) QUIET=1;;
    *) usage; exit 1;;
esac
done
OUTPUT_PATH=${!#}


#=====================
#ERROR CHECKS
#=====================
if [ ! -d "$OUTPUT_PATH" ]; then
    echo "ERROR: Given Path does not exist [$OUTPUT_PATH]" >&2
    usage && exit 1
fi

#=====================
#FLash Files
#=====================
if [ "$FLASH_FILES" ]; then
    for file in $FLASH_FILES; do
        if [ -f $file ]; then
            let NUM_FILES_FLASHED+=1
            echo "Flashing device with [$file]..."
            cfp load $file
        else
            echo "ERROR: Flash file does not exist: [$file]" >&2
            exit 1
        fi
    done

    #Turn radio on
    countdown $BBJAM_EXECUTION_TIME
    echo "Enabeling Radio..."
    $javaloader radio on

fi

#=====================
#INFORMATION
#=====================
[ ! $QUIET ] && echo "
================
INFO
================
OUTPUT_PATH        [$OUTPUT_PATH]
FLASH FILES:       [$FLASH_FILES]
TESTS:             [$TESTS]
INITIALIZE_DEVICE: [$INITIALIZE]
"


#=====================
#INITIALIZE DEVICE
#=====================
if [ $NUM_FILES_FLASHED -le 0 ]; then
    echo "Rebooting device for a clean start..."
    $reboot_device >/dev/null 2>&1 && wait_for_boot
fi
if [ $INITIALIZE -gt 0 ]; then
    echo "Initializing device..."
    javaws $BBJAM_DIR/launch.jnlp -open '-script c:\olympia\olympiabuildtools\buildbot\bbjam\initialize.groovy'
    countdown $BBJAM_EXECUTION_TIME
fi


echo "Launching Browser..."
#Launch the Browser
javaws $BBJAM_DIR/launch.jnlp -open '-script c:\olympia\olympiabuildtools\buildbot\bbjam\launch_browser.groovy'

#Wait for bbjam to finish
countdown $BBJAM_EXECUTION_TIME

#=====================
#TESTS
#=====================
for test in $TESTS; do
    echo "Starting test: [$test]...";
    test_file=`ls $BBJAM_DIR | grep browser_test | grep $test`
    if [ ! $test_file ] || [ ! -f "$BBJAM_DIR/$test_file" ]; then
        echo "ERROR: Test file not found for test [$test] ($test_file)" >&2
        exit 1
    fi


    #BAH !  I can't run javaws like this:
    #javaws $BBJAM_DIR/launch.jnlp -open '-script $test_path'
    #FIXME - Please God FIXME !!!
    case $test in
    acid2)
        javaws $BBJAM_DIR/launch.jnlp -open '-script c:\olympia\olympiabuildtools\buildbot\bbjam\browser_test_acid2.groovy'
        ;;

    acid3)
        javaws $BBJAM_DIR/launch.jnlp -open '-script c:\olympia\olympiabuildtools\buildbot\bbjam\browser_test_acid3.groovy'
        ;;

    sunspider)
        javaws $BBJAM_DIR/launch.jnlp -open '-script c:\olympia\olympiabuildtools\buildbot\bbjam\browser_test_sunspider.groovy'
        #Sunspider tests take a long time
        countdown 600
        ;;
    *) 
        echo "ERROR: Unknown test [$test]." >&2
        exit 1
        ;;
    esac

    countdown $BBJAM_EXECUTION_TIME
    [ ! $QUIET ] && echo "Taking snapshot of device..."
    $javaloader -u screenshot $OUTPUT_PATH/$test.bmp
    
done
exit

#Acid2 test
echo "Starting Acid2..."
javaws $BBJAM_DIR/launch.jnlp -open '-script c:\olympia\olympiabuildtools\buildbot\bbjam\browser_test_acid2.groovy'
countdown $BBJAM_EXECUTION_TIME
$javaloader -u screenshot /c/temp/acid2.bmp


#Acid3 test
echo "Starting Acid3..."
javaws $BBJAM_DIR/launch.jnlp -open '-script c:\olympia\olympiabuildtools\buildbot\bbjam\browser_test_acid3.groovy'
countdown $BBJAM_EXECUTION_TIME
$javaloader -u screenshot /c/temp/acid3.bmp


#SunSpider test
echo "Starting SunSpider..."
javaws $BBJAM_DIR/launch.jnlp -open '-script c:\olympia\olympiabuildtools\buildbot\bbjam\browser_test_sunspider.groovy'
countdown $BBJAM_EXECUTION_TIME
echo "Waiting for Sunspider test to finish..."
countdown 300
$javaloader -u screenshot /c/temp/acid3.bmp

