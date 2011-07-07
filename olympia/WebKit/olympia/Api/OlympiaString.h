/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 */

#ifndef OlympiaString_h
#define OlympiaString_h

#include "OlympiaGlobal.h"

namespace Olympia {
namespace WebKit {

    class StringImpl;

    class OLYMPIA_EXPORT String {
    public:
        String() : m_impl(0) {}
        ~String();
        String(const char*);
        String(const char*, unsigned length);
        String(const unsigned short*, unsigned length);
        String(StringImpl*);
        String(const String&);
        String& operator=(const String&);

        const unsigned short* characters() const;
        unsigned length() const;
        bool isEmpty() const;

        StringImpl* impl() const { return m_impl; }
    private:
        StringImpl* m_impl;
    };
} // namespace WebKit
} // namespace Olympia

#endif // OlympiaString_h
