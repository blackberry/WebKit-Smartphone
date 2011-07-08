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

#include "config.h"
#include "FontPlatformData.h"

#include "AtomicString.h"

#if PLATFORM(EGL)
#include "EGLDisplayOpenVG.h"
#include "EGLUtils.h"
#endif

#include "FontDescription.h"
#include "SurfaceOpenVG.h"
#include "VGUtils.h"

#include <wtf/MathExtras.h>
#include <wtf/StdLibExtras.h>

#define OLYMPIA_FIXED_FONT_SIZE 0

namespace WebCore {

FontPlatformDataStaticPrivate::FontPlatformDataStaticPrivate()
{
    m_engine = Olympia::Platform::Text::engine();
    ASSERT(m_engine);

    Olympia::Platform::Text::NativeGraphicsDisplay display;
#if PLATFORM(EGL)
    ASSERT_EGL_NO_ERROR();
    EGLDisplayOpenVG* displayManager = EGLDisplayOpenVG::current();
    display = static_cast<Olympia::Platform::Text::NativeGraphicsDisplay>(displayManager->display());
#endif
    Olympia::Platform::Text::ReturnCode error;
    m_context = m_engine->createGraphicsContext(error, Olympia::Platform::Text::OpenVGGraphicsContext, display);
    ASSERT(m_context);
    ASSERT(!error);

#if PLATFORM(EGL)
    ASSERT_EGL_NO_ERROR();
    SurfaceOpenVG* sharedSurface = displayManager->sharedPlatformSurface();
    m_context->setDisplay(static_cast<Olympia::Platform::Text::NativeGraphicsDisplay>(sharedSurface->eglDisplay()));
    m_context->setSurface(static_cast<Olympia::Platform::Text::NativeGraphicsSurface>(sharedSurface->eglSurface()));
    m_context->setContext(static_cast<Olympia::Platform::Text::NativeGraphicsContext>(sharedSurface->eglContext()));
    ASSERT_EGL_NO_ERROR();
#endif
}

FontPlatformDataStaticPrivate::~FontPlatformDataStaticPrivate()
{
    delete m_context;
}

FontPlatformDataPrivate::~FontPlatformDataPrivate()
{
    delete m_font;
}


// Static methods.

Olympia::Platform::Text::Engine* FontPlatformData::engine()
{
    return staticPrivate()->m_engine;
}

Olympia::Platform::Text::GraphicsContext* FontPlatformData::textGraphicsContext()
{
    return staticPrivate()->m_context;
}

FontPlatformDataStaticPrivate* FontPlatformData::staticPrivate()
{
    DEFINE_STATIC_LOCAL(FontPlatformDataStaticPrivate, staticPrivate, ());
    return &staticPrivate;
}


// Instance methods.

FontPlatformData::FontPlatformData()
    : m_private(0)
{
}

FontPlatformData::FontPlatformData(WTF::HashTableDeletedValueType)
    : m_private(WTF::HashTableDeletedValue)
{
}

FontPlatformData::FontPlatformData(const FontDescription& fontDescription, const String& family)
    : m_private(0)
{
    Olympia::Platform::Text::ReturnCode error;
    Olympia::Platform::Text::FontSpec spec;

#if OLYMPIA_FIXED_FONT_SIZE
    float scaleFactor = static_cast<float>(fontDescription.computedPixelSize()) / OLYMPIA_FIXED_FONT_SIZE;
    if (scaleFactor < 0.001)
        scaleFactor = 0.001; // avoid divisions by 0.0
    spec.m_height = OLYMPIA_FIXED_FONT_SIZE;
#else
    float scaleFactor = 1.0; // used in FontPlatformDataPrivate constructor
    spec.m_height = fontDescription.computedPixelSize();
#endif

    spec.m_style = fontDescription.italic()
        ? Olympia::Platform::Text::ItalicStyle
        : Olympia::Platform::Text::PlainStyle;
    spec.m_variant = fontDescription.smallCaps()
        ? Olympia::Platform::Text::SmallCapsVariant
        : Olympia::Platform::Text::PlainVariant;

    switch (fontDescription.weight()) {
    case FontWeight100:
        spec.m_weight = 100;
        break;
    case FontWeight200:
        spec.m_weight = 200;
        break;
    case FontWeight300:
        spec.m_weight = 300;
        break;
    case FontWeight400:
        spec.m_weight = 400;
        break;
    case FontWeight500:
        spec.m_weight = 500;
        break;
    case FontWeight600:
        spec.m_weight = 600;
        break;
    case FontWeight700:
        spec.m_weight = 700;
        break;
    case FontWeight800:
        spec.m_weight = 800;
        break;
    case FontWeight900:
    default:
        spec.m_weight = 900;
        break;
    }

    if (family.length()) {
        const Olympia::Platform::Text::Utf16Char* characters = family.characters();
        unsigned int length = family.length();

        if (length > Olympia::Platform::Text::MAX_FONT_NAME)
            length = Olympia::Platform::Text::MAX_FONT_NAME;
        for (unsigned int i = 0; characters[i] && i < length; i++)
            spec.m_name[i] = characters[i];
    }

    Olympia::Platform::Text::Font* font = engine()->createFont(error, spec);

    if (error || !font)
        return; // without setting m_private to something non-zero

    Olympia::Platform::Text::FontSpec matchedSpec;
    font->getMatchedFontSpec(matchedSpec);

    String matchedFamily(matchedSpec.m_name, Olympia::Platform::Text::MAX_FONT_NAME);
    matchedFamily = matchedFamily.replace('\0', ' ').stripWhiteSpace();

    // We don't want to match fonts with different family names,
    // FontCache expects us to not return a valid font in that case.
    if (!family.isEmpty() && family != "-webkit-last-resort") {
        bool familyMatches = equalIgnoringCase(family, matchedFamily);

        if (!familyMatches) {
            // FIXME: We know we'll be getting a suitable fallback,
            // but we should not have to hardcode certain fonts here,
            // the Text API should tell us whether or not we got a good match.
            if (equalIgnoringCase(family, "Calibri") || equalIgnoringCase(family, "Consolas") || equalIgnoringCase(family, "Candara"))
                familyMatches = true;
        }

        if (!familyMatches) {
            delete font;
            return; // without setting m_private to something non-zero
        }
    }

    m_private = adoptRef(new FontPlatformDataPrivate(font, scaleFactor, matchedSpec, matchedFamily));
}

FontPlatformData::FontPlatformData(float size, bool bold, bool italic)
    : m_private(0)
{
    if (size <= FLT_EPSILON) // see FontDataCacheKeyTraits::emptyValue()
        return;

    Olympia::Platform::Text::ReturnCode error;
    Olympia::Platform::Text::FontSpec spec;

#if OLYMPIA_FIXED_FONT_SIZE
    float scaleFactor = size / OLYMPIA_FIXED_FONT_SIZE;
    if (scaleFactor < 0.001)
        scaleFactor = 0.001; // avoid divisions by 0.0
    spec.m_height = OLYMPIA_FIXED_FONT_SIZE;
#else
    float scaleFactor = 1.0; // used in FontPlatformDataPrivate constructor
    spec.m_height = size;
#endif
    spec.m_style = italic
        ? Olympia::Platform::Text::ItalicStyle
        : Olympia::Platform::Text::PlainStyle;
    spec.m_variant = Olympia::Platform::Text::PlainVariant;
    spec.m_weight = bold ? FontWeightBold : FontWeightNormal;

    Olympia::Platform::Text::Font* font = engine()->createFont(error, spec);

    if (error || !font)
        return; // without setting m_private to something non-zero

    Olympia::Platform::Text::FontSpec matchedSpec;
    font->getMatchedFontSpec(matchedSpec);

    String matchedFamily(matchedSpec.m_name, Olympia::Platform::Text::MAX_FONT_NAME);
    matchedFamily = matchedFamily.replace('\0', ' ').stripWhiteSpace();

    m_private = adoptRef(new FontPlatformDataPrivate(font, scaleFactor, matchedSpec, matchedFamily));
}

bool FontPlatformData::operator==(const FontPlatformData& other) const
{
    if (!m_private || !other.m_private || m_private.isHashTableDeletedValue() || other.m_private.isHashTableDeletedValue())
        return m_private == other.m_private;

    return *(m_private->m_font) == *(other.m_private->m_font)
        && m_private->m_scaleFactor == other.m_private->m_scaleFactor;
}

bool FontPlatformData::isValid() const
{
    // If m_font was invalid, m_private hadn't been created in the first place.
    return (bool) m_private;
}

Olympia::Platform::Text::Font* FontPlatformData::font() const
{
    ASSERT(m_private);
    return m_private->m_font;
}

const String& FontPlatformData::fontFamily() const
{
    ASSERT(m_private);
    return m_private->m_matchedFamily;
}

float FontPlatformData::scaleFactor() const
{
    ASSERT(m_private);
    return m_private->m_scaleFactor;
}

unsigned FontPlatformData::hash() const
{
    if (!m_private)
        return 0;

    Olympia::Platform::Text::FontSpec& spec = m_private->m_matchedSpec;

    UChar hashString[Olympia::Platform::Text::MAX_FONT_NAME];
    for (int i = 0; i < Olympia::Platform::Text::MAX_FONT_NAME - 4; ++i)
        hashString[i] = spec.m_name[i];

#if OLYMPIA_FIXED_FONT_SIZE
    hashString[Olympia::Platform::Text::MAX_FONT_NAME - 4] = static_cast<int>(m_private->m_scaleFactor * 1000);
#else
    hashString[Olympia::Platform::Text::MAX_FONT_NAME - 4] = spec.m_height;
#endif
    hashString[Olympia::Platform::Text::MAX_FONT_NAME - 3] = spec.m_weight;
    hashString[Olympia::Platform::Text::MAX_FONT_NAME - 2] = spec.m_style;
    hashString[Olympia::Platform::Text::MAX_FONT_NAME - 1] = spec.m_variant;

    return StringImpl::computeHash(hashString, Olympia::Platform::Text::MAX_FONT_NAME);
}

bool FontPlatformData::isFixedPitch() const
{
    ASSERT(m_private);
    return m_private->m_matchedSpec.m_monospace;
}

bool FontPlatformData::isHashTableDeletedValue() const
{
    return m_private.isHashTableDeletedValue();
}

#ifndef NDEBUG
String FontPlatformData::description() const
{
    ASSERT(m_private);
    return String();
}
#endif

int FontPlatformData::size() const
{
    ASSERT(m_private);
    return m_private->m_matchedSpec.m_height * m_private->m_scaleFactor + 0.9;
}

}
