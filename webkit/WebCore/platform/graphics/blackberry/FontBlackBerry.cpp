/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
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
#include "Font.h"

#include "FloatPoint.h"
#include "FloatRect.h"
#include "FontCache.h"
#include "GraphicsContext.h"
#include "PainterOpenVG.h"
#include "SurfaceOpenVG.h"
#include "AffineTransform.h"
#include "VGUtils.h"

#if PLATFORM(EGL)
#include "EGLDisplayOpenVG.h"
#include "EGLUtils.h"
#endif

#include <OlympiaPlatformText.h>
#include <wtf/MathExtras.h>

namespace WebCore {

static String setupTextDrawing(const Font* font, const TextRun& run, Olympia::Platform::Text::DrawParam* drawParam)
{
    const float scaleFactor = font->primaryFont()->platformData().scaleFactor();

    drawParam->m_textOrder = run.rtl()
        ? Olympia::Platform::Text::ReverseTextOrder
        : Olympia::Platform::Text::NoTextOrder;
    drawParam->m_letterspace.m_value = font->letterSpacing() / scaleFactor;

    if (font->wordSpacing()) {
        drawParam->m_wordspace.m_mode = Olympia::Platform::Text::WordSpaceAdd;
        drawParam->m_wordspace.m_size.m_value = font->wordSpacing() / scaleFactor;
    }

    String sanitized = Font::normalizeSpaces(String(run.characters(), run.length()));

    if (run.padding()) {
        // Justifying characters. Text API doesn't have dedicated
        // functionality for this (yet), so we emulate it for now.
        const UChar* characters = sanitized.characters();
        int gapCount = 0;

        for (int i = 0; i < sanitized.length(); ++i) {
            if (characters[i] == space || characters[i] == zeroWidthSpace)
                ++gapCount;
        }

        // wordSpacing() and justification are mutually exclusive,
        // so we override any previous settings as well.
        if (gapCount) {
            drawParam->m_wordspace.m_mode = Olympia::Platform::Text::WordSpaceAdd;
            drawParam->m_wordspace.m_size.m_value = run.padding() / (gapCount * scaleFactor);
        } else
            drawParam->m_wordspace.m_mode = Olympia::Platform::Text::WordSpaceNatural;
    }

    return sanitized;
}

static void adjustOffsetsForTextDrawing(const TextRun& run, int& from, int& to)
{
    if (run.rtl()) {
        from = run.length() - from;
        to = run.length() - to;
    }
}

void Font::drawComplexText(GraphicsContext* context, const TextRun& run, const FloatPoint& point, int from, int to) const
{
    const FontPlatformData& fontPlatformData = primaryFont()->platformData();
    const float scaleFactor = fontPlatformData.scaleFactor();
    Olympia::Platform::Text::Font* font = fontPlatformData.font(); // FIXME: use fallback fonts as well?
    Olympia::Platform::Text::DrawParam drawParam;

    String sanitized = setupTextDrawing(this, run, &drawParam);
    adjustOffsetsForTextDrawing(run, from, to);

    SurfaceOpenVG* surface = context->platformContext();
    surface->makeCurrent();

    // Before passing control to Text API, make sure we're ok ourselves.
    ASSERT_VG_NO_ERROR();

    PainterOpenVG* painter = surface->activePainter();
    painter->save();
    painter->setStateModified(PainterOpenVG::AllStateCategories);

    Olympia::Platform::Text::GraphicsContext* textContext = FontPlatformData::textGraphicsContext();

#if PLATFORM(EGL)
    textContext->setDisplay(static_cast<Olympia::Platform::Text::NativeGraphicsDisplay>(surface->eglDisplay()));
    textContext->setSurface(static_cast<Olympia::Platform::Text::NativeGraphicsSurface>(surface->eglSurface()));
    textContext->setContext(static_cast<Olympia::Platform::Text::NativeGraphicsContext>(surface->eglContext()));
    ASSERT_EGL_NO_ERROR();
#endif

    if (painter->textDrawingMode() & cTextClip)
        return; // unsupported for every port except CG at the time of writing
    if (!(painter->textDrawingMode() & cTextFill))
        painter->setFillColor(Color::transparent);
    if (!(painter->textDrawingMode() & cTextStroke))
        painter->setStrokeStyle(NoStroke);

    if (from > 0 || to < sanitized.length()) {
        // Clip to the from-to region. We need to draw the full text
        // (and clip) because characters might change appearance when
        // connected to other characters.
        double fromX;
        FontPlatformData::engine()->textPosToX(from, fromX, *font,
            sanitized.characters(), sanitized.length(), drawParam);
        ASSERT_VG_NO_ERROR();
        fromX *= scaleFactor;

        double toX;
        FontPlatformData::engine()->textPosToX(to, toX, *font,
            sanitized.characters(), sanitized.length(), drawParam);
        ASSERT_VG_NO_ERROR();
        toX *= scaleFactor;

        // FIXME: Some glyphs can be higher than primary font's height.
        // We multiply ascent and descent by 4 so the clip rect will be high
        // enough for all glyphs we know. This is safe because we only apply
        // the clip rect when drawing partial strings, and we are only interested
        // in horizontal clip range.
        const float y = point.y() - primaryFont()->ascent() * 4;
        const float height = (primaryFont()->ascent() + primaryFont()->descent()) * 4;
        FloatRect clipRect = (toX > fromX)
            ? FloatRect(point.x() + fromX, y, toX - fromX, height)
            : FloatRect(point.x() + toX, y, fromX - toX, height);

        painter->clipRect(clipRect, PainterOpenVG::IntersectClip);
    }

    AffineTransform transformation = painter->transformation();
    transformation.translate(point.x(), point.y());
    FloatPoint currentTranslation(transformation.e(), transformation.f());
    transformation.setE(0);
    transformation.setF(0);
    transformation.scale(scaleFactor);
    transformation.setE(roundf(currentTranslation.x()));
    transformation.setF(roundf(currentTranslation.y()));
    painter->setTransformation(transformation);

    if (painter->shadowEnabled()) {
        painter->beginDrawShadow();
        FontPlatformData::engine()->drawText(textContext, *font,
            sanitized.characters(), sanitized.length(), 0, 0,
            0 /* no wrap */, &drawParam, 0 /* returned metrics */);
        painter->endDrawShadow();
    }

    FontPlatformData::engine()->drawText(textContext, *font,
        sanitized.characters(), sanitized.length(), 0, 0,
        0 /* no wrap */, &drawParam, 0 /* returned metrics */);

#if PLATFORM(EGL)
    SurfaceOpenVG* sharedSurface = surface->sharedSurface();
    textContext->setDisplay(static_cast<Olympia::Platform::Text::NativeGraphicsDisplay>(sharedSurface->eglDisplay()));
    textContext->setSurface(static_cast<Olympia::Platform::Text::NativeGraphicsSurface>(sharedSurface->eglSurface()));
    textContext->setContext(static_cast<Olympia::Platform::Text::NativeGraphicsContext>(sharedSurface->eglContext()));
    ASSERT_EGL_NO_ERROR();
#endif

    // Let's make sure Text API didn't cause any troubles.
    ASSERT_VG_NO_ERROR();
    painter->restore();
}

float Font::floatWidthForComplexText(const TextRun& run, HashSet<const SimpleFontData*>* fallbackFonts, GlyphOverflow*) const
{
    if (run.length() == 0)
        return run.padding();

    const FontPlatformData& fontPlatformData = primaryFont()->platformData();
    const float scaleFactor = fontPlatformData.scaleFactor();
    Olympia::Platform::Text::Font* font = fontPlatformData.font();
    Olympia::Platform::Text::DrawParam drawParam;

    String sanitized = setupTextDrawing(this, run, &drawParam);

    // WebKit calls us with plain spaces almost half of the time.
    // Exit early with a cached value to avoid lots of calls into Text API.
    if (sanitized.length() == 1 && sanitized[0] == ' ')
        return primaryFont()->spaceWidth();

#if PLATFORM(EGL) // FIXME: remove after Text API fixes shared context handling
    if (eglGetCurrentContext() == EGL_NO_CONTEXT)
        EGLDisplayOpenVG::current()->sharedPlatformSurface()->makeCurrent();
#endif
    Olympia::Platform::Text::TextMetrics metrics;
    FontPlatformData::engine()->drawText(0 /* no drawing, only measuring */,
        *font, sanitized.characters(), sanitized.length(), 0 /*x*/, 0 /*y*/,
        0 /* no wrap */, &drawParam, &metrics);

    if (fallbackFonts && primaryFont()->lineSpacing() < metrics.m_ascent + metrics.m_descent) {
        FontPlatformData tallerFontPlatformData = fontPlatformData;
        tallerFontPlatformData.setLineSpacingOverride(metrics.m_ascent, metrics.m_descent);

        // HACK: We can't generate a cached SimpleFontData from outside of FontCache,
        //   so instead we abuse FontCache::getFontDataForCharacters() to do that.
        //   Safe to use this way because we also pass 0 as string length,
        //   which avoids clashes with possible regular calls of that method.
        fallbackFonts->add(fontCache()->getFontDataForCharacters(*this,
            reinterpret_cast<const UChar*>(&tallerFontPlatformData), 0));
    }

    return metrics.m_linearAdvance * scaleFactor;
}

int Font::offsetForPositionForComplexText(const TextRun& run, float position, bool includePartialGlyphs) const
{
    // In here, we determine the cursor index for an x (mouse-click) position.
    const FontPlatformData& fontPlatformData = primaryFont()->platformData();
    const float scaleFactor = fontPlatformData.scaleFactor();
    Olympia::Platform::Text::Font* font = fontPlatformData.font();
    Olympia::Platform::Text::DrawParam drawParam;

    String sanitized = setupTextDrawing(this, run, &drawParam);

#if PLATFORM(EGL) // FIXME: remove after Text API fixes shared context handling
    if (eglGetCurrentContext() == EGL_NO_CONTEXT)
        EGLDisplayOpenVG::current()->sharedPlatformSurface()->makeCurrent();
#endif
    int offset;
    FontPlatformData::engine()->xToTextPos(position / scaleFactor, offset,
        Olympia::Platform::Text::TextPosRoundNearest, *font,
        sanitized.characters(), sanitized.length(), drawParam);

    return offset;
}

FloatRect Font::selectionRectForComplexText(const TextRun& run, const FloatPoint& point, int height, int from, int to, int width) const
{
    const FontPlatformData& fontPlatformData = primaryFont()->platformData();
    const float scaleFactor = fontPlatformData.scaleFactor();
    Olympia::Platform::Text::Font* font = fontPlatformData.font();
    Olympia::Platform::Text::DrawParam drawParam;

    String sanitized = setupTextDrawing(this, run, &drawParam);
    adjustOffsetsForTextDrawing(run, from, to);

#if PLATFORM(EGL) // FIXME: remove after Text API fixes shared context handling
    if (eglGetCurrentContext() == EGL_NO_CONTEXT)
        EGLDisplayOpenVG::current()->sharedPlatformSurface()->makeCurrent();
#endif
    double fromX;
    FontPlatformData::engine()->textPosToX(from, fromX, *font,
        sanitized.characters(), sanitized.length(), drawParam);
    ASSERT_VG_NO_ERROR();
    fromX *= scaleFactor;

    double toX;
    FontPlatformData::engine()->textPosToX(to, toX, *font,
        sanitized.characters(), sanitized.length(), drawParam);
    ASSERT_VG_NO_ERROR();
    toX *= scaleFactor;

    if (width) {
        double bound = width * scaleFactor;
        if (from == run.length())
            fromX = bound;
        if (to == run.length())
            toX = bound;
    }

    return (toX > fromX)
        ? FloatRect(point.x() + fromX, point.y(), toX - fromX, height)
        : FloatRect(point.x() + toX, point.y(), fromX - toX, height);
}

void Font::drawGlyphs(GraphicsContext* context, const SimpleFontData* font, const GlyphBuffer& glyphBuffer,
                      int from, int numGlyphs, const FloatPoint& point) const
{
    TextRun run(glyphBuffer.glyphs(from), numGlyphs, false, 0, 0, false, false, false, false);
    drawComplexText(context, run, point, 0, numGlyphs);
}

float Font::floatWidthForSimpleText(const TextRun& run, GlyphBuffer*, HashSet<const SimpleFontData*>* fallbackFonts, GlyphOverflow*) const
{
    return floatWidthForComplexText(run);
}

int Font::offsetForPositionForSimpleText(const TextRun& run, float position, bool includePartialGlyphs) const
{
    return offsetForPositionForComplexText(run, position, includePartialGlyphs);
}

FloatRect Font::selectionRectForSimpleText(const TextRun& run, const FloatPoint& point, int height, int from, int to, int width) const
{
    return selectionRectForComplexText(run, point, height, from, to, width);
}

bool Font::canReturnFallbackFontsForComplexText()
{
    return true;
}

}
