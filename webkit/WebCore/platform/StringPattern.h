/*
 *  StringPattern.h
 *
 *  Copyright (C) Research In Motion Limited 2010. All rights reserved.
 *
 */

#ifndef StringPattern_h
#define StringPattern_h

#include "AtomicString.h"
#include "HashMap.h"
#include "PlatformString.h"
#include "StringHash.h"

namespace WebCore {

class RegularExpression;

class StringPattern
{
public:
    static StringPattern& getPattern(const AtomicString& type);
    static const Vector<String> getAllFormats();

    class Match
    {
    public:
        int beginIndex;
        int endIndex;

        Match();
    };

    StringPattern(const AtomicString& type, const AtomicString& linkPrefix);
    virtual ~StringPattern();

    virtual bool findMatch(const String& str, int beginIndex, Match& match);

    AtomicString type() const;
    AtomicString linkPrefix() const;

private:
    static StringPattern* s_defaultPattern;
    static HashMap<String, StringPattern*> s_patterns;

    AtomicString m_type;
    AtomicString m_linkPrefix;
};


class RegexStringPattern : public StringPattern
{
public:
    RegexStringPattern(const String& regexString, const AtomicString& type, const AtomicString& linkPrefix);
    ~RegexStringPattern();

    bool findMatch(const String& str, int beginIndex, Match& match);

private:
    RegularExpression* m_regex;
};


class TelephoneStringPattern : public RegexStringPattern
{
public:
    TelephoneStringPattern();
};


class EmailStringPattern : public RegexStringPattern
{
public:
    EmailStringPattern();
};


class UrlStringPattern : public RegexStringPattern
{
public:
    UrlStringPattern();
};

}

#endif
