# Copyright (C) Research in Motion Limited 2010. All rights reserved.
# Copyright (C) 2007, 2008, 2009 Apple Inc.  All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1.  Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
# 2.  Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
# 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
#     its contributors may be used to endorse or promote products derived
#     from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# Module to share code to get Olympia directories.

use strict;
use warnings;

use FindBin;
use lib $FindBin::Bin;
use File::Basename;
use File::Find;
use File::Spec;
use File::Temp ();
use POSIX;
use Term::ReadKey;
use constant DoNotEchoInput => 1;

BEGIN {
    use Exporter   ();
    our ($VERSION, @ISA, @EXPORT, @EXPORT_OK, %EXPORT_TAGS);
    $VERSION     = 1.00;
    @ISA         = qw(Exporter);
    @EXPORT      = qw(
        &blackBerryConnectionState
        &buildSpecByHardwareId
        &buildToolsDir
        &chdirAthens
        &configurationForFledge
        &determineRepoDir
        &devicePropertyValue
        &dieIfBlackBerryIsNotConnected
        &dieIfNotWindows
        &dieIfRequiredRepositoriesAreMissing
        &exitStatus
        &fledgeProductDir
        &isBlackBerryAccessible
        &isBlackBerryConnected
        &isMacOSX
        &isMinGW
        &isWindows
        &javaApplicationsDir
        &lynxDir
        &lynxProductDir
        &olympiaDir
        &olympiaWebKitProductDir
        &pathToFledge
        &pathToJavaLoader
        &processDirectories
        &productDir
        &promptUser
        &repositories
        &sourceDir
        &toWindowsPath
        DoNotEchoInput
    );
    %EXPORT_TAGS = ( );
    @EXPORT_OK   = ();
}

our @EXPORT_OK;

my $sourceDir;
my $vcBuildPath;

sub repositories
{
    # Note: This list must be kept in sync. with determineSourceDir().
    return ("p4", "olympia", "olympiabuildtools");
}

sub olympiaDir
{
    return sourceDir() . "/olympia";
}

sub dieIfRequiredRepositoriesAreMissing
{
    my $sourceDir = sourceDir();
    die "Cannot find root Athens directory '$sourceDir'\n" unless -d $sourceDir;

    my @directories = repositories();
    my $savedCwd = getcwd();
    chdir($sourceDir);
    foreach my $repo (@directories) {
        if (! -d $repo) {
            print "*************************************************************\n";
            print "No '$sourceDir/$repo' repository found.\n";
            print "Please do a fresh checkout of all of the required repositories.\n";
            print "See the wiki page <http://confluence/display/athens/Getting+the+\n";
            print "Code#GettingtheCode-CheckingOut> for more details.\n";
            print "*************************************************************\n";
            die;
        }
    }
    chdir($savedCwd);
}

sub processDirectories($$)
{
    my ($directories, $processDirectoryFunction) = @_;

    my $savedCwd = getcwd();
    foreach my $directoryName (@$directories) {
        chdir(sourceDir() . "/$directoryName");
        &$processDirectoryFunction($directoryName);
    }
    chdir($savedCwd);
}

sub isMacOSX
{
    return $^O eq "darwin";
}

sub isMinGW
{
    return $^O eq "msys";
}

sub isWindows
{
    return ($^O eq "MSWin32" || isMinGW());
}

sub dieIfNotWindows
{
    if (!isWindows()) {
        die("Cannot run " . basename($0) . " on a non-Windows computer.");
    }
}

# This method is for portability. Return the system-appropriate exit
# status of a child process.
#
# Args: pass the child error status returned by the last pipe close,
#       for example "$?".
sub exitStatus($)
{
    my ($returnvalue) = @_;
    if (isWindows()) {
        return $returnvalue >> 8;
    }
    return WEXITSTATUS($returnvalue);
}


sub toWindowsPath($)
{
    my ($path) = @_;
    $path =~ s/\//\\/g;
    $path = substr($path, 1) if substr($path, 0, 1) eq '\\';
    my $drive = substr($path, 0, index($path, "\\"));
    substr($path, 0, index($path, "\\"), "$drive:") unless $drive =~/:$/;
    return $path;
}

sub determineRepoDir
{
    my $sourceDir = getcwd();
    $sourceDir =~ s|/+$||; # Remove trailing '/' as we would die later

    my %paths = map { sourceDir() . "/$_" => 1 } repositories();

    # Walks up path checking each directory to see if it's one of the Athens repositories.
    until ($paths{$sourceDir}) {
        if ($sourceDir !~ s|/[^/]+$||) {
            die("Could not find top level Athens repository above source directory using FindBin.");
        }
    }
    return $sourceDir;
}

sub determineSourceDir
{
    return if $sourceDir;

    $sourceDir = $ENV{'OLYMPIAROOT'};
    return if defined($sourceDir);

    $sourceDir = $FindBin::Bin;
    $sourceDir =~ s|/+$||; # Remove trailing '/' as we would die later

    # Walks up path checking each directory to see if it is the main Athens
    # project dir, defined by containing Olympia, and P4.
    # Note: These directories must be kept in sync. with repositories().
    until (-d "$sourceDir/olympia" && -d "$sourceDir/p4") {
        if ($sourceDir !~ s|/[^/]+$||) {
            die("Could not find top level Olympia directory above source directory using FindBin.");
        }
    }
}

sub chdirAthens
{
    chdir(sourceDir()) or die;
}

sub sourceDir
{
    determineSourceDir();
    return $sourceDir;
}

sub buildToolsDir
{
    return sourceDir() . "/olympiabuildtools";
}

sub productDir
{
    return sourceDir() . "/publish";
}

sub olympiaWebKitProductDir
{
    my ($architecture, $buildConfig) = @_;

    # FIXME: We should find a way to determine the architecture and
    # build config of the most recent build.
    $architecture = "talladega" if !$architecture;
    $buildConfig = "release" if !$buildConfig;

    my $productPath = productDir();

    return "$productPath/$architecture/webkitplayer/$buildConfig";
}

sub fledgeProductDir
{
    my $javaFilesPath = $ENV{'OLYMPIAJAVA'};
    return "$javaFilesPath/lynx/fledge/bin";
}

sub lynxDir
{
    my $javaFilesPath = $ENV{'OLYMPIAJAVA'};
    return "$javaFilesPath/lynx";
}

sub lynxProductDir
{
    my $lynxPath = lynxDir();
    return "$lynxPath/debug";
}

sub javaApplicationsDir
{
    my $javaFilesPath = $ENV{'OLYMPIAJAVA'};
    return "$javaFilesPath/javaapplications";
}

sub vcBuildPath
{
    setupOlympiaEnv();
    return $vcBuildPath;
}

sub setupOlympiaEnv
{
    return if $vcBuildPath;

    my $vsInstallDir;
    my $programFilesPath = $ENV{'PROGRAMFILES'} || "C:\\Program Files";
    if ($ENV{'VSINSTALLDIR'}) {
        $vsInstallDir = $ENV{'VSINSTALLDIR'};
    } else {
        $vsInstallDir = "$programFilesPath/Microsoft Visual Studio 9.0";
    }
    $vcBuildPath = "$vsInstallDir/Common7/IDE/devenv.com";

    if (! -e $vcBuildPath) {
        print "*************************************************************\n";
        print "Cannot find '$vcBuildPath'\n";
        print "\n";
        print "Please execute the file 'vcvars32.bat' from\n";
        print "'$programFilesPath\\Microsoft Visual Studio 9.0\\VC\\bin\\'\n";
        print "to setup the necessary environment variables.\n";
        print "*************************************************************\n";
        die;
    }
}

sub pathToJavaLoader
{
    return lynxDir() . "/bin/JavaLoader.exe";
}

sub pathToFledge
{
    my $fledgeDir = fledgeProductDir();
    return "$fledgeDir/fledge.exe";
}

sub configurationForFledge
{
    my $lynxProductDir = lynxProductDir();
    # FIXME: We should not hardcode these. Instead, we should determine the device and
    # use the params. in the corresponding .bat file in Lynx/Debug, augmented with any
    # custom params.
    return ("-app=$lynxProductDir/JvmFledgeSimulator.dll", '-app-param=DisableRegistration', '-app-param=NoFileIndexing',
            '-data-port=0x4d44', '-data-port=0x4d4e', '-pin=0x2100000A', '-handheld=talladega', '-session=talladega');
}

sub fledgeLauncherCommand
{
    my ($debugger) = @_;

    my @cmd;
    if ($debugger) {
        setupOlympiaEnv();
        push @cmd, (vcBuildPath(), '-debugExe');
    }
    push @cmd, (pathToFledge(), configurationForFledge());
    return @cmd;
}

sub runFledge
{
    my ($debugger) = @_;
    my $cwd = getcwd();
    chdir(lynxProductDir());
    exitStatus(system(fledgeLauncherCommand($debugger))) == 0 or die "Failed to run fledge: $!.";
    chdir($cwd);
}

sub devicePropertyValue
{
    my ($propertyName, $password) = @_;
    my $javaLoaderApp = pathToJavaLoader();
    my $devNull = File::Spec->devnull();

    my @args = ($javaLoaderApp);
    push @args, "-w'$password'" if $password;
    push @args, ("deviceinfo", "2>$devNull", "|");
    open (OUTPUT, join(" ", @args));
    my $result = parseDeviceInfoProperty(\*OUTPUT, $propertyName);
    close(OUTPUT);
    return $result;
}

sub isBlackBerryAccessible
{
    # FIXME: Need to implement Mac support.
    if (isWindows()) {
        return  !blackBerryConnectionState()->{Inaccessible};
    }
}

sub isBlackBerryConnected
{
    if (isMacOSX()) {
        return length(`ioreg -r -d 1 -n \"BlackBerry\@0\"`) > 0;
    }
    if (isWindows()) {
        return blackBerryConnectionState()->{Connected};
    }
}

sub blackBerryConnectionState
{
    # FIXME: Need to implement Mac support.
    if (isWindows()) {
        # We start JavaLoader in the background and output to a temporary file so that we can detect
        # when JavaLoader blocks for the device password on STDIN.

        my %connectionState;
        my $tempFile = File::Temp->new();
        my $timeout = 5;

        my $pid = fork;
        if ($pid > 0) {
            eval{
                local $SIG{ALRM} = sub {kill(9, $pid); die; };
                alarm $timeout;
                waitpid($pid, 0);
                alarm 0;
            };
        } elsif ($pid == 0) {
            # Ensure JavaLoader has separate IO from parent
            open STDOUT, ">" . $tempFile->filename or die "Cannot write to '" . $tempFile->filename . "' : $!";
            open STDIN, File::Spec->devnull() or die "Cannot read from '" . File::Spec->devnull() . "': $!";
            open STDERR, ">&STDOUT" or die "Can't dup stdout: $!";
            exec(pathToJavaLoader(), "-u", "deviceinfo");
            exit 0;
        }

        # Return the reference to a hash of connection states based on the JavaLoader output.
        open FILE, $tempFile->filename;
        my @output = <FILE>;
        close FILE;

        $connectionState{Connected} = 1 if @output;
        $connectionState{HasPassword} = 1 if grep(/(0x80040008)/, @output);
        $connectionState{Inaccessible} = 1 if grep(/(0x80040001)/, @output);

        return \%connectionState;
    }
}

sub dieIfBlackBerryIsNotConnected
{
    if (!isBlackBerryConnected()) {
        print "*************************************************************\n";
        print "No BlackBerry smartphone connected to this computer.\n";
        print "You must have a BlackBerry smartphone connected via USB\n";
        print "to this computer to be able to run " . basename($0) . ".\n";
        print "*************************************************************\n";
        die;
    }
}

sub dieIfBlackBerryIsNotAccessible
{
    if (!isBlackBerryAccessible()) {
        print "*************************************************************\n";
        print "Failed to communicate with the BlackBerry smartphone.\n";
        print "The device may be booting, missing an OS, or being accessed by\n";
        print "another program.\n";
        print "\n";
        print "Quit any other applications that may communicate with a\n";
        print "BlackBerry smartphone, such as the BlackBerry Desktop Manager,\n";
        print "and the background process BbDevMgr.exe.\n";
        print "Then run " . basename($0) . " again.\n";
        print "*************************************************************\n";
        die;
    }
}

sub buildSpecByHardwareId($)
{
    my ($hardwareId) = @_;
    my %buildSpec;
    my $didFindDeviceFile;
    my $wantedFunction = sub {
        if ($didFindDeviceFile) {
            $File::Find::prune = 1;
            return;
        }
        # The following statements are explicitly ordered since $_ is reused in parseDeviceSpecVariables().
        $File::Find::prune = 1 if /^\../;
        if (/^([a-zA-Z].*)\.env$/) {
            %buildSpec = parseBuildSpec($File::Find::name);
            $didFindDeviceFile = $buildSpec{HardwareId} && ($buildSpec{HardwareId} eq $hardwareId);
        }
    };
    find($wantedFunction, (buildToolsDir() . "/devices"));
    return %buildSpec;
}

sub parseBuildSpec($)
{
    my ($file) = @_;
    open(FILE, $file) or die "Failed to open file '$file': $!";
    my %buildSpec = parseBuildSpecProperties(\*FILE);
    close(FILE);
    return %buildSpec;
}

sub parseBuildSpecProperties($)
{
    my ($fileHandle) = @_;
    my %buildSpec;
    while (<$fileHandle>) {
        # Temporarily strip off any end-of-line characters to simplify
        # regex matching below.
        s/([\n\r]+)$//;

        $buildSpec{PlatformName} = $1 if (/^export PLATFORMNAME=(.*)$/);
        $buildSpec{SFI} = $1 if (/^export SFINAME=(.*)$/);
        $buildSpec{HardwareId} = $1 if ($buildSpec{SFI} && $buildSpec{SFI} =~ /rim([^\.]+).sfi/);
    }
    return %buildSpec;
}

sub parseDeviceInfoProperty($$)
{
    my ($fileHandle, $propertyName) = @_;
    my $didFindProperty;
    my $result = "";
    while (<$fileHandle>) {
        # Temporarily strip off any end-of-line characters to simplify
        # regex matching below.
        s/([\n\r]+)$//;

        if (/^([^:]+):\s*(.+)$/) {
            $didFindProperty = $1 eq $propertyName;
            $result .= $2 if $didFindProperty;
            next;
        }
        $result .= $_ if $didFindProperty;
    }
    return $result;
}

sub promptUser
{
    my ($message, $doNotEchoInput) = @_;

    print $message;
    ReadMode('noecho') if $doNotEchoInput;
    chomp(my $input = <>);
    ReadMode('restore') if $doNotEchoInput;
    print "\n";
    return $input;
}


1;
