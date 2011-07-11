/*
 * Copyright (c) 2008, Google Inc. All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef ScriptString_h
#define ScriptString_h

#include "JSDOMBinding.h"
#include "PlatformString.h"
#include <runtime/UString.h>
#include <wtf/Forward.h>
#include <wtf/Deque.h>

namespace WebCore {

class ScriptString {
public:
    ScriptString() : m_ropeLength(0) {}
    ScriptString(const char* s) : m_str(s), m_ropeLength(0) {}
    ScriptString(const WTF::String& s) : m_str(stringToUString(s)), m_ropeLength(0) {}
    ScriptString(const JSC::UString& s) : m_str(s), m_ropeLength(0) {}

    operator JSC::UString() const { buildIfNeeded(); return m_str; }
    operator WTF::String() const { buildIfNeeded(); return ustringToString(m_str); }
    const JSC::UString& ustring() const { buildIfNeeded(); return m_str; }

    bool isNull() const { return m_str.isNull() && !m_ropeLength; }
    size_t size() const { return m_str.length() + m_ropeLength; }

    ScriptString(const ScriptString& s)
        : m_ropeLength(0)
    {
        s.buildIfNeeded();
        m_str = s.m_str;
    }

    ScriptString& operator=(const ScriptString& s)
    {
        s.buildIfNeeded();
        m_str = s.m_str;
        m_rope.clear();
        m_ropeLength = 0;
        return *this;
    }

    ScriptString& operator=(const char* s)
    {
        m_str = s;
        m_rope.clear();
        m_ropeLength = 0;
        return *this;
    }

    ScriptString& operator+=(const WTF::String& s)
    {
        m_rope.append(stringToUString(s));
        m_ropeLength += s.length();
        return *this;
    }

    bool operator==(const ScriptString& s) const
    {
        buildIfNeeded();
        return m_str == s.m_str;
    }

    bool operator!=(const ScriptString& s) const
    {
        buildIfNeeded();
        // Avoid exporting an extra symbol by re-using "==" operator.
        return !(m_str == s.m_str);
    }

private:

    void buildIfNeeded() const
    {
        if (!m_ropeLength)
            return;

        Vector<UChar> buffer;
        buffer.reserveCapacity(m_str.length() + m_ropeLength);
        buffer.append(m_str.characters(), m_str.length());

        Deque<JSC::UString>::const_iterator it = m_rope.begin();
        Deque<JSC::UString>::const_iterator end = m_rope.end();

        for (; it != end; ++it) {
            const JSC::UString& s = *it;
            buffer.append(s.characters(), s.length());
        }

        ScriptString* that = const_cast<ScriptString*>(this);
        that->m_rope.clear();
        that->m_str = JSC::UString(buffer.data(), buffer.size());
        that->m_ropeLength = 0;
    }

    JSC::UString m_str;
    Deque<JSC::UString> m_rope;
    size_t m_ropeLength;
};

} // namespace WebCore

#endif // ScriptString_h
