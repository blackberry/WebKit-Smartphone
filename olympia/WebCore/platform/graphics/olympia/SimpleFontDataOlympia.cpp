/*
 * Copyright (C) Research In Motion Limited 2009. All rights reserved.
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
#include "SimpleFontData.h"

#include "FontDescription.h"
#include "NotImplemented.h"
#include "SurfaceOpenVG.h"
#include "VGUtils.h"

#if PLATFORM(EGL)
#include "EGLDisplayOpenVG.h"
#endif

#include <OlympiaPlatformText.h>

namespace WebCore {

SimpleFontData* SimpleFontData::smallCapsFontData(const FontDescription& desc) const
{
    if (m_smallCapsFontData)
        return m_smallCapsFontData;

    FontDescription smallCapsDesc = desc;
    smallCapsDesc.setSmallCaps(true);

    FontPlatformData smallCapsPlatformFont(smallCapsDesc, platformData().fontFamily());
    m_smallCapsFontData = new SimpleFontData(smallCapsPlatformFont);
    return m_smallCapsFontData;
}

bool SimpleFontData::containsCharacters(const UChar*, int length) const
{
    notImplemented();
    return true;
}

void SimpleFontData::determinePitch()
{
    if (platformData().isValid()) {
        m_treatAsFixedPitch = platformData().isFixedPitch();
    }
}

void SimpleFontData::platformInit()
{
    if (platformData().isValid()) {
        Olympia::Platform::Text::Font* font = platformData().font();
        const float scaleFactor = platformData().scaleFactor();
        ASSERT(font);

        Olympia::Platform::Text::FontMetrics fontMetrics;
        font->getFontMetrics(fontMetrics);

        m_ascent = fontMetrics.m_maxAscent * scaleFactor + 0.9;
        m_descent = fontMetrics.m_maxDescent * scaleFactor + 0.9;
        m_lineGap = (fontMetrics.m_leadingAbove + fontMetrics.m_leadingBelow) * scaleFactor + 0.9;
        m_lineSpacing = m_ascent + m_descent;
        m_maxCharWidth = fontMetrics.m_maxCharWidth * scaleFactor;
        m_unitsPerEm = fontMetrics.m_height * scaleFactor;

        static const Olympia::Platform::Text::Utf16Char characters[] = { 'x' };
        Olympia::Platform::Text::TextMetrics metrics;

#if PLATFORM(EGL) // FIXME: remove after Text API fixes shared context handling
        if (eglGetCurrentContext() == EGL_NO_CONTEXT)
            EGLDisplayOpenVG::current()->sharedPlatformSurface()->makeCurrent();
#endif
        FontPlatformData::engine()->drawText(0 /* no drawing, only measuring */,
            *font, characters, 1 /* number of characters */, 0 /*x*/, 0 /*y*/,
            0 /* no wrap */, 0 /* draw params */, &metrics);

        m_xHeight = (metrics.m_boundsBottom - metrics.m_boundsTop) * scaleFactor;
        m_avgCharWidth = metrics.m_linearAdvance * scaleFactor;
    }
}

void SimpleFontData::platformCharWidthInit()
{
}

void SimpleFontData::platformDestroy()
{
    delete m_smallCapsFontData;
    m_smallCapsFontData = 0;
}

float SimpleFontData::platformWidthForGlyph(Glyph glyph) const
{
    Olympia::Platform::Text::Font* font = platformData().font();
    ASSERT(font);

    const Olympia::Platform::Text::Utf16Char characters[] = { glyph };
    Olympia::Platform::Text::TextMetrics metrics;

#if PLATFORM(EGL) // FIXME: remove after Text API fixes shared context handling
    if (eglGetCurrentContext() == EGL_NO_CONTEXT)
        EGLDisplayOpenVG::current()->sharedPlatformSurface()->makeCurrent();
#endif
    FontPlatformData::engine()->drawText(0 /* no drawing, only measuring */,
        *font, characters, 1 /* number of characters */, 0 /*x*/, 0 /*y*/,
        0 /* no wrap */, 0 /* draw params */, &metrics);

    return metrics.m_linearAdvance * platformData().scaleFactor();
}

#if USE(FONT_FAST_PATH)
FloatRect SimpleFontData::platformBoundsForGlyph(Glyph glyph) const
{
    Olympia::Platform::Text::Font* font = platformData().font();
    ASSERT(font);

    const Olympia::Platform::Text::Utf16Char characters[] = { glyph };
    Olympia::Platform::Text::TextMetrics metrics;

#if PLATFORM(EGL) // FIXME: remove after Text API fixes shared context handling
    if (eglGetCurrentContext() == EGL_NO_CONTEXT)
        EGLDisplayOpenVG::current()->sharedPlatformSurface()->makeCurrent();
#endif
    FontPlatformData::engine()->drawText(0 /* no drawing, only measuring */,
        *font, characters, 1 /* number of characters */, 0 /*x*/, 0 /*y*/,
        0 /* no wrap */, 0 /* draw params */, &metrics);

    const float scaleFactor = platformData().scaleFactor();
    float boundLeft = metrics.m_boundsLeft * scaleFactor;
    float boundTop = metrics.m_boundsTop * scaleFactor;
    float boundRight = metrics.m_boundsRight * scaleFactor;
    float boundBottom = metrics.m_boundsBottom * scaleFactor;
    return FloatRect(boundLeft, boundTop, boundRight - boundLeft, boundBottom - boundTop);
}
#endif

}
