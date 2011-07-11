/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 */

#include "config.h"
#include "WebString.h"

#include "WebStringImpl.h"

using WTF::String;

namespace Olympia {
namespace WebKit {

WebString::WebString(const char* string)
: m_impl(static_cast<WebStringImpl*>(WTF::StringImpl::create(string).releaseRef()))
{
}

WebString::WebString(const char* string, unsigned length)
: m_impl(static_cast<WebStringImpl*>(WTF::StringImpl::create(string, length).releaseRef()))
{
}

WebString::WebString(const unsigned short* string, unsigned length)
: m_impl(static_cast<WebStringImpl*>(WTF::StringImpl::create(string, length).releaseRef()))
{
}

WebString::WebString(WebStringImpl* impl)
    : m_impl(impl)
{
    if (m_impl)
        m_impl->ref();
}

WebString::~WebString()
{
    if (m_impl)
        m_impl->deref();
}

WebString::WebString(const WebString& str)
: m_impl(str.m_impl)
{
    if (m_impl)
        m_impl->ref();
}

WebString& WebString::operator=(const WebString& str)
{
    if (str.m_impl)
        str.m_impl->ref();
    if (m_impl)
        m_impl->deref();
    m_impl = str.m_impl;

    return *this;
}

const unsigned short* WebString::characters() const
{
    return m_impl ? m_impl->characters() : 0;
}

unsigned WebString::length() const
{
    return m_impl ? m_impl->length() : 0;
}

bool WebString::isEmpty() const
{
    return !m_impl || !m_impl->length();
}

bool WebString::equal(const char* utf8) const
{
    return WTF::equal(m_impl, utf8);
}

bool WebString::equalIgnoringCase(const char* utf8) const
{
    return WTF::equalIgnoringCase(m_impl, utf8);
}

} // namespace WebKit
} // namespace Olympia
