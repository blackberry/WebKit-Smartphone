/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 */

#ifndef WebString_h
#define WebString_h

#include "BlackBerryGlobal.h"

namespace Olympia {
namespace WebKit {

    class WebStringImpl;

    class OLYMPIA_EXPORT WebString {
    public:
        WebString() : m_impl(0) {}
        ~WebString();
        WebString(const char*);
        WebString(const char*, unsigned length);
        WebString(const unsigned short*, unsigned length);
        WebString(WebStringImpl*);
        WebString(const WebString&);
        WebString& operator=(const WebString&);

        const unsigned short* characters() const;
        unsigned length() const;
        bool isEmpty() const;

        bool equal(const char* utf8) const;
        bool equalIgnoringCase(const char* utf8) const;

        WebStringImpl* impl() const { return m_impl; }
    private:
        WebStringImpl* m_impl;
    };
} // namespace WebKit
} // namespace Olympia

#endif // WebString_h
