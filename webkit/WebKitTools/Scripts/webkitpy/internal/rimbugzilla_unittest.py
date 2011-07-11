# Copyright (c) 2010 Research In Motion Limited. All rights reserved.
# Research In Motion Limited proprietary and confidential.

import unittest

import datetime
from webkitpy.internal.rimbugzilla import RIMBugzilla

from webkitpy.thirdparty.BeautifulSoup import BeautifulSoup

class RIMBugzillaTest(unittest.TestCase):

    _example_bug = """
    <?xml version="1.0" encoding="UTF-8" standalone="yes" ?>
    <!DOCTYPE bugzilla SYSTEM "https://bugs.webkit.org/bugzilla.dtd">
    <bugzilla version="3.2.3"
              urlbase="https://bugs.webkit.org/"
              maintainer="admin@webkit.org"
              exporter="eric@webkit.org"
    >
        <bug>
              <bug_id>32585</bug_id>
              <creation_ts>2009-12-15 15:17 PST</creation_ts>
              <short_desc>bug to test webkit-patch and commit-queue failures</short_desc>
              <delta_ts>2009-12-27 21:04:50 PST</delta_ts>
              <reporter_accessible>1</reporter_accessible>
              <cclist_accessible>1</cclist_accessible>
              <classification_id>1</classification_id>
              <classification>Unclassified</classification>
              <product>WebKit</product>
              <component>Tools / Tests</component>
              <version>528+ (Nightly build)</version>
              <rep_platform>PC</rep_platform>
              <op_sys>Mac OS X 10.5</op_sys>
              <bug_status>NEW</bug_status>
              <keywords>LayoutTestFailure, Regression</keywords>
              <priority>P2</priority>
              <bug_severity>Normal</bug_severity>
              <target_milestone>---</target_milestone>
              <everconfirmed>1</everconfirmed>
              <reporter name="Eric Seidel">eric@webkit.org</reporter>
              <assigned_to name="Nobody">webkit-unassigned@lists.webkit.org</assigned_to>
              <cc>foo@bar.com</cc>
        <cc>example@example.com</cc>
              <long_desc isprivate="0">
                <who name="Eric Seidel">eric@webkit.org</who>
                <bug_when>2009-12-15 15:17:28 PST</bug_when>
                <thetext>bug to test webkit-patch and commit-queue failures

    Ignore this bug.  Just for testing failure modes of webkit-patch and the commit-queue.</thetext>
              </long_desc>
              <attachment 
                  isobsolete="0"
                  ispatch="1"
                  isprivate="0"
              > 
                <attachid>45548</attachid> 
                <date>2009-12-27 23:51 PST</date> 
                <desc>Patch</desc> 
                <filename>bug-32585-20091228005112.patch</filename> 
                <type>text/plain</type> 
                <size>10882</size> 
                <attacher>mjs@apple.com</attacher> 

                  <token>1261988248-dc51409e9c421a4358f365fa8bec8357</token> 
                  <data encoding="base64">SW5kZXg6IFdlYktpdC9tYWMvQ2hhbmdlTG9nCj09PT09PT09PT09PT09PT09PT09PT09PT09PT09
    removed-because-it-was-really-long
    ZEZpbmlzaExvYWRXaXRoUmVhc29uOnJlYXNvbl07Cit9CisKIEBlbmQKIAogI2VuZGlmCg==
    </data>        

                  <flag name="review"
                        id="27602"
                        status="?"
                        setter="mjs@apple.com"
                   /> 
              </attachment> 
        </bug>
    </bugzilla>
    """

    _expected_example_bug_parsing = {
        "id" : 32585,
        "title" : u"bug to test webkit-patch and commit-queue failures",
        "cc_emails" : ["foo@bar.com", "example@example.com"],
        "reporter_email" : "eric@webkit.org",
        "assigned_to_email" : "webkit-unassigned@lists.webkit.org",
        "platform" : "PC",
        "os" : "Mac OS X 10.5",
        "long_description" : """bug to test webkit-patch and commit-queue failures

Ignore this bug.  Just for testing failure modes of webkit-patch and the commit-queue.""",
        "keywords" : ["LayoutTestFailure", "Regression"],
        "component" : "Tools / Tests",
        "attachments" : [{
            "attach_date": datetime.datetime(2009, 12, 27, 23, 51),
            'name': u'Patch',
            'url' : "https://bugs.webkit.org/attachment.cgi?id=45548",
            'is_obsolete': False,
            'review': '?',
            'is_patch': True,
            'attacher_email': 'mjs@apple.com',
            'bug_id': 32585,
            'type': 'text/plain',
            'id': 45548
        }],
    }

    def test_bug_parsing(self):
        bug = RIMBugzilla()._parse_bug_page(self._example_bug)
        self._assert_dictionaries_equal(bug, self._expected_example_bug_parsing)

    # FIXME: This should move to a central location and be shared by more unit tests.
    def _assert_dictionaries_equal(self, actual, expected):
        # Make sure we aren't parsing more or less than we expect
        self.assertEquals(sorted(actual.keys()), sorted(expected.keys()))
