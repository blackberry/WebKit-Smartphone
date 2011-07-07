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
#include "GraphicsContext.h"

#include "AffineTransform.h"
#include "Gradient.h"
#include "GraphicsContextPrivate.h"
#include "KURL.h"
#include "NotImplemented.h"
#include "PaintOpenVG.h"
#include "PainterOpenVG.h"
#include "PlatformPathOpenVG.h"
#include "PatternOpenVG.h"
#include "SurfaceOpenVG.h"
#include "TransformationMatrix.h"

#include <wtf/Assertions.h>
#include <wtf/MathExtras.h>
#include <wtf/UnusedParam.h>
#include <wtf/Vector.h>

#if PLATFORM(EGL)
#include "EGLDisplayOpenVG.h"
#include "EGLUtils.h"
#include <egl.h>
#endif

namespace WebCore {

// typedef'ing doesn't work, let's inherit from PainterOpenVG instead
class GraphicsContextPlatformPrivate : public PainterOpenVG {
public:
    GraphicsContextPlatformPrivate(SurfaceOpenVG* surface)
        : PainterOpenVG(surface)
    {
    }
};

GraphicsContext::GraphicsContext(SurfaceOpenVG* surface)
    : m_common(createGraphicsContextPrivate())
    , m_data(surface ? new GraphicsContextPlatformPrivate(surface) : 0)
{
    setPaintingDisabled(!surface);

    if (!m_data)
        return;

    // Align the initial settings in platform data
    m_data->setTextDrawingMode(m_common->state.textDrawingMode);
    m_data->setStrokeStyle(m_common->state.strokeStyle);
    m_data->setStrokeThickness(m_common->state.strokeThickness);
    m_data->setStrokeColor(m_common->state.strokeColor);
    m_data->setFillColor(m_common->state.fillColor);
}

GraphicsContext::~GraphicsContext()
{
    destroyGraphicsContextPrivate(m_common);
    delete m_data;
}

PlatformGraphicsContext* GraphicsContext::platformContext() const
{
    if (paintingDisabled())
        return 0;

    return m_data->baseSurface();
}

AffineTransform GraphicsContext::getCTM() const
{
    if (paintingDisabled())
        return AffineTransform();

    return m_data->transformation();
}

void GraphicsContext::savePlatformState()
{
    if (paintingDisabled())
        return;

    m_data->save();
}

void GraphicsContext::restorePlatformState()
{
    if (paintingDisabled())
        return;

    m_data->restore();
}

void GraphicsContext::drawRect(const IntRect& rect)
{
    if (paintingDisabled())
        return;

    m_data->drawRect(rect);
}

void GraphicsContext::drawLine(const IntPoint& from, const IntPoint& to)
{
    if (paintingDisabled())
        return;

    m_data->drawLine(from, to);
}

/**
 * Draw the largest ellipse that fits into the given rectangle.
 */
void GraphicsContext::drawEllipse(const IntRect& rect)
{
    if (paintingDisabled())
        return;

    m_data->drawEllipse(rect);
}

void GraphicsContext::strokeArc(const IntRect& rect, int startAngle, int angleSpan)
{
    if (paintingDisabled())
        return;

    m_data->drawArc(rect, startAngle, angleSpan, VG_STROKE_PATH);
}

void GraphicsContext::drawConvexPolygon(size_t numPoints, const FloatPoint* points, bool shouldAntialias)
{
    if (paintingDisabled())
        return;

    m_data->drawPolygon(numPoints, points);

    UNUSED_PARAM(shouldAntialias); // FIXME
}

void GraphicsContext::fillPath()
{
    if (paintingDisabled())
        return;

    m_data->drawPath(*m_data->currentPath(), VG_FILL_PATH, m_common->state.fillRule);
    m_data->beginPath();
}

void GraphicsContext::strokePath()
{
    if (paintingDisabled())
        return;

    m_data->drawPath(*m_data->currentPath(), VG_STROKE_PATH, m_common->state.fillRule);
    m_data->beginPath();
}

void GraphicsContext::drawPath()
{
    if (paintingDisabled())
        return;

    m_data->drawPath(*m_data->currentPath(), VG_FILL_PATH | VG_STROKE_PATH, m_common->state.fillRule);
    m_data->beginPath();
}

void GraphicsContext::fillRect(const FloatRect& rect)
{
    if (paintingDisabled())
        return;

    m_data->drawRect(rect, VG_FILL_PATH);
}

void GraphicsContext::fillRect(const FloatRect& rect, const Color& color, ColorSpace colorSpace)
{
    if (paintingDisabled())
        return;

    PaintOpenVG currentPaint = m_data->fillPaint();
    m_data->setFillColor(color);
    m_data->drawRect(rect, VG_FILL_PATH);
    m_data->setFillPaint(currentPaint);

    UNUSED_PARAM(colorSpace); // FIXME
}

void GraphicsContext::fillRoundedRect(const IntRect& rect, const IntSize& topLeft, const IntSize& topRight, const IntSize& bottomLeft, const IntSize& bottomRight, const Color& color, ColorSpace colorSpace)
{
    if (paintingDisabled())
        return;

    PaintOpenVG currentPaint = m_data->fillPaint();
    m_data->setFillColor(color);
    m_data->drawRoundedRect(rect, topLeft, topRight, bottomLeft, bottomRight, VG_FILL_PATH);
    m_data->setFillPaint(currentPaint);

    UNUSED_PARAM(colorSpace); // FIXME
}

void GraphicsContext::beginPath()
{
    if (paintingDisabled())
        return;

    m_data->beginPath();
}

void GraphicsContext::addPath(const Path& path)
{
    if (paintingDisabled())
        return;

    m_data->addPath(path);
}

void GraphicsContext::clip(const FloatRect& rect)
{
    if (paintingDisabled())
        return;

    m_data->clipRect(rect, PainterOpenVG::IntersectClip);
}

void GraphicsContext::clipPath(WindRule clipRule)
{
    if (paintingDisabled())
        return;

    m_data->clipPath(*(m_data->currentPath()), PainterOpenVG::IntersectClip, clipRule);
}

void GraphicsContext::drawFocusRing(const Vector<IntRect>& rects, int width, int offset, const Color& color)
{
    if (paintingDisabled())
        return;

    if (rects.isEmpty())
        return;

    // FIXME: We should outline the edge of the full region.
    PaintOpenVG currentFillPaint = m_data->fillPaint();
    CompositeOperator currentOp = m_data->compositeOperation();
    m_data->setFillColor(0xffa3c8fe);
    m_data->setCompositeOperation(CompositePlusDarker);

    for (unsigned i = 0; i < rects.size(); i++) {
        IntRect focusRect = rects[i];
        focusRect.inflate(2);
        m_data->drawRect(focusRect, VG_FILL_PATH);
    }
    m_data->setFillPaint(currentFillPaint);
    m_data->setCompositeOperation(currentOp);
}

void GraphicsContext::drawFocusRing(const Vector<Path>& paths, int width, int offset, const Color& color)
{
    if (paintingDisabled())
        return;

    if (paths.isEmpty())
        return;

    UNUSED_PARAM(width);
    UNUSED_PARAM(offset);

    StrokeStyle currentStyle = m_data->strokeStyle();
    float currentThickness = m_data->strokeThickness();
    PaintOpenVG currentPaint = m_data->strokePaint();

    m_data->setStrokeStyle(SolidStroke);
    m_data->setStrokeThickness(1);
    m_data->setStrokeColor(color);

    Path unitedPath;

    for (unsigned i = 0; i < paths.size(); i++) {
        vgAppendPath(unitedPath.platformPath()->vgPath(), paths[i].platformPath()->vgPath());
        ASSERT_VG_NO_ERROR();
    }
    m_data->drawPath(unitedPath, VG_STROKE_PATH, RULE_NONZERO);

    m_data->setStrokeStyle(currentStyle);
    m_data->setStrokeThickness(currentThickness);
    m_data->setStrokePaint(currentPaint);
}

void GraphicsContext::drawLineForText(const IntPoint& origin, int width, bool printing)
{
    if (paintingDisabled())
        return;

    if (width <= 0)
        return;

    StrokeStyle oldStyle = m_data->strokeStyle();
    m_data->setStrokeStyle(SolidStroke);
    drawLine(origin, origin + IntSize(width, 0));
    m_data->setStrokeStyle(oldStyle);

    UNUSED_PARAM(printing);
}

// FIXME:  Style should probably be pulled from the render theme or the paint pushed there.
static const RGBA32 spellingUnderline = 0xFFFF0000;
static const RGBA32 grammarUnderline = 0xFF008000;

void GraphicsContext::drawLineForMisspellingOrBadGrammar(const IntPoint& origin, int width, bool grammar)
{
    if (paintingDisabled())
        return;

    if (width <= 0)
        return;

    StrokeStyle currentStyle = m_data->strokeStyle();
    PaintOpenVG currentPaint = m_data->strokePaint();
    m_data->setStrokeStyle(DottedStroke);
    m_data->setStrokeColor(grammar ? grammarUnderline : spellingUnderline);

    drawLine(origin, origin + IntSize(width, 0));

    m_data->setStrokeStyle(currentStyle);
    m_data->setStrokePaint(currentPaint);
}

FloatRect GraphicsContext::roundToDevicePixels(const FloatRect& rect)
{
    if (paintingDisabled())
        return FloatRect();

    return FloatRect(enclosingIntRect(m_data->transformation().mapRect(rect)));
}

void GraphicsContext::setPlatformShadow(const IntSize& size, int blur, const Color& color, ColorSpace colorSpace)
{
    if (paintingDisabled())
        return;

    // FIXME: This still does not address the issue that shadows
    // within canvas elements should ignore transforms.
    if (m_common->state.shadowsIgnoreTransforms)  {
        // Currently only the GraphicsContext associated with the
        // CanvasRenderingContext for HTMLCanvasElement have shadows ignore
        // Transforms. So with this flag set, we know this state is associated
        // with a CanvasRenderingContext. CG uses natural orientation for
        // Y axis, but the HTML5 canvas spec does not. So we now flip the
        // height since it was flipped in CanvasRenderingContext in order to
        // work with CG.
        m_data->setShadow(IntSize(size.width(), -size.height()), blur, color);
    } else
        m_data->setShadow(size, blur, color);

    UNUSED_PARAM(colorSpace); // FIXME
}

void GraphicsContext::clearPlatformShadow()
{
    if (paintingDisabled())
        return;

    m_data->clearShadow();
}

void GraphicsContext::beginTransparencyLayer(float opacity)
{
    if (paintingDisabled())
        return;

    m_data->beginTransparencyLayer(opacity);
}

void GraphicsContext::endTransparencyLayer()
{
    if (paintingDisabled())
        return;

    m_data->endTransparencyLayer();
}

void GraphicsContext::clearRect(const FloatRect& rect)
{
    if (paintingDisabled())
        return;

    CompositeOperator op = m_data->compositeOperation();
    float opacity = m_data->opacity();
    m_data->setCompositeOperation(CompositeClear);
    m_data->setOpacity(1.0);
    m_data->drawRect(rect, VG_FILL_PATH);
    m_data->setOpacity(opacity);
    m_data->setCompositeOperation(op);
}

void GraphicsContext::strokeRect(const FloatRect& rect)
{
    if (paintingDisabled())
        return;

    m_data->drawRect(rect, VG_STROKE_PATH);
}

void GraphicsContext::strokeRect(const FloatRect& rect, float lineWidth)
{
    if (paintingDisabled())
        return;

    float oldThickness = m_data->strokeThickness();
    m_data->setStrokeThickness(lineWidth);
    m_data->drawRect(rect, VG_STROKE_PATH);
    m_data->setStrokeThickness(oldThickness);
}

void GraphicsContext::setLineCap(LineCap lc)
{
    if (paintingDisabled())
        return;

    m_data->setLineCap(lc);
}

void GraphicsContext::setLineDash(const DashArray& dashes, float dashOffset)
{
    if (paintingDisabled())
        return;

    // SVG code sets an empty line dash for solid strokes, don't rely on
    // painter internals to get that one right.
    if (dashes.isEmpty()) {
        m_data->setStrokeStyle(SolidStroke);
        return;
    }
    m_data->setLineDash(dashes, dashOffset);
    m_data->setStrokeStyle(DashedStroke);
}

void GraphicsContext::setLineJoin(LineJoin lj)
{
    if (paintingDisabled())
        return;

    m_data->setLineJoin(lj);
}

void GraphicsContext::setMiterLimit(float limit)
{
    if (paintingDisabled())
        return;

    m_data->setMiterLimit(limit);
}

void GraphicsContext::setAlpha(float opacity)
{
    if (paintingDisabled())
        return;

    m_data->setOpacity(opacity);
}

void GraphicsContext::setCompositeOperation(CompositeOperator op)
{
    if (paintingDisabled())
        return;

    m_data->setCompositeOperation(op);
}

void GraphicsContext::clip(const Path& path)
{
    if (paintingDisabled())
        return;

    m_data->clipPath(path, PainterOpenVG::IntersectClip, m_common->state.fillRule);
}

void GraphicsContext::canvasClip(const Path& path)
{
    clip(path);
}

void GraphicsContext::clipOut(const Path& path)
{
    if (paintingDisabled())
        return;

    m_data->clipPath(path, PainterOpenVG::SubtractClip, m_common->state.fillRule);
}

void GraphicsContext::scale(const FloatSize& scaleFactors)
{
    if (paintingDisabled())
        return;

    m_data->scale(scaleFactors);
}

void GraphicsContext::rotate(float radians)
{
    if (paintingDisabled())
        return;

    m_data->rotate(radians);
}

void GraphicsContext::translate(float dx, float dy)
{
    if (paintingDisabled())
        return;

    m_data->translate(dx, dy);
}

IntPoint GraphicsContext::origin()
{
    if (paintingDisabled())
        return IntPoint();

    AffineTransform transformation = m_data->transformation();
    return IntPoint(roundf(transformation.e()), roundf(transformation.f()));
}

void GraphicsContext::clipOut(const IntRect& rect)
{
    if (paintingDisabled())
        return;

    m_data->clipRect(rect, PainterOpenVG::SubtractClip);
}

void GraphicsContext::clipOutEllipseInRect(const IntRect& rect)
{
    if (paintingDisabled())
        return;

    Path path;
    path.addEllipse(rect);
    m_data->clipPath(path, PainterOpenVG::SubtractClip, m_common->state.fillRule);
}

void GraphicsContext::clipToImageBuffer(const FloatRect& rect, const ImageBuffer* imageBuffer)
{
    if (paintingDisabled())
        return;

    notImplemented();
    UNUSED_PARAM(rect);
    UNUSED_PARAM(imageBuffer);
}

void GraphicsContext::addInnerRoundedRectClip(const IntRect& rect, int thickness)
{
    if (paintingDisabled())
        return;

    Path path;
    path.addEllipse(rect);
    path.addEllipse(FloatRect(rect.x() + thickness, rect.y() + thickness,
        rect.width() - (thickness * 2), rect.height() - (thickness * 2)));

    m_data->clipPath(path, PainterOpenVG::IntersectClip, m_common->state.fillRule);
}

void GraphicsContext::concatCTM(const AffineTransform& transformation)
{
    if (paintingDisabled())
        return;

    m_data->concatTransformation(transformation);
}

void GraphicsContext::setURLForRect(const KURL& link, const IntRect& destRect)
{
    notImplemented();
    UNUSED_PARAM(link);
    UNUSED_PARAM(destRect);
}

void GraphicsContext::setPlatformStrokeColor(const Color& color, ColorSpace colorSpace)
{
    if (paintingDisabled())
        return;

    m_data->setStrokeColor(color);

    UNUSED_PARAM(colorSpace); // FIXME
}

void GraphicsContext::setPlatformStrokeGradient(Gradient* gradient)
{
    if (paintingDisabled())
        return;

    m_data->setStrokeGradient(*(gradient->platformGradient()));
}

void GraphicsContext::setPlatformStrokePattern(Pattern* p)
{
    if (paintingDisabled())
        return;

    PatternOpenVG* platformPattern = p->createPlatformPattern(AffineTransform());
    m_data->setStrokePattern(*platformPattern);
    delete platformPattern;
}

void GraphicsContext::setPlatformStrokeStyle(const StrokeStyle& strokeStyle)
{
    if (paintingDisabled())
        return;

    m_data->setStrokeStyle(strokeStyle);
}

void GraphicsContext::setPlatformStrokeThickness(float thickness)
{
    if (paintingDisabled())
        return;

    m_data->setStrokeThickness(thickness);
}

void GraphicsContext::setPlatformFillColor(const Color& color, ColorSpace colorSpace)
{
    if (paintingDisabled())
        return;

    m_data->setFillColor(color);

    UNUSED_PARAM(colorSpace); // FIXME
}

void GraphicsContext::setPlatformFillGradient(Gradient* gradient)
{
    if (paintingDisabled())
        return;

    m_data->setFillGradient(*(gradient->platformGradient()));
}

void GraphicsContext::setPlatformFillPattern(Pattern* p)
{
    if (paintingDisabled())
        return;

    PatternOpenVG* platformPattern = p->createPlatformPattern(AffineTransform());
    m_data->setFillPattern(*platformPattern);
    delete platformPattern;
}

void GraphicsContext::setPlatformShouldAntialias(bool enable)
{
    if (paintingDisabled())
        return;

    m_data->setAntialiasingEnabled(enable);
}

void GraphicsContext::setImageInterpolationQuality(InterpolationQuality)
{
    notImplemented();
}

InterpolationQuality GraphicsContext::imageInterpolationQuality() const
{
    notImplemented();
    return InterpolationDefault;
}

}
