/*
 *  StringPattern.cpp
 *
 *  Copyright (C) Research In Motion Limited 2010. All rights reserved.
 *
 */

#include "StringPattern.h"

#include "CharacterNames.h"
#include "RegularExpression.h"

#define DEBUG_STRINGPATTERN 0
#if DEBUG_STRINGPATTERN
#include "CString.h"
//#include <sys/log.h>
#endif

namespace WebCore {

StringPattern* StringPattern::s_defaultPattern;
HashMap<String, StringPattern*> StringPattern::s_patterns;

// get the desired StringPattern from the global registry, creating it if necessary
StringPattern& StringPattern::getPattern(const AtomicString& type)
{
    StringPattern* pattern = s_patterns.get(type);

    if (!pattern) {
        if (equalIgnoringCase(type, "telephone"))
            pattern = new TelephoneStringPattern();
        else if (equalIgnoringCase(type, "email"))
            pattern = new EmailStringPattern();
        else if (equalIgnoringCase(type, "url"))
            pattern = new UrlStringPattern();
        else
            pattern = s_defaultPattern;
    }

    return *pattern;
}

const Vector<String> StringPattern::getAllFormats()
{
    Vector<String> keys;
    keys.append("telephone");
    keys.append("email");
    keys.append("url");
    return keys;
}

StringPattern::Match::Match()
    : beginIndex(-1), endIndex(-1)
{
}

StringPattern::StringPattern(const AtomicString& type, const AtomicString& linkPrefix)
    : m_type(type), m_linkPrefix(linkPrefix)
{
    StringPattern* oldPattern = s_patterns.take(type);
    if (oldPattern)
        delete oldPattern;

    s_patterns.set(type, this);
}

StringPattern::~StringPattern()
{
    if (s_patterns.get(m_type) == this)
        s_patterns.remove(m_type);
}

bool StringPattern::findMatch(const String&, int, Match&)
{
    return false;
}

AtomicString StringPattern::type() const
{
    return m_type;
}

AtomicString StringPattern::linkPrefix() const
{
    return m_linkPrefix;
}


RegexStringPattern::RegexStringPattern(const String& regexString, const AtomicString& type, const AtomicString& linkPrefix)
    : StringPattern(type, linkPrefix), m_regex(new RegularExpression(regexString, TextCaseInsensitive))
{
}

RegexStringPattern::~RegexStringPattern()
{
    delete m_regex;
}

bool RegexStringPattern::findMatch(const String& str, int beginIndex, Match& match)
{
    if (str.isEmpty())
        return false;

    int matchLength = 0;
    int matchOffset = m_regex->match(str, beginIndex, &matchLength);

    if (matchOffset < 0) {
        match.beginIndex = match.endIndex = -1;
        return false;
    }

#if DEBUG_STRINGPATTERN
    LOG("  RegexStringPattern::findMatch(\"%s\", %d): len %d, offset %d\n", str.latin1().data(), beginIndex, matchLength, matchOffset);
#endif

    match.beginIndex = beginIndex + matchOffset;
    match.endIndex = match.beginIndex + matchLength - 1;
    return true;
}


// FIXME: we really want (?<=^|\\s) at the front, but JavaScript regexes don't support lookbehind
TelephoneStringPattern::TelephoneStringPattern()
    : RegexStringPattern("(?:^|\\b|(?:\\+|^|\\b)1\\s*[-\\/\\.]?\\s*(?:(?:\\(\\d{3}\\)|\\d{3})\\s*[-\\/\\.]?\\s*)|(?:(?:\\(\\d{3}\\)|(?:^|\\b)\\d{3})\\s*[-\\/\\.]?\\s*))\\d{3}\\s*[-\\/\\.]?\\s*\\d{4}(?:\\s*([xX]|[eE][xX][tT])\\.?\\s*\\d+#?)?(?=\\s|$)", "telephone", "tel:")
{
}


EmailStringPattern::EmailStringPattern()
    : RegexStringPattern("(?:^|\\b)[a-z0-9!#$%&'*+/=?^_`{|}~-]+(?:\\.[a-z0-9!#$%&'*+/=?^_`{|}~-]+)*@(?:[a-z0-9](?:[a-z0-9-]*[a-z0-9])?\\.)+(?:[a-z]{2}|com|org|net|gov|mil|biz|info|mobi|name|aero|jobs|museum|travel)(?=\\s|$)", "email", "mailto:")
{
}


UrlStringPattern::UrlStringPattern()
    : RegexStringPattern("(?:^|\\b)(http[s]?\\:\\/\\/(?:\\w+:\\w+@)?(?:[-\\w]+\\.)*[-\\w]+(?::[\\d]{1,5})?(?:(?:(?:\\/(?:[-\\w~!$+|.,=]|%[a-f\\d]{2})+)+|\\/)+|\\?|#)?(?:(?:\\?(?:[-\\w~!$+|.,*:]|%[a-f\\d{2}])+=?(?:[-\\w~!$+|.,*:=]|%[a-f\\d]{2})*)(?:&(?:[-\\w~!$+|.,*:]|%[a-f\\d{2}])+=?(?:[-\\w~!$+|.,*:=]|%[a-f\\d]{2})*)*)*(?:#(?:[-\\w~!$+|.,*:=]|%[a-f\\d]{2})*)?|[pP][iI][nN]\\:([0-9]{7}|[0-9a-fA-F]{8}))(?=\\s|$)", "url", "")
{
}

}
