# Copyright (C) Research in Motion Limited 2010. All rights reserved.
#
# Various SCM functions.

use warnings;
use strict;

use FindBin;
use lib $FindBin::Bin;
use File::Basename;
use File::Spec;
use POSIX;

use constant GitResetSoft => "--soft";
use constant GitResetHard => "--hard";
use constant GitResetMerge => "--merge";

BEGIN {
    use Exporter   ();
    our ($VERSION, @ISA, @EXPORT, @EXPORT_OK, %EXPORT_TAGS);
    $VERSION     = 1.00;
    @ISA         = qw(Exporter);
    @EXPORT      = qw(
        GitResetSoft
        GitResetHard
        GitResetMerge
        &gitBranch
        &gitFetch
        &gitLastFetchedSHA
        &gitRebase
        &gitRemoteBranch
        &gitRemoteBranches
        &gitReset
        &gitPopStash
        &gitPushStash
        &isGitDirectory
        &isGitWorkingDirectoryClean
        &isNewGitCheckout
    );
    %EXPORT_TAGS = ( );
    @EXPORT_OK   = ();
}

our @EXPORT_OK;

sub isGitDirectory
{
    return exitStatus(system("git rev-parse > " . File::Spec->devnull() . " 2>&1")) == 0;
}

sub isNewGitCheckout
{
    chomp(my $output = `git rev-parse --verify HEAD`);
    return $output eq "fatal: Needed a single revision";
}

sub gitLastFetchedSHA($)
{
    my ($remoteBranch) = @_;
    chomp(my $output = `git rev-parse $remoteBranch`);
    return $output;
}

sub gitReset($$)
{
    my ($level, $sha) = @_;
    my @cmd = ("git", "reset");
    push @cmd, GitResetSoft if $level eq GitResetSoft;
    push @cmd, GitResetHard if $level eq GitResetHard;
    push @cmd, GitResetMerge if $level eq GitResetMerge;
    push @cmd, $sha; # The sha must be the last command-line argument.
    exitStatus(system(@cmd)) == 0 or die "Failed to run '" . join(" ", @cmd) . "': $!.";
}

sub gitPushStash
{
    exitStatus(system("git", "stash", "--quiet")) == 0 or die "Failed to run 'git stash': $!.";
}

sub gitPopStash
{
    exitStatus(system("git", "stash", "pop", "--quiet")) == 0 or die "Failed to run 'git stash pop': $!.";
}

sub isGitWorkingDirectoryClean
{
    chomp(my $output = `git diff HEAD --name-only`);
    return $output eq "";
}

sub gitBranch
{
    chomp(my $gitBranch = `git symbolic-ref -q HEAD`);
    $gitBranch = "" if exitStatus($?);
    $gitBranch =~ s#^refs/heads/##;
    return $gitBranch;
}

sub gitRemoteBranch
{
    my $localBranchName = gitBranch();
    if (!$localBranchName) {
        return "";
    }

    chomp(my $remote = `git config branch.$localBranchName.remote`);
    if (exitStatus($?)) {
        return "";
    }

    chomp(my $merge = `git config branch.$localBranchName.merge`);
    if (exitStatus($?)) {
        return "";
    }

    $merge =~ s#^refs/heads/##;
    return "$remote/$merge";
}

sub gitRemoteBranches
{
    my @branches = ();
    open(OUTPUT, "git branch -r|");
    while (<OUTPUT>) {
        s/([\n\r]+)$//mg;
        push(@branches, $1) if /\s+([^ ]+)/;
    }
    close(OUTPUT);
    return @branches;
}

sub gitFetch($)
{
    my ($remoteName) = @_;
    exitStatus(system("git", "fetch", "--quiet", $remoteName)) == 0 or die "Failed to run 'git fetch --quiet $remoteName': $!.";
}

sub gitRebase
{
    my ($newBase, $statFilterFunction) = @_;
    my @cmd = ("git", "rebase");
    if (!defined($statFilterFunction)) {
        exitStatus(system((@cmd, "--quiet", $newBase))) == 0 or die "Failed to run 'git rebase --quiet $newBase': $!.";
        return;
    }
    open(OUTPUT, join(" ", (@cmd, "--stat", $newBase, "|"))) or die "Failed to run 'git rebase --quiet $newBase': $!.";
    while (<OUTPUT>) {
        s/([\n\r]+)$//mg;
        &$statFilterFunction($_);
    }
    close(OUTPUT);
}


1;
