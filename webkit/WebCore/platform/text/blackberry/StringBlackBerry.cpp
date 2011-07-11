/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 */

#include "config.h"
#include "PlatformString.h"

#include "WebString.h"
#include "WebStringImpl.h"

using Olympia::WebKit::WebString;
using Olympia::WebKit::WebStringImpl;

using Olympia::WebKit::WebString;
using Olympia::WebKit::WebStringImpl;

namespace WTF {

String::String(const WebString& webString)
    : m_impl(webString.impl())
{
}

String::operator WebString() const
{
    WebString webString(static_cast<WebStringImpl*>(m_impl.get()));
    return webString;
}

} // namespace WTF
