/*
 * Copyright (C) 2009 Torch Mobile Inc. http://www.torchmobile.com/
 */

#include "config.h"
#include "PlatformString.h"

#include "OlympiaString.h"
#include "OlympiaStringImpl.h"

namespace WebCore {

String::String(const Olympia::WebKit::String& olympiaString)
    : m_impl(olympiaString.impl())
{
}

String::operator Olympia::WebKit::String() const
{
    Olympia::WebKit::String olympiaString(static_cast<Olympia::WebKit::StringImpl*>(m_impl.get()));
    return olympiaString;    
}

} // namespace WebCore
