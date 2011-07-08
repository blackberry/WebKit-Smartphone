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

#ifndef FontPlatformDataPrivate_h
#define FontPlatformDataPrivate_h

#include "FloatSize.h"
#include "GlyphBuffer.h"

#include <OlympiaPlatformText.h>
#include <wtf/text/WTFString.h>
#include <wtf/Platform.h>
#include <wtf/RefCounted.h>

namespace WebCore {

struct FontPlatformDataStaticPrivate {
    Olympia::Platform::Text::Engine* m_engine;
    Olympia::Platform::Text::GraphicsContext* m_context;

    FontPlatformDataStaticPrivate();
    ~FontPlatformDataStaticPrivate();
};

struct FontPlatformDataPrivate : public RefCounted<FontPlatformDataPrivate> {
    FontPlatformDataPrivate(Olympia::Platform::Text::Font* font, double scaleFactor, Olympia::Platform::Text::FontSpec& matchedSpec, const String& matchedFamily)
        : m_font(font)
        , m_scaleFactor(scaleFactor)
        , m_matchedSpec(matchedSpec)
        , m_matchedFamily(matchedFamily)
    {
    }

    ~FontPlatformDataPrivate();

    Olympia::Platform::Text::Font* m_font;
    double m_scaleFactor; // transforming the large fixed-size font to its actual height
    Olympia::Platform::Text::FontSpec m_matchedSpec;
    String m_matchedFamily;
};

} // namespace WebCore

#endif
