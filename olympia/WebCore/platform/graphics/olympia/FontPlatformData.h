/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef FontPlatformData_h
#define FontPlatformData_h

#include "FontPlatformDataPrivate.h"

#include <OlympiaPlatformText.h>
#include <wtf/text/WTFString.h>
#include <wtf/RefPtr.h>

namespace WebCore {

class FontDescription;

class FontPlatformData {
public:
    static Olympia::Platform::Text::Engine* engine();
    static Olympia::Platform::Text::GraphicsContext* textGraphicsContext();

    FontPlatformData();
    FontPlatformData(WTF::HashTableDeletedValueType);
    FontPlatformData(const FontDescription& fontDescription, const String& family);
    FontPlatformData(float size, bool bold, bool italic);

    bool isValid() const;

    bool isFixedPitch() const;

    Olympia::Platform::Text::Font* font() const;
    const String& fontFamily() const;
    double originalHeight() const;

    unsigned hash() const;
    bool isHashTableDeletedValue() const;

    bool operator==(const FontPlatformData& other) const;

#ifndef NDEBUG
    String description() const;
#endif

    int size() const;
    float scaleFactor() const;

private:
    static FontPlatformDataStaticPrivate* staticPrivate();

    RefPtr<FontPlatformDataPrivate> m_private;
};

}

#endif
