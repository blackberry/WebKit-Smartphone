# -*- Mode: perl; indent-tabs-mode: nil -*-
#
# The contents of this file are subject to the Mozilla Public
# License Version 1.1 (the "License"); you may not use this file
# except in compliance with the License. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS
# IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
# implied. See the License for the specific language governing
# rights and limitations under the License.
#
# The Original Code is the Bugzilla Example Plugin.
#
# The Initial Developer of the Original Code is Everything Solved, Inc.
# Portions created by Everything Solved are Copyright (C) 2008 
# Everything Solved, Inc. All Rights Reserved.
#
# Contributor(s): Max Kanat-Alexander <mkanat@bugzilla.org>


#+++++++++++++++++++++++++++++++++
#Scott Cameron <sccameron@rim.com>
#
#Add checks for NeedsMKS and get a MKS ID.
#+++++++++++++++++++++++++++++++++
use strict;
use warnings;
use Bugzilla;

$|++;
my $args = Bugzilla->hook_args;
my $bug = $args->{'bug'};
my $bug_id = $bug->id;
my $MKSCodePath = Bugzilla::Constants::bz_locations()->{extnesionsdir} ? Bugzilla::Constants::bz_locations()->{extnesionsdir} : "/usr/share/bugzilla3/extensions";
$MKSCodePath .= "/MKS/code";

if (! -d $MKSCodePath ) {
    return 1;
}

if (my $pid = fork) {
    close STDOUT;
    my %keyword_names;
    foreach my $keyword_object (@{$bug->{'keyword_objects'}}){
        $keyword_names{$keyword_object->{'name'}} = $keyword_object->{'id'};
    }

    #Exit if has ReviewedForMKS
    if (exists $keyword_names{'ReviewedForMKS'}){
        return;
    }

    #Exit if has InMKS
    if (exists $keyword_names{'InMKS'}){
        return;
    }

    #Exit if no NeedsMKS
    if (! exists $keyword_names{'NeedsMKS'}){
        return;
    }
    exec("$MKSCodePath/mks-create_comment.pl $bug_id &");
}
