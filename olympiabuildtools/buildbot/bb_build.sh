#!/bin/bash
#=======================
#BuildBot Slave Build Script for the following buid types:
# Incremental    - Get changes from olympia and run ob-webkit and drt
# Full        - CleanAll, get all changes, rebuild fledge and talladega
#
#Usage:
# (see usage function)
#=======================

function usage
{
    echo "Usage: $0 -a <branch> -t <incremental|full|try|drt> -b <build number> -r <revision>"
}

NIGHTLY=""
REVISION=""
while getopts a:t:b:d:n:r:? opt  2> /dev/null; do
case $opt in
    a) BRANCH=$OPTARG ;;
    b) BUILDNUMBER=$OPTARG ;;
    d) DEVICE=$OPTARG ;;
    n) NIGHTLY=$OPTARG ;;
    r) REVISION=$OPTARG ;;
    t) TYPE=$OPTARG ;;

  esac
done

if [ "$DEVICE" = "warnings" ]; then
    grep -E "line [0-9]*: *Warning:" /tmp/buildslave_stdio | grep -v Lynx | sed -e 's/\\/\//g' -e 's/^.*\(.\{10\}\/.*\"\), line/\1, line/'| sort | uniq -w 25 -c | sort -r
    exit 0
fi

#Default to device
[ -z "$DEVICE" ] && DEVICE="talladega"

#Ensure known build type
if [ -z $TYPE ] || ( [ "$TYPE" != "full" ] && [ "$TYPE" != "incremental" ] && [ "$TYPE" != "try" ] && [ "$TYPE" != "drt" ] ); then
    echo "Error: Unknown Build Type: [$TYPE]"
    usage
    exit 1
fi

#Ensure build number or revision
if [ -z $REVISION ] && [ -z $BUILDNUMBER ]; then
    echo "Error: Either revision or build number must be supplied."
    usage
    exit 1
fi
REVISION=`echo $REVISION | sed 's/^rev://'`

#Path to put DRT results
DRT_RESULTS_BASE="/c/Temp/layout-test-results"
DRT_RESULTS_DIR=$DRT_RESULTS_BASE/$REVISION
REMOTE_DRT_RESULTS_BASE="DRT_RESULTS"    #This is the directory structure on the server side
REMOTE_DRT_RESULTS_DIR=""
[ -z $REVISION ] && DRT_RESULTS_DIR=$DRT_RESULTS_BASE/$BUILDNUMBER
if [ ! -z $NIGHTLY ] && [ "$NIGHTLY" = "$COMPUTERNAME" ]; then
    DRT_RESULTS_DIR="$DRT_RESULTS_BASE/nightly/`date +%Y%m%d`"
    REMOTE_DRT_RESULTS_DIR="nightly"
fi




#===========================
#START OF FUNCTIONS
#===========================

#DRT expected results are extracted into the platoform/olympia/ directory
#as a base for the DRT comparisons
#Also, known problematic tests are added to the Skipped file.
function setup_drt
{
    DRT_TESTS=""
    mkdir -p $1 2> /dev/null
    for subtest in `cat $OLYMPIAROOT/olympiabuildtools/buildbot/drt/subtests.txt`; do
        DRT_TESTS="$DRT_TESTS $subtest"
    done
    bb_print "DRT SUBTESTS: [$DRT_TESTS]"
}

#Get the latest log info from each olympia repository
#(this is used for p4 commits)
function set_latest_shas
{
    esc_OLYMPIAROOT=`echo $OLYMPIAROOT | sed 's/\\//\\\\\\//g'`
    LATEST_SHAS=""

    for dir in `git ob dirs`; do
        repo=`echo $dir | sed "s/$esc_OLYMPIAROOT\\///"`
        pushd $dir > /dev/null
            sha=`git log --pretty=oneline -1 | sed 's/ .*//'`;
        popd > /dev/null

        LATEST_SHAS="$LATEST_SHAS
$repo:    $sha";
    done

}

#Reset, fetch and rebase the olympia repo
#Also checkout the specific revision (if given)
function setup_repo
{
    repo_dir=$1
    repo_branch=$2
    repo_revision=$3

    pushd $OLYMPIAROOT/$repo_dir

        bb_print "EXECUTE: [checkout $repo_branch]"
        git checkout $repo_branch;

        bb_print "EXECUTE: [git fetch in ($repo_dir)]"
        (try_x_times 3 git fetch) || cleanup_and_exit $?

        bb_print "EXECUTE: [git reset --hard]"
        git reset --hard origin/$repo_branch || cleanup_and_exit $?

        if ! [ -z $repo_revision ]; then
            bb_print "EXECUTE: [checkout $repo_revision]"
            git checkout $repo_revision --quiet
        fi
    popd
}

#Step 2 of the build operation - clean and rebuild ob-bbnsl
function clean_and_rebuild_bbnsl
{
    bb_print "EXECUTE: [ob-bbnsl-clean]"
    time ob-bbnsl-clean > /dev/null

    bb_print "EXECUTE: [ob-player-clean]"
    time ob-player-clean > /dev/null

    bb_print "EXECUTE: [ob-webkit-clean]"
    time ob-webkit-clean > /dev/null

    bb_print "EXECUTE: [ob-bbnsl]"
    check_for_stop_the_build
    time ob-bbnsl 2>&1 | tee -a /tmp/buildslave_stdio
    return ${PIPESTATUS[0]}
}

#Wait for the number of fledge processes to come down to the specified number
#Default: 0
function wait_for_fledge_drt
{
    #Wait for the fledge processes to finish (Max 600sec)
    timewaited=0
    SLEEPTIME=10
    [ -z $MAXDRTWAIT ] && MAXDRTWAIT=2000
    MAXNUMFLEDGEPROCS=0
    [ ! -z "$1" ] && MAXNUMFLEDGEPROCS=$1


    num_fledge_pids=`jobs | grep -v grep | grep -v Done | wc -l`

    while [ $num_fledge_pids -gt $MAXNUMFLEDGEPROCS ] && [ $timewaited -lt $MAXDRTWAIT ]; do
        num_fledge_pids=`jobs | grep -v grep | grep -v Done | wc -l`
        if [ $num_fledge_pids -gt $MAXNUMFLEDGEPROCS ];then
            sleep $SLEEPTIME;
            let timewaited+=$SLEEPTIME
            if [ $(($timewaited % 100)) = 0 ]; then
                bb_print "INFO: [Waited [$timewaited] sec. for fledge DRTs...]"
            fi
        else
            #Fledge may just be restarting - wait a bit and check again.
            sleep 60  #Yes, 60secs is necessary
            num_fledge_pids=`jobs | grep -v grep | grep -v Done | wc -l`
        fi
    done

    #Try to kill fledge if MAXDRTWAIT exceeded.
    if [ $timewaited -ge $MAXDRTWAIT ]; then
        #Only warn if MAXDRTWAIT != 0.  if 0, this was called explicitly
        [ $MAXDRTWAIT -ne 0 ] &&
            bb_print "WARNING: [Wait-for-fledge time exceeded. ($timewaited seconds).  Trying to manually stop fledge...]"
        #Create the done file so that DRTs stop.
        for test in $DRT_TESTS; do
            touch $SDCARD_PATH.$test/done
        done
        echo -e "Waiting for DRT... "
        sleep 30 #Let the drt recognize the done file
        echo "Done."

        #All tests should be done now. - loop and kill jobs
        for pid in `jobs -l | grep -v grep | grep -v Done | awk '{print $2}'`; do
        if [ "`echo $pid | grep ^[1-9][0-9]*`" != "" ]; then
            bb_print "WARNING: [Trying to kill [$pid]...]"
                kill -9 $pid
        fi
        done

        #Now that we've killed all jobs, kill all fledge procs...
        CNT=0
        num_fledge_pids=`ps | grep fledge | wc -l`
        if [ $num_fledge_pids -eq 0 ];then
            echo -e "Waiting for DRT... "
            sleep 60  #Yes, 60secs is necessary
            echo "Done."
            num_fledge_pids=`ps | grep fledge | wc -l`
        fi
        while [ $CNT -lt 10 ] && [ $num_fledge_pids -gt 0 ];do
            for pid in `ps | grep fledge | awk '{print $1}'`; do
                bb_print "WARNING: [Trying to kill [$pid]...]"
                kill -9 $pid
            done
            let CNT=$CNT+1
            num_fledge_pids=`ps | grep fledge | awk '{print $1}' | wc -l`

            #Wait for some time to see if any fledge procs start up.
            if [ $num_fledge_pids -eq 0 ];then
                echo -e "Waiting for DRT... "
                sleep 60  #Yes, 60secs is necessary
                echo "Done."
                num_fledge_pids=`ps | grep fledge | awk '{print $1}' | wc -l`
            fi
            sleep 1
        done
        bb_print "INFO: [All fledge processes have been terminated...]"
    fi
    num_fledge_pids=`ps | grep fledge | awk '{print $1}' | wc -l`
    return $num_fledge_pids
}

#Execute DRT tests and copy results (if any) to the $ATHENS_SERVER
function execute_and_archive_drt_results
{
    #FIXME: I'm changing to just run one ob-run-drt with all tests.
    #I'm running it in the foreground and hope the DRTs will print out their prigress.
    #(If debugger doesn't start, nothing will be printed and this buildbot run will fail)
    #(This is really bad for full builds so I'm moving the DRT stuff to the end)

    #Set the tests to run from subtests.txt file
    bb_print "EXECUTE: [setup_drt]"
    setup_drt $OLYMPIAROOT/olympia/LayoutTests/platform/olympia
    check_for_stop_the_build


    #Patch the DRT script to support automated manual mounting of the sdcard.
    bb_print "INFO: [Patching DRT script to start fledge without sd-card mounted...]"
    auto_mount_script="/c/olympia/olympiabuildtools/buildbot/util/auto_mount_sdcard_fledge.ahk"
    patch="84,87d83
<                 print \"INFO: Starting Fledge Automation...\n\";
<                 system(\"AutoHotkey.exe $auto_mount_script 'c:\\\\Temp\\\\sdcard' &\");
< 
< 
89c85
<                 exec \"\$FLEDGE_PATH/fledge.exe\", qw(-app-param=NoFileIndexing -app=JvmFledgeSimulator.dll -handheld=talladega -session=talladega -app-param=DisableRegistration -app-param=JvmAlxConfigFile:talladega.xml -JvmDisableBacklightTimeout -data-port=0x4d44 -data-port=0x4d4e -pin=0x2100000A);
---
>                 exec \"\$FLEDGE_PATH/fledge.exe\", \"-fs-sdcard\", \"-fs-sdcard-root=\$SDCARD_DIR\", qw(-start-hidden -app-param=NoFileIndexing -app=JvmFledgeSimulator.dll -handheld=talladega -session=talladega -app-param=DisableRegistration -app-param=JvmAlxConfigFile:talladega.xml -JvmDisableBacklightTimeout -data-port=0x4d44 -data-port=0x4d4e -pin=0x2100000A);"

    pushd $OLYMPIAROOT/olympia
        #Make sure it's not already patched and patch it.
        git checkout WebKitTools/DumpRenderTree/olympia/DumpRenderTree
        echo "$patch" | patch -R WebKitTools/DumpRenderTree/olympia/DumpRenderTree
    popd


    #Setup DRT environment
    export SDCARD_PATH=/c/Temp/sdcard;
    mkdir $SDCARD_PATH
    rm -rf $SDCARD_PATH/*.drt $SDCARD_PATH/results $SDCARD_PATH/done;
    mkdir -p $DRT_RESULTS_DIR > /dev/null 2>&1

    #bb_print "INFO: [Start Mount SDCard automation...]"
    #AutoHotkey.exe $OLYMPIAROOT/olympiabuildtools/buildbot/util/auto_mount_sdcard_fledge.ahk 'c:\Temp\sdcard' &
    #AutoHotkey.exe /c/olympia/olympiabuildtools/buildbot/util/auto_mount_sdcard_fledge.ahk 'c:\Temp\sdcard' &
    #sleep 3

    bb_print "INFO: [Start DRT tests. ($DRT_TESTS)]"
    ob-run-drt -o $DRT_RESULTS_DIR $DRT_TESTS 2>&1 | tee $DRT_RESULTS_DIR/output.txt

    #=================
    #Setup to copy DRT Results to $ATHENS_SERVER
    #=================
    check_for_stop_the_build
    DRT_RESULTS=0
    if [ -f "$DRT_RESULTS_DIR/results.html" ]; then
        let DRT_RESULTS+=1
        #Fix paths in DRT files
        perl -p -i.bak -e "s~$OLYMPIAROOT/olympia~..~g" $DRT_RESULTS_DIR/results.html 2> /dev/null
        perl -p -i.bak -e "s~../LayoutTests~../../LayoutTests~g" $DRT_RESULTS_DIR/results.html 2> /dev/null
    fi
    #Show the last 4 lines of the output.txt to show the results
    if [ -f "$DRT_RESULTS_DIR/output.txt" ] && [ $DRT_RESULT -le 0 ]; then
        bb_print "INFO: [Successful DRT run ($DRT_TESTS):]"
        tail -4 $DRT_RESULTS_DIR/output.txt
        rm -rf $DRT_RESULTS_DIR
    fi

#FIXME: Uncomment once parallel fledge is working....
#    FLEDGE_PIDS=0
#
#    #I'm not starting if there are already fledge procs...
#    MAXDRTWAIT=0 && wait_for_fledge_drt 0
#    MAXDRTWAIT=''
#
#    #This copies all expected DRT results (without overwriting)
#    bb_print "EXECUTE: [setup_drt]"
#    setup_drt $OLYMPIAROOT/olympia/LayoutTests/platform/olympia
#    check_for_stop_the_build
#
#    #Run each sub-test in its own fledge process
#    #NOTE: Only allow 2 fledge sub-tests to run simulatneously.
#    #FIXME:  Parallel fledge is broken due to indexing
#    #MAX_FLEDGE_PROCESSES=$NUMBER_OF_PROCESSORS
#    MAX_FLEDGE_PROCESSES=1
#    [ -z MAX_FLEDGE_PROCESSES ] && MAX_FLEDGE_PROCESSES=1
#
#    TOTAL_NUM_TESTS=0
#    for test in $DRT_TESTS; do
#        let TOTAL_NUM_TESTS+=1
#    done
#
#    TEST_CNT=0
#    for test in $DRT_TESTS; do
#        check_for_stop_the_build
#        let TEST_CNT+=1
#        bb_print "INFO: [Start DRT subtest. ($test)]"
#        export SDCARD_PATH=/tmp/sdcard.$test;
#        rm -rf $SDCARD_PATH/*.drt $SDCARD_PATH/results $SDCARD_PATH/done;
#        mkdir -p $DRT_RESULTS_DIR/$test > /dev/null 2>&1
#        ob-run-drt -o $DRT_RESULTS_DIR/$test $test 2>&1 | tee $DRT_RESULTS_DIR/$test/output.txt
#        echo -e "Waiting for DRT... "
#        sleep 30 #Sleep to let the fledge process start
#
#        #Wait for fledge to finish
#        if [ $TEST_CNT -ge $MAX_FLEDGE_PROCESSES ]; then
#            #If there are more tests, tell wait_for_fledge_drt to return
#            #once any of the tests are finished.
#            let max_number_fledge_processes=$MAX_FLEDGE_PROCESSES-1
#            wait_for_fledge_drt "$max_number_fledge_processes"
#            TEST_CNT=$?
#        fi
#    done

#   Wait for all fledge procs to finish
#    wait_for_fledge_drt 0

#    #Allow time for all DRT results to be written
#    echo -e "Waiting for DRT... "
#    sleep 30;
#
#    #=================
#    #Copy DRT Results to $ATHENS_SERVER
#    #=================
#    DRT_RESULTS=0
#    #Initialize common results file
#    for test in $DRT_TESTS; do
#        check_for_stop_the_build
#        if [ -f "$DRT_RESULTS_DIR/$test/results.html" ]; then
#            let DRT_RESULTS+=1
#            #Fix paths in DRT files
#            perl -p -i.bak -e "s~$OLYMPIAROOT/olympia~..~g" $DRT_RESULTS_DIR/$test/results.html 2> /dev/null
#            perl -p -i.bak -e "s~../LayoutTests~../../LayoutTests~g" $DRT_RESULTS_DIR/$test/results.html 2> /dev/null
#        else
#            #If everything passed, only the output.txt will reside in $DRT_RESULTS_DIR/$test/.
#            #That's fine - just that if other tests failed, this dir will be copied
#            #to the athens drt results server which may be confusing for the developer.
#            #
#            #Solution:  Show the last 4 lines of the output.txt proving its success and remove
#            #the directory.
#            if [ -d "$DRT_RESULTS_DIR/$test" ]; then
#                bb_print "DEBUG: [DRT Test direcroty exists but no results.html.]"
#                bb_print "INFO: [Successful DRT test ($test):]"
#set -x
#                tail -4 $DRT_RESULTS_DIR/$test/output.txt
#                ls $DRT_RESULTS_DIR/$test
#                rm -rf $DRT_RESULTS_DIR/$test
#set +x
#            fi
#            bb_print "DEBUG: [No $test dir ($DRT_RESULTS_DIR/$test)]"
#        fi
#    done

    #Copy DRT Results to $ATHENS_SERVER
    if [ $DRT_RESULTS -gt 0 ]; then
        bb_print "INFO: [Copying DRT Results from $DRT_RESULTS_DIR/ ...]"
set -x
        pushd $DRT_RESULTS_BASE
            #Create the remote directory structure and scp it over.
            mkdir -p $REMOTE_DRT_RESULTS_BASE/$REMOTE_DRT_RESULTS_DIR 2> /dev/null
            #Not moving because a stay process may have the directory locked
            cp -r $DRT_RESULTS_DIR $REMOTE_DRT_RESULTS_BASE/$REMOTE_DRT_RESULTS_DIR/
            tar -cvpf - $REMOTE_DRT_RESULTS_BASE | ssh buildbot@$ATHENS_SERVER "tar -xvpf -"
            rm -rf  $REMOTE_DRT_RESULTS_BASE
        popd
set +x
    else
        bb_print "WARNING: [No DRT Results to copy from ($DRT_RESULTS_DIR).]"
    fi

    return 0
}

#Copy binaries to p4 repository for nightly builds
function copy_binaries_to_p4
{
    bb_print "EXECUTE: [Nightly to P4...]"
    P4DATE=`date +%Y%m%d`
    P4BUILD_DIR=/c/P4/dev/athens/master/build
    mkdir -p $P4BUILD_DIR/arm 2> /dev/null
    mkdir -p $P4BUILD_DIR/fledge 2> /dev/null

    #Prep the files for modification
    p4 edit $P4BUILD_DIR/arm/*
    p4 edit $P4BUILD_DIR/fledge/*

    #Copy binaries to p4...
    cp -f $BBNSLDIR/players/build/xscale-release/lib/*          $P4BUILD_DIR/arm/
    cp -f $OLYMPIAJAVA/lynx/jvm/arm/ReleaseARM/rim0x05001807.sfi $P4BUILD_DIR/arm/
    cp -f $OLYMPIAJAVA/javaapplications/FSLinked.dmp            $P4BUILD_DIR/arm/

    cp -f $BBNSLDIR/players/build/win32-debug/lib/*             $P4BUILD_DIR/fledge/
    cp -f $OLYMPIAJAVA/Lynx/bin/JavaLoader.exe                  $P4BUILD_DIR/fledge/
    cp -f $OLYMPIAJAVA/Lynx/Debug/OlympiaDebugger.cod           $P4BUILD_DIR/fledge/

    #Set the LATEST_SHAS variable to include in the p4 commit message
    set_latest_shas

    #Add them to p4
    p4 add $P4BUILD_DIR/*/* > /dev/null 2>&1
    p4 submit -d "[$P4DATE] Nightly Olympia Build port to P4:$LATEST_SHAS"

    #Label the file versions
    P4DIR=//depot/dev/athens/master/build
    p4 tag -a -l "${P4DATE}_nightly" $P4DIR/arm/*
    p4 tag -a -l "${P4DATE}_nightly" $P4DIR/fledge/*

}

#Used for Incremental step 3 or nightly/full builds
#Clean and reset all repos
function clean_and_reset_all
{
    bb_print "EXECUTE: [git ob-clean-ump]"
    time ob-clean-ump > /dev/null

    bb_print "EXECUTE: [git ob clean -fxd]"
    time git ob clean -fxd > /dev/null

    bb_print "EXECUTE: [git ob fetch]"
    try_x_times 3 git ob fetch || cleanup_and_exit $?

    bb_print "EXECUTE: [git reset p4]"
    pushd $OLYMPIAROOT/p4
        git reset --hard origin/$P4_BRANCH
        sleep 5
    popd

    bb_print "EXECUTE: [git reset olympia]"
    pushd $OLYMPIAROOT/olympia
        git reset --hard origin/$OLYMPIA_BRANCH
        sleep 5
    popd
    bb_print "EXECUTE: [git reset olympiabuildtools]"
    pushd $OLYMPIAROOT/olympiabuildtools
        git reset --hard origin/$BUILDTOOLS_BRANCH
        sleep 5
    popd

}

function try_x_times
{
    num_times=$1
    #Ensure $num_times is an int
    if [ ! $(echo "$num_times" | grep -E "^[0-9]+$") ]; then
        echo "Error [try_x_times]: expected integer as first arg [$num_times]." 2>&1
        return 1
    fi
    shift
    CNT=0
    while [ $CNT -ne $num_times ]; do
        $*
        last_ret=$?
        if [ $last_ret -eq 0 ]; then
            CNT=$num_times
        else
            let "CNT+=1"
        fi
    done
    return $last_ret
}

#Print the current time and buildbot msg
function bb_print
{
    echo "[`date +%T`] BUILDBOT $1"
}

#Try to send an email to the blame list
#This function will get the blame list from the current revision
#and try to send them a mail using bmail.exe
function send_update_mail
{
    #usage: send_update_mail <subject> <message>

    myslavebotname=`echo $SLAVEBOT_NAME | sed 's/__/_%2B/g'`

    #Ensure latest log is $REVISION
    pushd $OLYMPIAROOT/olympia
        blame_email_address=`git log -1 --pretty=%cE`
        current_rev=`git log -1 --pretty=oneline | awk '{print $1}'`
    popd
    if [ "$current_rev" != "$REVISION" ]; then
        bb_print "WARNING: [Not emailing [$blame_email_address] due to unknown revision: [$current_rev]]"
        return 1
    fi
    if [ "`echo $blame_email_address | grep '@rim.com$'`" = "" ];then
        bb_print "WARNING: [Not emailing [$blame_email_address] due to non-compliant email address.]"
        return 1
    fi

    #This is included in the path to the stdio
    device_path="device"
    [ "$DEVICE" = "fledge" ] && device_path=$DEVICE

    #Build nice email message
    MESSAGE="$1\n\n$2"
    [ ! -z "$2" ] && MESSAGE="${MESSAGE}\n\n"
    MESSAGE="${MESSAGE}The BuildSlave [$SLAVEBOT_NAME] encountered an error in this step.\n"
    MESSAGE="${MESSAGE}We are trying to build again.  You can check the current STDIO messages here:\n"
    MESSAGE="${MESSAGE}   http://$ATHENS_SERVER:8010/builders/$myslavebotname/builds/$BUILDNUMBER/steps/$device_path/logs/stdio\n\n"
    MESSAGE="${MESSAGE}Error messages found in the current stdio are:\n"
    MESSAGE="${MESSAGE}========================================\n\n"
    MESSAGE=${MESSAGE}`grep -i error /tmp/buildslave_stdio  | egrep -iv "standard error"\|"0 error"\|'error\.'\|"no error"\|if.*error\|error[a-z]\|[a-z]error`
    MESSAGE="${MESSAGE}\n\n========================================\n"
    MESSAGE="${MESSAGE}Admin: Scotty (sccameron@rim.com)\n"

    echo -e "$MESSAGE\n" > /tmp/buildslave_email_msg
    bb_print "INFO: [Sending warning mail to: $blame_email_address]"

    $OLYMPIAROOT/olympiabuildtools/bin/bmail -s xch81ykf.rim.net -f "olympia-buildbot@rim.com" -t "$blame_email_address,sccameron@rim.com" -h -a "$1" -m /tmp/buildslave_email_msg

    return $?
}

#Pressing the "STOP BUILD" button on the buildbot isn't enough.
#The slave now creates the empty file [$OLYMPIAROOT/STOP_THE_BUILD] for this action
#If the file exists, this function will remove it and exit -1
function check_for_stop_the_build
{
    if [ -f "$BUILDBOTROOT/STOP_THE_BUILD" ]; then
        bb_print "WARNING !!! [STOP_THE_BUILD file found.  Exiting.]"
        rm -f "$BUILDBOTROOT/STOP_THE_BUILD"

        if [ "$TYPE" = "drt" ] || [ "$TYPE" = "full" ]; then
            MAXDRTWAIT=0 && wait_for_fledge_drt 0
        fi
        cleanup_and_exit 0
    fi
}

#If the build succeeds, we need to copy the SHA for each repo
#into files on gitorious for use with the incremental build machine
#from Sam Xiao
function save_successful_shas
{
    SUCCESSFUL_SHA_DIR="$1"
    mkdir -p $SUCCESSFUL_SHA_DIR 2> /dev/null

    for repo in `git ob dirs`; do
        pushd $repo > /dev/null
            this_reponame=`echo $repo | sed 's/.*\///'`
            this_sha=`git log -1 --pretty=oneline | awk {'print $1'}`
            this_branch=`git branch | grep '\*' | awk '{print $2}' | sed 's/\//./g'`
        popd > /dev/null
        echo $this_sha > $SUCCESSFUL_SHA_DIR/${this_reponame}_${this_branch}.txt
        scp $SUCCESSFUL_SHA_DIR/${this_reponame}_${this_branch}.txt buildbot@md-athens-01.swlab.rim.net:SUCCESSFUL_SHAS/ &
    done

}

#Copy binaries to our archive server
function save_binaries
{
    #Return if not nightly and no revision.
    if ( [ -z $NIGHTLY ] && [ -z $REVISION ] ) || ( [ ! -z $NIGHTLY ] && [ "$NIGHTLY" != "$COMPUTERNAME" ] ); then
       bb_print "WARNING [Not saving binaries in archive if not nightly and no revision was given]"
       return 1
    fi

    #Which binaries to save depends on 2 things:
    # 1. Which device was build (fledge/talladega/etc.)
    # 2. What was built (just webkit or full build?)
    #
    #This information is held in:
    # $DEVICE (fledge/talladega/etc.)
    # $TYPE   (full/incremental)
    # $STEP   (if incremental, which step?)

    destination_base="public_html/binaries"
    destination_name="`date +%Y%m%d`"

    #Set archive server path based on incremental/full
    if [ "$NIGHTLY" = "$COMPUTERNAME" ]; then
        destination_name="nightly/$destination_name"
    else
        destination_name="incremental/$destination_name.$REVISION"
    fi
    destination_full_path="$destination_base/$destination_name"

    mkdir -p /tmp/$destination_full_path
    copy_fledge=0
    copy_device=0
    ( [ "$DEVICE" = "fledge" ] || [ "$TYPE" = "full" ] ) && copy_fledge=1
    ( [ "$DEVICE" != "fledge" ] || [ "$TYPE" = "full" ] ) && copy_device=1
    if [ $copy_fledge -gt 0 ]; then
        cp -p $BBNSLDIR/players/build/win32-debug/lib/* /tmp/$destination_full_path/
    fi
    if [ $copy_device -gt 0 ]; then
        cp -p $BBNSLDIR/players/build/xscale-release/lib/OlympiaWebKit.sfi /tmp/$destination_full_path/
        cp -p $BBNSLDIR/players/build/xscale-release/lib/OlympiaWebKit.gdb.elf /tmp/$destination_full_path/

        #Copy the OS and Lynx if built.
        if [ "$TYPE" = "full" ] || [ $STEP -ge 3 ]; then
            cp -p $OLYMPIAJAVA/lynx/jvm/arm/ReleaseARM/rim0x05001807.sfi /tmp/$destination_full_path/
            cp -p $OLYMPIAJAVA/javaapplications/FSLinked.dmp /tmp/$destination_full_path/
        fi

    fi

    #ssh over to archive server
    bb_print "INFO: [Copying binaries to archive server...]"
    bb_print "INFO: [NOTE: This process can take several minutes.]"
    pushd /tmp >/dev/null
        tar -zcvpf - public_html/ | ssh buildbot@$ARCHIVE_SERVER "tar -zxvpf -"
    popd

    #replace latest links on archive server
    bb_print "INFO: [Setting LATEST/ directory on archive server...]"
    ssh buildbot@$ARCHIVE_SERVER "\$HOME/bin/link_latest.sh $destination_name"

    #copy any missing files on archive server
    bb_print "INFO: [Linking missing binaries on archive server...]"
    ssh buildbot@$ARCHIVE_SERVER "cp -vP \$HOME/public_html/binaries/LATEST/* $destination_full_path"

    rm -rf /tmp/public_html
}
#cleanup - checkout default branches
function cleanup
{
    bb_print "INFO: [Checking out default branches...]"
    pushd $OLYMPIAROOT/olympia
        try_x_times 3 git checkout $OLYMPIA_BRANCH;
    popd
    pushd $OLYMPIAROOT/olympiabuildtools
        try_x_times 3 git checkout $BUILDTOOLS_BRANCH
    popd
}
function cleanup_and_exit
{
    cleanup
    exit $1;
}


#===========================
#END OF FUNCTIONS
#===========================

SLAVEBOT_NAME=`echo $PWD | sed 's/.*\\\\//' | sed 's/.*\\///'`

#Remove any leftover STOP_THE_BUILD files
rm -f "$BUILDBOTROOT/STOP_THE_BUILD"

#Change to the real olympiaroot dir.
case "$TYPE" in
incremental|full|drt)
    cd ../incr
    ;;
try)
    cd ../try
    ;;
*)
    echo "Error: Unknown build type [$TYPE]." 2>&1
    exit 1;
esac
bb_print "DIRECTORY: [`pwd`]"



#Set Secret P4 Passwd.
export P4PASSWD=`cat ~/.p4passwd 2>/dev/null`
export P4PORT=perforce:1666
export P4USER=sccameron
export P4EDITOR=gvim

#Set OLYMPIA environment accordingly
OLYMPIAROOT="/`pwd | sed 's/:*\\\/\\//g'`"
export OLYMPIAROOT=`echo $OLYMPIAROOT | sed 's/\\/\\/*/\\//g'`
export OLYMPIAJAVA=$OLYMPIAROOT/p4
export BBNSLDIR=$OLYMPIAJAVA/bbnsl


#Branches
OLYMPIA_BRANCH="master_26"
P4_BRANCH="p4/2010.1-main-pdc"
BUILDTOOLS_BRANCH="master"

#Servers
ARCHIVE_SERVER="10.121.60.207"
ATHENS_SERVER="md-athens-01.swlab.rim.net"

#NOTE: Make sure you run: [drwtsn32.exe -i] on the buildbot
#for error handling.

#Before I source b-o-t, I need to get its latest changes.
#...especially if we were called due to a commit form this repo.
pushd $OLYMPIAROOT/olympiabuildtools
    git reset --hard origin/$BUILDTOOLS_BRANCH || cleanup_and_exit $?
popd

if [ "$BRANCH" = "$BUILDTOOLS_BRANCH" ];then
    setup_repo "olympiabuildtools" "$BUILDTOOLS_BRANCH" "$REVISION"
else
    setup_repo "olympiabuildtools" "$BUILDTOOLS_BRANCH"
fi

source $OLYMPIAROOT/olympiabuildtools/bin/build-olympia-tools.env
export PATH=$OLYMPIATOOLS/bin/ob:$PATH




#Information
echo "
========================================
   DATE:        [`date +%Y'.'%m'.'%d' '%T`]
   SLAVEBOT:    [$SLAVEBOT_NAME]
   ARCHIVE_SVR  [$ARCHIVE_SERVER]
   OLYMPIAROOT: [$OLYMPIAROOT]
   OLYMPIAJAVA: [$OLYMPIAJAVA]
   BBNSLDIR:    [$BBNSLDIR]
   DEVICE:      [$DEVICE]
   REVISION:    [$REVISION]
   TYPE:        [$TYPE]
   BRANCH:      [$OLYMPIA_BRANCH]
   BUILDNUMBER: [$BUILDNUMBER]
   DRT_RESULTS  [$DRT_RESULTS_DIR]
   OS_LATEST    [`cat $OLYMPIAROOT/os_latest/xscale/original_location.txt`]
   OLYMPIADIRS: "
set_latest_shas
echo -e "$LATEST_SHAS\n========================================"


case "$TYPE" in

full)

    #remote os_latest
    bb_print "INFO: [Removing OS_LATEST...]"
    rm -rf $OLYMPIAROOT/os_latest

    #This is the stdio output file used for warnings step
    echo > /tmp/buildslave_stdio

    #Make sure we're on right olympia branch
    pushd $OLYMPIAROOT/olympia
        try_x_times 3 git checkout $OLYMPIA_BRANCH
    popd

    #Full builds are scheduled nightly to rebuild everything from scratch
    check_for_stop_the_build
    clean_and_reset_all

    #Show all SHAs
    set_latest_shas
    bb_print "INFO: [$LATEST_SHAS]"

    #Build device
    bb_print "EXECUTE: [ob talladega]"
    time ob talladega || cleanup_and_exit $?

    bb_print "EXECUTE: [ob-all]"
    check_for_stop_the_build
    time ob-all 2>&1 | tee -a /tmp/buildslave_stdio
    ob_all_device_exit_value=${PIPESTATUS[0]}
    sleep 5; #Allow time for the output to synchronize

    if [ $ob_all_device_exit_value -ne 0 ]; then
        bb_print "ERROR! [Build failed for ob-all (device) (exit value $ob_all_device_exit_value).]"
    else
        bb_print "SUCCESS! [Build passed for ob-all (device).]"
    fi

    sleep 5; #Allow time for the output to synchronize
    bb_print "EXECUTE: [ob fledge]"
    time ob fledge || cleanup_and_exit $?

    bb_print "EXECUTE: [ob-all]"
    check_for_stop_the_build
    time ob-all 2>&1 | tee -a /tmp/buildslave_stdio
    ob_all_fledge_exit_value=${PIPESTATUS[0]}
    sleep 5; #Allow time for the output to synchronize

    if [ $ob_all_fledge_exit_value -ne 0 ]; then
        bb_print "ERROR! [Build failed for ob-all (fledge) (exit value $ob_all_fledge_exit_value).]"
    else
        bb_print "SUCCESS! [Build passed for ob-all (fledge).]"
    fi

    #==============================================
    #Nightly builds (scheduled full builds)
    #   BINARIES -> P4
    #==============================================
    if [ $ob_all_device_exit_value -eq 0 ] || [ $ob_all_fledge_exit_value -eq 0 ]; then
        if [ ! -z $NIGHTLY ] && [ "$NIGHTLY" = "$COMPUTERNAME" ]; then
            bb_print "INFO [Copying Binaries to P4...]"
            copy_binaries_to_p4
        fi
    fi


    #==============================================
    #Actions when full build successful
    #==============================================
    if [ $ob_all_device_exit_value -eq 0 ] && [ $ob_all_fledge_exit_value -eq 0 ]; then

        #Create latest working SHA files...
        bb_print "INFO: [Saving successful SHAs...]"
        save_successful_shas "$OLYMPIAROOT/../successful_repos/$BUILDNUMBER"

        #Tag repos with [nightly_<yyyy.mm.dd>]
        if [ ! -z $NIGHTLY ] && [ "$NIGHTLY" = "$COMPUTERNAME" ]; then
            bb_print "INFO: [Tagging nightly SHAs...]"
            for repo in `git ob dirs`; do
                pushd $repo > /dev/null
                    git tag nightly_`date +%Y'.'%m'.'%d`
                    git push --tags write_upstream 2> /dev/null
                popd > /dev/null
            done
        fi
    fi


    full_build_exit_value=$ob_all_device_exit_value
    let full_build_exit_value+=$ob_all_fledge_exit_value

    #Save Binaries
    if [ $ob_all_device_exit_value -eq 0 ] || [ $ob_all_fledge_exit_value -eq 0 ]; then
        bb_print "INFO: [Saving binaries...]"
        time save_binaries
    fi

    #Run DRTs
    if [ $ob_all_fledge_exit_value -eq 0 ]; then
        bb_print "INFO [Starting Fledge DRT...]"
        execute_and_archive_drt_results
        bb_print "INFO [DONE Starting Fledge DRT...]"
    fi

    cleanup_and_exit $full_build_exit_value
    ;;


incremental|try)
    #Incremental builds are automatically run when a commit occurs in the
    #olympia repository on gitorous

    #try/trial builds are run when submissions are made on the olympia-try
    #repo, on the buildbot_try branch

    #Check for "STOP BUILD" button event
    check_for_stop_the_build

    bb_print "EXECUTE: [ob $DEVICE]"
    time ob $DEVICE || cleanup_and_exit $?

    #This is the stdio output file used for emails
    echo > /tmp/buildslave_stdio

    #Reset, fetch and rebase the olympia repo
    if [ "$BRANCH" = "$OLYMPIA_BRANCH" ];then
        setup_repo "olympia" "$OLYMPIA_BRANCH" "$REVISION"
    else
        setup_repo "olympia" "$OLYMPIA_BRANCH"
    fi

    #Step 1: just build webkit and player
    STEP=1
    if ! [ -z $REVISION ]; then
        pushd $OLYMPIAROOT/olympia
            echo "BUILDBOT EXECUTE: [checkout $REVISION]"
            git checkout $REVISION --quiet
        popd
    fi

    bb_print "INFO: [Trying Incremental Build (Step $STEP)]"
    ob_player_exit_value=0
    ob_webkit_exit_value=0
    ob_bbnsl_exit_value=0

    bb_print "EXECUTE: [ob-webkit]"
    check_for_stop_the_build
    time ob-webkit 2>&1 | tee -a /tmp/buildslave_stdio
    ob_webkit_exit_value=${PIPESTATUS[0]}

    sleep 5; #Allow time for the output to synchronize

    if [ $ob_webkit_exit_value -eq 0 ]; then
        bb_print "EXECUTE: [ob-player]"
        check_for_stop_the_build


        time ob-player 2>&1 | tee -a /tmp/buildslave_stdio
        ob_player_exit_value=${PIPESTATUS[0]}
        sleep 5; #Allow time for the output to synchronize
    fi

    #Step 2:  Clean and rebuild webkit, bbnsl and player
    if [ $ob_webkit_exit_value -ne 0 ] || [ $ob_player_exit_value -ne 0 ]; then
        bb_print "WARNING! [Build-Step ($STEP) Failed for Incremental Build]"
        bb_print "   [Exit Values: (webkit: [$ob_webkit_exit_value] player: [$ob_player_exit_value])]"
        send_update_mail "WARNING! [Build-Step ($STEP) Failed for Incremental Build]" "Next Step: Trying clean/rebuild of bbnsl,webkit, and player"

        STEP=2
        bb_print "INFO: [Trying Incremental Build (Step $STEP)]"

        clean_and_rebuild_bbnsl
        ob_bbnsl_exit_value=$?

        if [ $ob_bbnsl_exit_value -eq 0 ]; then

            if ! [ -z $REVISION ]; then
                pushd $OLYMPIAROOT/olympia
                    echo "BUILDBOT EXECUTE: [checkout $REVISION]"
                    git checkout $REVISION --quiet
                popd
            fi
            bb_print "EXECUTE: [ob-webkit]"
            check_for_stop_the_build
            time ob-webkit 2>&1 | tee -a /tmp/buildslave_stdio
            time ob_webkit_exit_value=${PIPESTATUS[0]}

            if [ $ob_webkit_exit_value -eq 0 ]; then
                bb_print "EXECUTE: [ob-player]"
                check_for_stop_the_build
                time ob-player 2>&1 | tee -a /tmp/buildslave_stdio
                ob_player_exit_value=${PIPESTATUS[0]}
            fi

        fi
    fi

    #Step 3:  Clean All, Reset All, Fetch All, Rebase All, Build All
    if [ $ob_webkit_exit_value -ne 0 ] || [ $ob_player_exit_value -ne 0 ] || [ $ob_bbnsl_exit_value -ne 0 ]; then
        #If incremental builds fail, try a full build
        sleep 5; #Allow time for the output to synchronize
        bb_print "WARNING! [Build-Step ($STEP) Failed for Incremental Build]"
        bb_print "   [Exit Values: (bbnsl: [$ob_bbnsl_exit_value] webkit: [$ob_webkit_exit_value] player: [$ob_player_exit_value])]"
        send_update_mail "WARNING! [Build-Step ($STEP) Failed for Incremental Build]" "Next Step: Trying clean/rebase/rebuild ALL"

        STEP=3
        bb_print "INFO: [Trying Incremental Build (Step $STEP)]"

        pushd $OLYMPIAROOT/olympia
            git checkout $OLYMPIA_BRANCH;
        popd
        check_for_stop_the_build
        clean_and_reset_all

        if ! [ -z $REVISION ]; then
            bb_print "EXECUTE: [checkout $REVISION]"
            pushd $OLYMPIAROOT/olympia
                git checkout $REVISION --quiet
            popd
        fi

        #Show all SHAs
        set_latest_shas
        bb_print "INFO: [$LATEST_SHAS]"

        bb_print "EXECUTE: [ob-all]"
        check_for_stop_the_build
        time ob-all
        ob_all_exit_value=$?

        sleep 5; #Allow time for the output to synchronize
        if [ $ob_all_exit_value -ne 0 ]; then
            bb_print "ERROR! [Build failed for ob-all (exit value $ob_all_exit_value).]"
            cleanup_and_exit $ob_all_exit_value
        else
            bb_print "SUCCESS! [Build passed for ob-all ($DEVICE).]"
        fi
    fi

    sleep 5; #Allow time for the output to synchronize
    cleanup

    #Create latest working SHA files...
    bb_print "INFO: [Saving successful SHAs...]"
    save_successful_shas "$OLYMPIAROOT/../successful_repos/$BUILDNUMBER"

    bb_print "INFO: [Saving binaries...]"
    save_binaries

    cleanup_and_exit 0
    ;;


drt)
    time ob $DEVICE || cleanup_and_exit $?

    #Check DRT results (means failed regression)
    execute_and_archive_drt_results

    cleanup_and_exit $?
    ;;


*)
    echo "Error - unknown build type [$TYPE] ARGS: [$*]" 2>&1
    usage
    cleanup_and_exit 1
    ;;
esac
cleanup_and_exit 0
