#!/bin/sh
# Opens either the P4ChangeHelper- or the WebKit- commit log editor.

move_back_staged_change_log_if_needed()
{
    if [ $willNeedToRenameStagedP4OutputFile ]
    then
        mv $p4ChangeHelperStagedOutputFile $p4ChangeHelperOutputFile # Restore the staged output file.
    fi
}

if [ "$1" = "--perforce" ] &&  [ ! `perl -e '$_ = <stdin>; print !(/^#/ || /^\s*$/)' < $2` ]
then
    if [ -d "$P4CHANGEHELPERDIR" ]
    then
        p4ChangeHelperOutputFile="$P4CHANGEHELPERDIR/last_change.txt"
        p4ChangeHelperStagedOutputFile="$P4CHANGEHELPERDIR/last_change_staged.txt"
        if [ -e $p4ChangeHelperOutputFile ]
        then
            # We need to temporarily rename an existing Perforce change log so that we can
            # differentiate between 1) when a person exits the P4 Change Helper called below
            # and 2) an existing change log created by another application that called P4
            # Change Helper (so that we avoid interfering with the other application).
            willNeedToRenameStagedP4OutputFile=1
            mv $p4ChangeHelperOutputFile $p4ChangeHelperStagedOutputFile
        fi

        "$P4CHANGEHELPERDIR/p4ch_helper.exe"

        if [ ! -e $p4ChangeHelperOutputFile ]
        then
            echo "No Perforce change log was created."
            move_back_staged_change_log_if_needed
            exit 1
        fi
        # P4ChangeHelper outputs a bunch of meta-data in addition to the actual Perforce
        # change list message. We only want the Perforce change list message, which is the
        # value of the Description field.
        perl -e 'while (<stdin>) { last if /^Description:([\r\n]+)$/; } print while (<stdin>);' < $p4ChangeHelperOutputFile > $2
        rm $p4ChangeHelperOutputFile
        move_back_staged_change_log_if_needed
    else
        echo "*************************************************************"
        if [ $P4CHANGEHELPERDIR ]
        then
            echo "Cannot find \"$P4CHANGEHELPERDIR\"."
        fi
        echo "You must set the P4ChangeHelperDir environment variable to the"
        echo "directory you installed the P4 Change Helper application. You"
        echo "can download P4 Change Helper from <http://indy/wiki/index.php/Percy>."
        echo "*************************************************************"
    fi
else
    # FIXME: Fall back to the WebKitTools commit log editor if we're rebasing/amending/etc. for now. This will
    # ultimately just invoke an editor since a commit message will already exist. Remove the --perforce option
    # from the command line since it won't understand it.
    if [ "$1" = "--perforce" ]
    then
        shift
    fi
    PERL5LIB="$OLYMPIAROOT/olympia/WebKitTools/Scripts"
    commit-log-editor $@
fi
