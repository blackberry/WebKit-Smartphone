# Copyright (c) 2010 Research In Motion Limited. All rights reserved.
# Research In Motion Limited proprietary and confidential.
#
# Python module for interacting with RIM's internal Bugzilla

import os.path
import re
import StringIO

from webkitpy.common.system.deprecated_logging import log, error
from webkitpy.common.net.bugzilla import Bugzilla, BugzillaQueries, Bug
from webkitpy.thirdparty.BeautifulSoup import BeautifulSoup, SoupStrainer

class RIMBug(Bug):

    def __init__(self, bug_dictionary, bugzilla=None):
        Bug.__init__(self, bug_dictionary, bugzilla)

    def title(self):
        return self.bug_dictionary["title"]

    def description(self):
        return self.bug_dictionary["long_description"]

    def reporter_email(self):
        return self.bug_dictionary["reporter_email"]

    def keywords(self):
        return self.bug_dictionary["keywords"].split(", ")

    def platform(self):
        return self.bug_dictionary["platform"]

    def os(self):
        return self.bug_dictionary["os"]

    def component(self):
        return self.bug_dictionary["component"]

# A container for all of the logic for making and parsing buzilla queries.
class RIMBugzillaQueries(BugzillaQueries):

    def __init__(self, bugzilla):
        BugzillaQueries.__init__(self, bugzilla)

    def fetch_bug_ids_that_need_mks(self):
        needs_mks_url = "buglist.cgi?query_format=advanced&short_desc_type=allwordssubstr&short_desc=&long_desc_type=substring&long_desc=&bug_file_loc_type=allwordssubstr&bug_file_loc=&keywords_type=allwords&keywords=&bug_status=UNCONFIRMED&bug_status=NEW&bug_status=ASSIGNED&bug_status=REOPENED&emailassigned_to1=1&emailtype1=substring&email1=&emailassigned_to2=1&emailreporter2=1&emailcc2=1&emailtype2=substring&email2=&bugidtype=include&bug_id=&chfieldfrom=&chfieldto=Now&chfieldvalue=&cmdtype=doit&order=Last+Changed&field0-0-0=keywords&type0-0-0=substring&value0-0-0=NeedsMKS&field0-1-0=keywords&type0-1-0=notsubstring&value0-1-0=InMKS&field0-2-0=keywords&type0-2-0=notsubstring&value0-2-0=ReviewedForMKS"
        return self._fetch_bug_ids_advanced_query(needs_mks_url)

class RIMBugzilla(Bugzilla):

    def __init__(self, dryrun=False, committers=None):
        Bugzilla.__init__(self, dryrun, committers)
        self.queries = RIMBugzillaQueries(self)

    # FIXME: Much of this should go into some sort of config module:
    bug_server_host = "bugzilla-torch.rim.net"
    bug_server_regex = "https?://%s/" % re.sub('\.', '\\.', bug_server_host)
    bug_server_url = "http://%s/" % bug_server_host

    def needs_mks(self):
        return self.queries.fetch_bug_ids_that_need_mks()

    def fetch_bug(self, bug_id):
        return RIMBug(self.fetch_bug_dictionary(bug_id), self)

    def add_keyword(self, bug_id, keyword):
        self.authenticate()

        log("Adding keyword %s to bug %s" % (keyword, bug_id))
        if self.dryrun:
            return

        self.browser.open(self.bug_url_for_bug_id(bug_id))
        self.browser.select_form(name="changeform")
        self.browser["keywords"] = "%s %s" % (self.browser["keywords"], keyword)
        self.browser.submit()

    def remove_keyword(self, bug_id, keyword):
        self.authenticate()

        log("Removing keyword %s from bug %s" % (keyword, bug_id))
        if self.dryrun:
            return

        self.browser.open(self.bug_url_for_bug_id(bug_id))
        self.browser.select_form(name="changeform")
        keywords = self.browser["keywords"].split(", ")
        keywords.remove(keyword)
        self.browser["keywords"] = ", ".join(keywords)
        self.browser.submit()

    def substitute_keyword(self, bug_id, keyword, new_keyword):
        self.authenticate()

        log("Removing keyword %s from bug %s" % (keyword, bug_id))
        log("Adding keyword %s to bug %s" % (new_keyword, bug_id))
        if self.dryrun:
            return

        self.browser.open(self.bug_url_for_bug_id(bug_id))
        self.browser.select_form(name="changeform")
        keywords = self.browser["keywords"].split(", ")
        keywords.remove(keyword)
        keywords.append(new_keyword)
        self.browser["keywords"] = ", ".join(keywords)
        self.browser.submit()

    def _parse_bug_page(self, page):
        soup = BeautifulSoup(page)
        bug = {}
        bug["id"] = int(soup.find("bug_id").string)
        bug["title"] = self._string_contents(soup.find("short_desc"))
        bug["reporter_email"] = self._string_contents(soup.find("reporter"))
        bug["assigned_to_email"] = self._string_contents(soup.find("assigned_to"))
        bug["cc_emails"] = [self._string_contents(element)
                            for element in soup.findAll('cc')]
        bug["attachments"] = [self._parse_attachment_element(element, bug["id"]) for element in soup.findAll('attachment')]
        bug["platform"] = self._string_contents(soup.find("rep_platform"))
        bug["os"] = self._string_contents(soup.find("op_sys"))
        bug["long_description"] = self._string_contents(soup.find("long_desc").findNext("thetext"))
        bug["keywords"] = self._string_contents(soup.find("keywords"))
        bug["component"] = self._string_contents(soup.find("component"))
        return bug
