#!/usr/bin/perl
#+++++++++++++++++++++++++++++++++
#Scott Cameron <sccameron@rim.com>
#Assign an MKS ID to a given BZ Bug ID
#+++++++++++++++++++++++++++++++++
use strict;
use warnings;
use Bugzilla;
use Bugzilla::Bug;
use Bugzilla::WebService::Bug;
use Switch;




sub addKeyword($$$);
sub createMksTable($);
sub debug_msg($$);
sub deleteKeyword($$$);
sub deleteMksEntry($$);
sub emailDebugFile($);
sub getKeywordId($$);
sub getMksID($$);
sub getMksStatus($$);
sub setMksStatus($$$$);
sub updateMksStatus($$$);

my $bugId = $ARGV[0];
my $EMAIL_FILE = "/tmp/bz_mks_email_file.$$.txt";
my $MksStatus;

# Initialize debug file
unlink $EMAIL_FILE;
debug_msg($EMAIL_FILE, "INFO: BUG: [$bugId]\n");

# Access to the database
my $dbh = Bugzilla->dbh;


# Test BZ Bug ID
if($bugId !~ /^\d+$/){
        debug_msg($EMAIL_FILE, "ERROR: Incorrect BZ Bug ID format: [$bugId]");
        emailDebugFile($EMAIL_FILE);
        exit 1;
}



# Create MKS Table if it doesn't exist
createMksTable($dbh);


# Check Current MKS Status for this bug
my $currentMksStatus = getMksStatus($bugId, $dbh);
switch ($currentMksStatus) {
    case "generating"	{ debug_msg($EMAIL_FILE, "Status is Generating."); exit 0; }
    case "updating"	{ debug_msg($EMAIL_FILE, "Status is Updating."); exit 0; }
    case "assigned"	{ debug_msg($EMAIL_FILE, "Status is Assigned."); exit 0; }
    case "error"	{ 
        debug_msg($EMAIL_FILE, "Previous MKS Status was [error]. Removing entry."); 
        deleteMksEntry($bugId, $dbh);
    }
    elsif ($currentMksStatus) { 
        debug_msg($EMAIL_FILE, "ERROR: Unknown MKS Status: [$currentMksStatus]");
        close EMAIL_FILE;
        emailDebugFile($EMAIL_FILE);
        exit 1;
    }
}


# Get MKS ID
my $mksId = getMksID($bugId, $dbh) ? getMksID($bugId, $dbh) : int(rand(8999)) + 1000; 
debug_msg($EMAIL_FILE, "INFO: MKS_ID: [$mksId]\n");


# Set MKS Status to "generating" - this means I'm generating the MKS ID.
$MksStatus = "generating";
$currentMksStatus = setMksStatus($bugId, $mksId, $MksStatus, $dbh);
if ($currentMksStatus ne $MksStatus){
        debug_msg($EMAIL_FILE, "ERROR: Current MKS Status mismatch: [$MksStatus] <-> [$currentMksStatus]\n");
        emailDebugFile($EMAIL_FILE);
        exit 1;
}


# Test MKSID
if($mksId !~ /^\d{4}$/){
        debug_msg($EMAIL_FILE, "ERROR: Incorrect MKS ID format: [$mksId]");
        emailDebugFile($EMAIL_FILE);
        exit 1;
}




# Set MKS Status to "updating" - this means I've received the MKS ID and am updating the BZ bug.
$MksStatus = "updating";
$currentMksStatus = updateMksStatus($bugId, $MksStatus, $dbh);
if ($currentMksStatus ne $MksStatus){
        debug_msg($EMAIL_FILE, "ERROR: Current MKS Status mismatch: [$MksStatus] <-> [$currentMksStatus]\n");
        close EMAIL_FILE;
        emailDebugFile($EMAIL_FILE);
        updateMksStatus($bugId, "error", $dbh);
        exit 1;
}

# Update BZ bug
# All external methods of _updating_ a bug, including pybugz are not working.
# We will therefore update the fields ourselves.

# Add InMKS and remove NeedsMKS flags
deleteKeyword($bugId, getKeywordId("NeedsMKS", $dbh), $dbh);
addKeyword($bugId, getKeywordId("InMKS", $dbh), $dbh);


# Create Comment with MKS ID
Bugzilla::WebService::Bug::add_comment('Bug.add_comment', { id => $bugId, comment => "MKS ID Assigned: [$mksId]" });



# Set MKS Status to "assigned" - this means I've received the MKS ID and have updated BZ with the info.
$MksStatus = "assigned";
$currentMksStatus = updateMksStatus($bugId, "assigned", $dbh);
if ($currentMksStatus ne $MksStatus){
        debug_msg($EMAIL_FILE, "ERROR: Current MKS Status mismatch: [$MksStatus] <-> [$currentMksStatus]\n");
        emailDebugFile($EMAIL_FILE);
        updateMksStatus($bugId, "error", $dbh);
        exit 1;
}


#EMAIL_FILE is no-longer needed
close EMAIL_FILE;
unlink $EMAIL_FILE;

exit 0;

#+++++++++++++++++++++++++++++++++
# Sub Functions
#+++++++++++++++++++++++++++++++++
sub addKeyword($$$){
    my ($bugId, $keywordID, $dbh) = @_;
    if ( ! $keywordID){
        debug_msg($EMAIL_FILE, "ERROR: Received NULL Keyword.\n");
        emailDebugFile($EMAIL_FILE);
        updateMksStatus($bugId, "error", $dbh);
        exit 1;
    }
    $dbh->do('INSERT into keywords set keywordid = ?, bug_id = ?', undef, $keywordID, $bugId);
}

sub deleteKeyword($$$){
    my ($bugId, $keywordID, $dbh) = @_;
    if ( ! $keywordID){
        debug_msg($EMAIL_FILE, "ERROR: Received NULL Keyword.\n");
        emailDebugFile($EMAIL_FILE);
        updateMksStatus($bugId, "error", $dbh);
        exit 1;
    }
    $dbh->do('DELETE from keywords where keywordid = ? and bug_id = ?', undef, $keywordID, $bugId);
}
sub getKeywordId($$){
    my ($keyword, $dbh) = @_;
    return $dbh->selectrow_array("SELECT id FROM keyworddefs WHERE name = ?", undef, $keyword);
}
sub debug_msg($$){
    my ($file, $msg) = @_;
    open FILE, ">>$file";
    print FILE "$msg\n";
    close FILE;
}

sub emailDebugFile($){
    my ($EMAIL_FILE) = @_;

    #EMAIL_FILE is no-longer needed
    open FILE, $EMAIL_FILE;
    while (<FILE>){
        warn $_;
    }
    close FILE;
    unlink $EMAIL_FILE;
}

sub updateMksStatus($$$){
    my ($bugId, $status, $dbh)  = @_;
    $dbh->do('UPDATE MKS SET status = ?  WHERE bug_id = ?', undef, $status, $bugId);
    return getMksStatus($bugId, $dbh);
}
sub deleteMksEntry($$){
    my ($bugId, $dbh) = @_;
    warn "INFO:DELETE from MKS WHERE bug_id = ?', undef, $bugId]\n";
    $dbh->do('DELETE from MKS WHERE bug_id = ?', undef, $bugId);
}
sub setMksStatus($$$$){
    my ($bugId, $mksId, $status, $dbh) = @_;
    $dbh->do('INSERT into MKS set bug_id = ?, mks_id = ?, status = ?', undef, $bugId, $mksId, $status);
    return getMksStatus($bugId, $dbh);
}

sub getMksID($$){
    my ($bug_id, $dbh) = @_;
    return $dbh->selectrow_array(
        "SELECT mks_id FROM MKS WHERE bug_id = ?", undef, $bugId);
}

sub getMksStatus($$){
    my ($bugId, $dbh) = @_;
    return $dbh->selectrow_array(
        "SELECT status FROM MKS WHERE bug_id = ?", undef, $bugId);
}

sub createMksTable($) {
    my ($dbh) = @_;
    $dbh->do('CREATE TABLE IF NOT EXISTS MKS (bug_id mediumint NOT NULL UNIQUE, mks_id mediumint NOT NULL UNIQUE, status varchar(64), timestamp TIMESTAMP)');
}
close EMAIL_FILE;
