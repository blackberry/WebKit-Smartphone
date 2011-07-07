/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 */

#include "config.h"
#include "OlympiaString.h"

#include "OlympiaStringImpl.h"

namespace Olympia {
namespace WebKit {

String::String(const char* string)
: m_impl(static_cast<StringImpl*>(WebCore::StringImpl::create(string).releaseRef()))
{
}

String::String(const char* string, unsigned length)
: m_impl(static_cast<StringImpl*>(WebCore::StringImpl::create(string, length).releaseRef()))
{
}

String::String(const unsigned short* string, unsigned length)
: m_impl(static_cast<StringImpl*>(WebCore::StringImpl::create(string, length).releaseRef()))
{
}

String::String(StringImpl* impl)
    : m_impl(impl)
{
    if (m_impl)
        m_impl->ref();
}

String::~String()
{
    if (m_impl)
        m_impl->deref();
}

String::String(const String& str)
: m_impl(str.m_impl)
{
    if (m_impl)
        m_impl->ref();
}

String& String::operator=(const String& str)
{
    if (str.m_impl)
        str.m_impl->ref();
    if (m_impl)
        m_impl->deref();
    m_impl = str.m_impl;

    return *this;
}

const unsigned short* String::characters() const
{
    return m_impl ? m_impl->characters() : 0;
}

unsigned String::length() const
{
    return m_impl ? m_impl->length() : 0;
}

bool String::isEmpty() const
{
    return !m_impl || !m_impl->length();
}

} // namespace WebKit
} // namespace Olympia

