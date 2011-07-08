# Copyright (c) 2010 Research In Motion Limited. All rights reserved.
# Research In Motion Limited proprietary and confidential.
#
# Python module for interacting with RIM's internal NeedsMKS queue

import os

from webkitpy.internal.rimbugzilla import RIMBugzilla, RIMBugzillaQueries
from webkitpy.internal.mksintegrity import MKSIntegrity, MKSBug

from webkitpy.tool.commands.queues import AbstractQueue, StepSequenceErrorHandler

class NeedsMKSQueue(AbstractQueue, StepSequenceErrorHandler):
    name = "rim-needs-mks-queue"
    def __init__(self):
        AbstractQueue.__init__(self)

    # AbstractQueue methods

    def begin_work_queue(self):
        AbstractQueue.begin_work_queue(self)
        # FIXME: We should find a better place to put this code so that we instantiate
        # a RIMBugzilla and MKSIntegrity instance with the dryrun paramater instead of
        # overridding the passed Bugzilla instance in self.tool.bugs.
        dryrun = self.tool.bugs.dryrun;
        self.tool.bugs = RIMBugzilla(dryrun=dryrun)
        self.mksintegrity = MKSIntegrity(dryrun=dryrun)

    def next_work_item(self):
        bug_ids = self.tool.bugs.needs_mks()
        all_bugs = [self.tool.bugs.fetch_bug(bug_id) for bug_id in bug_ids]
        if (all_bugs):
            return all_bugs[0]
        return None

    def should_proceed_with_work_item(self, bug):
        return True

    def work_item_log_path(self, bug):
        return os.path.join(self._log_directory(), "%s.log" % bug.id())

    def process_work_item(self, bug):
        mks_id = self.mksintegrity.create_bug(MKSBug(bug))
        self.tool.bugs.substitute_keyword(bug.id(), "NeedsMKS", "InMKS")
        self.tool.bugs.post_comment_to_bug(bug.id(), "MKS Bug #%d <http://mksintegrity:7001/im/viewissue?selection=%d>" % (mks_id, mks_id))
        return True

    def handle_unexpected_error(self, bug, message):
        print "Error when handling bug #%d: %s" % (bug.id(), message)

    # StepSequenceErrorHandler methods

    @classmethod
    def handle_script_error(cls, tool, state, script_error):
        # FIXME: We need to make this error message more informative.
        print "Script error occurred."
