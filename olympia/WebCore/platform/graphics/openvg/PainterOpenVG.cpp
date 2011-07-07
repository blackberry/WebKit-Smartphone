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
#include "PainterOpenVG.h"

#include "AffineTransform.h"
#include "Color.h"
#include "DashArray.h"
#include "FloatPoint.h"
#include "FloatQuad.h"
#include "FloatRect.h"
#include "IntRect.h"
#include "IntSize.h"
#include "NotImplemented.h"
#include "PaintOpenVG.h"
#include "PlatformPathOpenVG.h"
#include "SurfaceOpenVG.h"
#include "TiledImageOpenVG.h"
#include "VGUtils.h"

#if PLATFORM(EGL)
#include "EGLUtils.h"
#endif

#include <vgu.h>

#include <wtf/Assertions.h>
#include <wtf/MathExtras.h>
#include <wtf/RefPtr.h>

namespace WebCore {

static bool isNonRotatedAffineTransformation(const AffineTransform& t)
{
    return t.b() <= FLT_EPSILON && t.c() <= FLT_EPSILON;
}

static VGCapStyle toVGCapStyle(LineCap lineCap)
{
    switch (lineCap) {
    case RoundCap:
        return VG_CAP_ROUND;
    case SquareCap:
        return VG_CAP_SQUARE;
    case ButtCap:
    default:
        return VG_CAP_BUTT;
    }
}

static VGJoinStyle toVGJoinStyle(LineJoin lineJoin)
{
    switch (lineJoin) {
    case RoundJoin:
        return VG_JOIN_ROUND;
    case BevelJoin:
        return VG_JOIN_BEVEL;
    case MiterJoin:
    default:
        return VG_JOIN_MITER;
    }
}

static VGFillRule toVGFillRule(WindRule fillRule)
{
    return fillRule == RULE_EVENODD ? VG_EVEN_ODD : VG_NON_ZERO;
}

struct TransparencyLayer {
    TransparencyLayer(SurfaceOpenVG* surf, VGImage image, const IntRect& absRect, float alpha)
        : surface(surf)
        , surfaceImage(image)
        , rect(absRect)
        , opacity(alpha)
    {
    }

    ~TransparencyLayer()
    {
        delete surface;
        vgDestroyImage(surfaceImage);
        ASSERT_VG_NO_ERROR();
    }

    SurfaceOpenVG* surface;
    VGImage surfaceImage;
    IntRect rect;
    float opacity;
};

struct PlatformPainterState {
    PainterOpenVG::PainterStateCategoryFlags modifiedStateCategories;

    AffineTransform surfaceTransformation;
    CompositeOperator compositeOperation;
    float opacity;

    bool scissoringEnabled;
    FloatRect scissorRect;
#ifdef OPENVG_VERSION_1_1
    bool maskingChangedAndEnabled;
    VGMaskLayer mask;
#endif

    PaintOpenVG fillPaint;
    StrokeStyle strokeStyle;
    PaintOpenVG strokePaint;
    float strokeThickness;
    LineCap strokeLineCap;
    LineJoin strokeLineJoin;
    float strokeMiterLimit;
    DashArray strokeDashArray;
    float strokeDashOffset;

    IntSize shadowOffset;
    float shadowBlur;
    Color shadowColor;

    int textDrawingMode;
    bool antialiasingEnabled;

    PlatformPainterState()
        : modifiedStateCategories(PainterOpenVG::NoStateCategories)
        , compositeOperation(CompositeSourceOver)
        , opacity(1.0)
        , scissoringEnabled(false)
#ifdef OPENVG_VERSION_1_1
        , maskingChangedAndEnabled(false)
        , mask(VG_INVALID_HANDLE)
#endif
        , fillPaint(Color::black)
        , strokeStyle(NoStroke)
        , strokeThickness(0.0)
        , strokeLineCap(ButtCap)
        , strokeLineJoin(MiterJoin)
        , strokeMiterLimit(4.0)
        , strokeDashOffset(0.0)
        , shadowBlur(0.0)
        , textDrawingMode(cTextFill)
        , antialiasingEnabled(true)
    {
    }

    ~PlatformPainterState()
    {
#ifdef OPENVG_VERSION_1_1
        if (maskingChangedAndEnabled && mask != VG_INVALID_HANDLE) {
            vgDestroyMaskLayer(mask);
            ASSERT_VG_NO_ERROR();
            mask = VG_INVALID_HANDLE;
        }
#endif
    }

    PlatformPainterState(const PlatformPainterState& state)
    {
        modifiedStateCategories = PainterOpenVG::NoStateCategories;
        surfaceTransformation = state.surfaceTransformation;

        scissoringEnabled = state.scissoringEnabled;
        scissorRect = state.scissorRect;
#ifdef OPENVG_VERSION_1_1
        maskingChangedAndEnabled = false;
        mask = state.mask;
#endif
        copyPaintState(&state);
    }

    inline bool maskingEnabled()
    {
        return maskingChangedAndEnabled || mask != VG_INVALID_HANDLE;
    }

    void copyPaintState(const PlatformPainterState* other)
    {
        compositeOperation = other->compositeOperation;
        opacity = other->opacity;

        fillPaint = other->fillPaint;
        strokeStyle = other->strokeStyle;
        strokePaint = other->strokePaint;
        strokeThickness = other->strokeThickness;
        strokeLineCap = other->strokeLineCap;
        strokeLineJoin = other->strokeLineJoin;
        strokeMiterLimit = other->strokeMiterLimit;
        strokeDashArray = other->strokeDashArray;
        strokeDashOffset = other->strokeDashOffset;

        shadowOffset = other->shadowOffset;
        shadowBlur = other->shadowBlur;
        shadowColor = other->shadowColor;

        textDrawingMode = other->textDrawingMode;
        antialiasingEnabled = other->antialiasingEnabled;
    }

    void applyState(PainterOpenVG* painter, PainterOpenVG::PainterStateCategoryFlags modifiedStateCategories)
    {
        ASSERT(painter);

        if (modifiedStateCategories & PainterOpenVG::StrokeState) {
            strokePaint.apply(VG_STROKE_PATH, painter->currentSurface()->cachedPaint(SurfaceOpenVG::CachedStrokePaint));
            vgSetf(VG_STROKE_LINE_WIDTH, strokeThickness);
            vgSeti(VG_STROKE_CAP_STYLE, toVGCapStyle(strokeLineCap));
            vgSeti(VG_STROKE_JOIN_STYLE, toVGJoinStyle(strokeLineJoin));
            vgSetf(VG_STROKE_MITER_LIMIT, strokeMiterLimit);
            applyStrokeStyle();
        }

        if (modifiedStateCategories & PainterOpenVG::FillState) {
            fillPaint.apply(VG_FILL_PATH, painter->currentSurface()->cachedPaint(SurfaceOpenVG::CachedFillPaint));
        }

        if (modifiedStateCategories & PainterOpenVG::QualitySettingState) {
            if (antialiasingEnabled)
                vgSeti(VG_RENDERING_QUALITY, VG_RENDERING_QUALITY_FASTER);
            else
                vgSeti(VG_RENDERING_QUALITY, VG_RENDERING_QUALITY_NONANTIALIASED);
        }

        if (modifiedStateCategories & PainterOpenVG::BlendingState)
            applyBlending(painter);

        if (modifiedStateCategories & PainterOpenVG::TransformationState)
            applyTransformation(painter);

        if (modifiedStateCategories & PainterOpenVG::ScissorState)
            applyScissorRect();

#ifdef OPENVG_VERSION_1_1
        if (modifiedStateCategories & PainterOpenVG::MaskState) {
            if (maskingEnabled()) {
                vgSeti(VG_MASKING, VG_TRUE);
                if (mask != VG_INVALID_HANDLE)
                    vgMask(mask, VG_SET_MASK, 0, 0, painter->currentSurface()->width(), painter->currentSurface()->height());
            } else
                vgSeti(VG_MASKING, VG_FALSE);
        }
#endif
        ASSERT_VG_NO_ERROR();
    }

    void applyBlending(PainterOpenVG* painter)
    {
        VGBlendMode blendMode = VG_BLEND_SRC_OVER;

        switch (compositeOperation) {
        case CompositeClear: {
            // Clear means "set to fully transparent regardless of SRC".
            // We implement that by multiplying DST with white color
            // (= no changes) and an alpha of 1.0 - opacity, so the destination
            // pixels will be fully transparent when opacity == 1.0 and
            // unchanged when opacity == 0.0.
            blendMode = VG_BLEND_DST_IN;
            const VGfloat values[] = { 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0 - opacity };
            vgSetfv(VG_COLOR_TRANSFORM_VALUES, 8, values);
            vgSeti(VG_COLOR_TRANSFORM, VG_TRUE);
            ASSERT_VG_NO_ERROR();
            break;
        }
        case CompositeCopy:
            blendMode = VG_BLEND_SRC;
            break;
        case CompositeSourceOver:
            blendMode = VG_BLEND_SRC_OVER;
            break;
        case CompositeSourceIn:
            blendMode = VG_BLEND_SRC_IN;
            break;
        case CompositeSourceOut:
            notImplemented();
            break;
        case CompositeSourceAtop:
            notImplemented();
            break;
        case CompositeDestinationOver:
            blendMode = VG_BLEND_DST_OVER;
            break;
        case CompositeDestinationIn:
            blendMode = VG_BLEND_DST_IN;
            break;
        case CompositeDestinationOut:
            notImplemented();
            break;
        case CompositeDestinationAtop:
            notImplemented();
            break;
        case CompositeXOR:
            notImplemented();
            break;
        case CompositePlusDarker:
            blendMode = VG_BLEND_DARKEN;
            break;
        case CompositeHighlight:
            notImplemented();
            break;
        case CompositePlusLighter:
            blendMode = VG_BLEND_LIGHTEN;
            break;
        }

        if (compositeOperation != CompositeClear) {
            if (opacity >= (1.0 - FLT_EPSILON) && !painter->m_drawingShadow)
                vgSeti(VG_COLOR_TRANSFORM, VG_FALSE);
            else if (blendMode == VG_BLEND_SRC) {
                blendMode = VG_BLEND_SRC_OVER;
                VGfloat values[] = { 1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0, opacity };
                if (painter->m_drawingShadow) {
                    values[0] = 0.0;
                    values[1] = 0.0;
                    values[2] = 0.0;
                    values[4] = shadowColor.red() / 255.0;
                    values[5] = shadowColor.green() / 255.0;
                    values[6] = shadowColor.blue() / 255.0;
                    values[7] *= shadowColor.alpha() / 255.0;
                }
                vgSetfv(VG_COLOR_TRANSFORM_VALUES, 8, values);
                vgSeti(VG_COLOR_TRANSFORM, VG_TRUE);
            } else {
                VGfloat values[] = { 1.0, 1.0, 1.0, opacity, 0.0, 0.0, 0.0, 0.0 };
                if (painter->m_drawingShadow) {
                    values[0] = 0.0;
                    values[1] = 0.0;
                    values[2] = 0.0;
                    values[3] *= shadowColor.alpha() / 255.0;
                    values[4] = shadowColor.red() / 255.0;
                    values[5] = shadowColor.green() / 255.0;
                    values[6] = shadowColor.blue() / 255.0;
                }
                vgSetfv(VG_COLOR_TRANSFORM_VALUES, 8, values);
                vgSeti(VG_COLOR_TRANSFORM, VG_TRUE);
            }
            ASSERT_VG_NO_ERROR();
        }

        vgSeti(VG_BLEND_MODE, blendMode);
        ASSERT_VG_NO_ERROR();
    }

    void applyTransformation(PainterOpenVG* painter)
    {
        // There are *five* separate transforms that can be applied to OpenVG
        // as of 1.1. We use the same "world transformation" for paths, images
        // and glyphs, and only fill/stroke transformations (which are applied
        // before the others take effect) are separate ones.
        VGMatrix vgMatrix(surfaceTransformation);
        const VGfloat* vgFloatArray = vgMatrix.toVGfloat();

        vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
        vgLoadMatrix(vgFloatArray);
        ASSERT_VG_NO_ERROR();

        vgSeti(VG_MATRIX_MODE, VG_MATRIX_IMAGE_USER_TO_SURFACE);
        vgLoadMatrix(vgFloatArray);
        ASSERT_VG_NO_ERROR();

#ifdef OPENVG_VERSION_1_1
        vgSeti(VG_MATRIX_MODE, VG_MATRIX_GLYPH_USER_TO_SURFACE);
        vgLoadMatrix(vgFloatArray);
        ASSERT_VG_NO_ERROR();
#endif
    }

    void applyScissorRect()
    {
        if (scissoringEnabled) {
            vgSeti(VG_SCISSORING, VG_TRUE);
            vgSetfv(VG_SCISSOR_RECTS, 4, VGRect(scissorRect).toVGfloat());
        } else
            vgSeti(VG_SCISSORING, VG_FALSE);

        ASSERT_VG_NO_ERROR();
    }

    void applyStrokeStyle()
    {
        if (strokeStyle == DottedStroke) {
            VGfloat vgFloatArray[2] = { 1.0, 1.0 };
            vgSetfv(VG_STROKE_DASH_PATTERN, 2, vgFloatArray);
            vgSetf(VG_STROKE_DASH_PHASE, 0.0);
        } else if (strokeStyle == DashedStroke) {
            if (!strokeDashArray.size()) {
                VGfloat vgFloatArray[2] = { 4.0, 3.0 };
                vgSetfv(VG_STROKE_DASH_PATTERN, 2, vgFloatArray);
            } else {
                Vector<VGfloat> vgFloatArray(strokeDashArray.size());
                for (int i = 0; i < strokeDashArray.size(); ++i)
                    vgFloatArray[i] = strokeDashArray[i];

                vgSetfv(VG_STROKE_DASH_PATTERN, vgFloatArray.size(), vgFloatArray.data());
            }
            vgSetf(VG_STROKE_DASH_PHASE, strokeDashOffset);
        } else {
            vgSetfv(VG_STROKE_DASH_PATTERN, 0, 0);
            vgSetf(VG_STROKE_DASH_PHASE, 0.0);
        }

        ASSERT_VG_NO_ERROR();
    }

    inline bool strokeDisabled() const
    {
        return strokeStyle == NoStroke || strokeThickness < FLT_EPSILON
            || (compositeOperation == CompositeSourceOver && strokePaint.isEmpty());
    }

    inline bool fillDisabled() const
    {
        return (compositeOperation == CompositeSourceOver && fillPaint.isEmpty());
    }

    void saveMaskIfNecessary(PainterOpenVG* painter)
    {
#ifdef OPENVG_VERSION_1_1
        if (maskingChangedAndEnabled) {
            if (mask != VG_INVALID_HANDLE) {
                vgDestroyMaskLayer(mask);
                ASSERT_VG_NO_ERROR();
            }
            mask = vgCreateMaskLayer(painter->currentSurface()->width(), painter->currentSurface()->height());
            ASSERT(mask != VG_INVALID_HANDLE);
            vgCopyMask(mask, 0, 0, 0, 0, painter->currentSurface()->width(), painter->currentSurface()->height());
            ASSERT_VG_NO_ERROR();
        }
#endif
    }
};

PainterOpenVG::PainterOpenVG()
    : m_state(0)
    , m_baseSurface(0)
    , m_currentSurface(0)
    , m_currentPath(0)
    , m_drawingShadow(false)
{
}

PainterOpenVG::PainterOpenVG(SurfaceOpenVG* surface)
    : m_state(0)
    , m_baseSurface(0)
    , m_currentSurface(0)
    , m_currentPath(0)
    , m_drawingShadow(false)
{
    ASSERT(surface);
    begin(surface);
}

PainterOpenVG::~PainterOpenVG()
{
    end();
    delete m_currentPath;
}

void PainterOpenVG::begin(SurfaceOpenVG* surface)
{
    if (surface == m_baseSurface)
        return;

    ASSERT(surface);
    ASSERT(!m_state);

    m_baseSurface = surface;
    m_currentSurface = m_baseSurface;
    m_drawingShadow = false;

    m_stateStack.append(new PlatformPainterState());
    m_state = m_stateStack.last();

    m_currentSurface->setActivePainter(this);
    m_currentSurface->makeCurrent();
}

void PainterOpenVG::end()
{
    if (!m_baseSurface)
        return;

    destroyTransparencyLayers();

    m_baseSurface->setActivePainter(0);
    m_baseSurface = 0;
    m_currentSurface = 0;

    destroyPainterStates();
}

void PainterOpenVG::setStateModified(PainterStateCategoryFlags modifiedStateCategories)
{
    m_state->modifiedStateCategories |= modifiedStateCategories;
}

void PainterOpenVG::destroyTransparencyLayers()
{
    TransparencyLayer* layer = 0;
    while (!m_layers.isEmpty()) {
        layer = m_layers.last();
        m_layers.removeLast();
        delete layer;
    }
    m_currentSurface = m_baseSurface;
}

void PainterOpenVG::destroyPainterStates()
{
    PlatformPainterState* state = 0;
    while (!m_stateStack.isEmpty()) {
        state = m_stateStack.last();
        m_stateStack.removeLast();
        delete state;
    }
    m_state = 0;
}

// Called by friend SurfaceOpenVG, private otherwise.
void PainterOpenVG::applyState()
{
    ASSERT(m_state);
    m_state->applyState(this, AllStateCategories);
}

/**
 * Copy the current back buffer image onto the surface.
 *
 * Call this method when all painting operations have been completed,
 * otherwise the surface won't visibly change.
 */
void PainterOpenVG::blitToSurface()
{
    ASSERT(m_state); // implies m_baseSurface
    m_baseSurface->flush();
}

void PainterOpenVG::beginTransparencyLayer(float opacity)
{
    static const VGImageFormat surfaceImageFormat = VG_sRGBA_8888_PRE;

    ASSERT(m_state);

    // Retrieve scissor rect and transformation matrix using the offset of the
    // current layer, and apply the new offset later.
    AffineTransform surfaceIndependentTransformation = transformation();
    IntRect layerRect(0, 0, m_currentSurface->width(), m_currentSurface->height());

    if (m_state->scissoringEnabled) {
        layerRect.intersect(enclosingIntRect(m_state->scissorRect));
        if (layerRect.isEmpty())
            layerRect.setSize(IntSize(1, 1));
    }
    if (!m_layers.isEmpty()) {
        const IntRect& currentLayerRect = m_layers.last()->rect;
        layerRect.move(currentLayerRect.x(), currentLayerRect.y());
    }
    ASSERT(!layerRect.isEmpty());

    save(CreateNewStateWithPaintStateOnly);

    VGImage surfaceImage = vgCreateImage(surfaceImageFormat,
        layerRect.width(), layerRect.height(), VG_IMAGE_QUALITY_FASTER);
    ASSERT_VG_NO_ERROR();

#if PLATFORM(EGL)
    SurfaceOpenVG* surface = new SurfaceOpenVG((EGLClientBuffer*) surfaceImage, EGL_OPENVG_IMAGE, m_baseSurface->eglDisplay());
    if (surface->eglContext() != m_currentSurface->eglContext())
        m_state->modifiedStateCategories = AllStateCategories;
#else
    m_state->modifiedStateCategories = AllStateCategories;
#endif
    surface->setActivePainter(this);
    TransparencyLayer* layer = new TransparencyLayer(surface, surfaceImage, layerRect, opacity);
    m_layers.append(layer);

    m_currentSurface = surface;
    m_currentSurface->makeCurrent(SurfaceOpenVG::DontSaveOrApplyPainterState);
    m_state->applyState(this, m_state->modifiedStateCategories);

    // Clear the surface from whatever color was set before to transparent.
    // No need to turn off scissoring, it's already disabled in our new state.
    const VGfloat transparent[4] = { 0.0, 0.0, 0.0, 0.0 };
    vgSetfv(VG_CLEAR_COLOR, 4, transparent);
    vgClear(0, 0, layer->rect.width(), layer->rect.height());

    // Apply offset to the transformation.
    setTransformation(surfaceIndependentTransformation);
}

void PainterOpenVG::endTransparencyLayer()
{
    endTransparencyLayer(0);
}

void PainterOpenVG::endTransparencyLayer(int blurRadius)
{
    ASSERT(m_state);
    ASSERT(!m_layers.isEmpty());

    TransparencyLayer* layer = m_layers.last();
    IntRect layerRectAtZero(IntPoint(0, 0), layer->rect.size());
    m_layers.removeLast();
    layer->surface->setActivePainter(0);

    // Make the underlying surface current so that we can access the
    // surface image of our transparency layer on top.
    m_currentSurface = m_layers.isEmpty() ? m_baseSurface : m_layers.last()->surface;
    restore();

    if (blurRadius) {
        VGImageFormat format = (VGImageFormat) vgGetParameteri(layer->surfaceImage, VG_IMAGE_FORMAT);
        VGImage blurredTileImage = vgCreateImage(format, layer->rect.width(), layer->rect.height(), VG_IMAGE_QUALITY_FASTER);
        ASSERT_VG_NO_ERROR();

        setFillColor(Color::transparent);
        vgGaussianBlur(blurredTileImage, layer->surfaceImage, blurRadius, blurRadius, VG_TILE_FILL);
        ASSERT_VG_NO_ERROR();

        vgDestroyImage(layer->surfaceImage);
        ASSERT_VG_NO_ERROR();

        layer->surfaceImage = blurredTileImage;
    }

    AffineTransform originalTransformation = m_state->surfaceTransformation;
    CompositeOperator op = m_state->compositeOperation;
    float opacity = m_state->opacity;
    Color shadowColor = m_state->shadowColor;

    m_state->compositeOperation = CompositeSourceOver;
    m_state->opacity = layer->opacity;
    m_state->shadowColor = Color(); // make sure not to try to draw any shadows
    m_state->applyBlending(this);
    setSurfaceTransformation(AffineTransform());

    if (m_currentSurface != m_baseSurface) {
        const IntRect& lowerRect = m_layers.last()->rect;
        layer->rect.move(-lowerRect.x(), -lowerRect.y());
    }
    translate(layer->rect.x(), layer->rect.y());
    vgDrawImage(layer->surfaceImage);

    setSurfaceTransformation(originalTransformation);
    m_state->compositeOperation = op;
    m_state->opacity = opacity;
    m_state->shadowColor = shadowColor;
    m_state->applyBlending(this);

    delete layer;
}

AffineTransform PainterOpenVG::transformation() const
{
    ASSERT(m_state);

    if (m_currentSurface == m_baseSurface) // a.k.a. m_layers.isEmpty()
        return m_state->surfaceTransformation;

    AffineTransform transformationWithoutOffset;
    const IntRect& layerRect = m_layers.last()->rect;
    transformationWithoutOffset.translate(layerRect.x(), layerRect.y());
    transformationWithoutOffset.multLeft(m_state->surfaceTransformation);
    return transformationWithoutOffset;
}

void PainterOpenVG::setTransformation(const AffineTransform& transformation)
{
    if (m_currentSurface == m_baseSurface) // a.k.a. m_layers.isEmpty()
        setSurfaceTransformation(transformation);
    else {
        // If a transparency layer surface is put on top of the base surface,
        // take the layer offset into account so the transformation results in
        // correct rendering for the layer surface.
        AffineTransform transformationWithOffset;
        const IntRect& layerRect = m_layers.last()->rect;
        transformationWithOffset.translate(-layerRect.x(), -layerRect.y());
        transformationWithOffset.multLeft(transformation);
        setSurfaceTransformation(transformationWithOffset);
    }
}

void PainterOpenVG::concatTransformation(const AffineTransform& transformation)
{
    ASSERT(m_state);
    m_state->modifiedStateCategories |= TransformationState;
    m_currentSurface->makeCurrent();

    // We do the multiplication ourself using WebCore's AffineTransform rather
    // than offloading this to VG via vgMultMatrix() to keep things simple and
    // so we can maintain state ourselves.
    m_state->surfaceTransformation.multLeft(transformation);
    m_state->applyTransformation(this);
}

void PainterOpenVG::setSurfaceTransformation(const AffineTransform& transformation)
{
    ASSERT(m_state);
    m_state->modifiedStateCategories |= TransformationState;
    m_currentSurface->makeCurrent();

    m_state->surfaceTransformation = transformation;
    m_state->applyTransformation(this);
}

void PainterOpenVG::transformPath(VGPath dst, VGPath src, const AffineTransform& transformation)
{
    vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);

    // Save the transform state
    VGfloat currentMatrix[9];
    vgGetMatrix(currentMatrix);
    ASSERT_VG_NO_ERROR();

    // Load the new transform
    vgLoadMatrix(VGMatrix(transformation).toVGfloat());
    ASSERT_VG_NO_ERROR();

    // Apply the new transform
    vgTransformPath(dst, src);
    ASSERT_VG_NO_ERROR();

    // Restore the transform state
    vgLoadMatrix(currentMatrix);
    ASSERT_VG_NO_ERROR();
}

CompositeOperator PainterOpenVG::compositeOperation() const
{
    ASSERT(m_state);
    return m_state->compositeOperation;
}

void PainterOpenVG::setCompositeOperation(CompositeOperator op)
{
    ASSERT(m_state);

    if (op == m_state->compositeOperation)
        return;

    m_state->modifiedStateCategories |= BlendingState;
    m_currentSurface->makeCurrent();

    m_state->compositeOperation = op;
    m_state->applyBlending(this);
}

float PainterOpenVG::opacity() const
{
    ASSERT(m_state);
    return m_state->opacity;
}

void PainterOpenVG::setOpacity(float opacity)
{
    ASSERT(m_state);

    if (opacity == m_state->opacity)
        return;

    m_state->modifiedStateCategories |= BlendingState;
    m_currentSurface->makeCurrent();

    m_state->opacity = opacity;
    m_state->applyBlending(this);
}

float PainterOpenVG::strokeThickness() const
{
    ASSERT(m_state);
    return m_state->strokeThickness;
}

void PainterOpenVG::setStrokeThickness(float thickness)
{
    ASSERT(m_state);
    m_state->modifiedStateCategories |= StrokeState;
    m_currentSurface->makeCurrent();

    m_state->strokeThickness = thickness;
    vgSetf(VG_STROKE_LINE_WIDTH, thickness);
    ASSERT_VG_NO_ERROR();
}

StrokeStyle PainterOpenVG::strokeStyle() const
{
    ASSERT(m_state);
    return m_state->strokeStyle;
}

void PainterOpenVG::setStrokeStyle(const StrokeStyle& style)
{
    ASSERT(m_state);
    m_state->modifiedStateCategories |= StrokeState;
    m_currentSurface->makeCurrent();

    m_state->strokeStyle = style;
    m_state->applyStrokeStyle();
}

void PainterOpenVG::setLineDash(const DashArray& dashArray, float dashOffset)
{
    ASSERT(m_state);

    m_state->modifiedStateCategories |= StrokeState;
    m_currentSurface->makeCurrent();

    m_state->strokeDashArray = dashArray;
    m_state->strokeDashOffset = dashOffset;
    m_state->applyStrokeStyle();
}

void PainterOpenVG::setLineCap(LineCap lineCap)
{
    ASSERT(m_state);
    m_state->modifiedStateCategories |= StrokeState;
    m_currentSurface->makeCurrent();

    m_state->strokeLineCap = lineCap;
    vgSeti(VG_STROKE_CAP_STYLE, toVGCapStyle(lineCap));
    ASSERT_VG_NO_ERROR();
}

void PainterOpenVG::setLineJoin(LineJoin lineJoin)
{
    ASSERT(m_state);
    m_state->modifiedStateCategories |= StrokeState;
    m_currentSurface->makeCurrent();

    m_state->strokeLineJoin = lineJoin;
    vgSeti(VG_STROKE_JOIN_STYLE, toVGJoinStyle(lineJoin));
    ASSERT_VG_NO_ERROR();
}

void PainterOpenVG::setMiterLimit(float miterLimit)
{
    ASSERT(m_state);
    m_state->modifiedStateCategories |= StrokeState;
    m_currentSurface->makeCurrent();

    m_state->strokeMiterLimit = miterLimit;
    vgSetf(VG_STROKE_MITER_LIMIT, miterLimit);
    ASSERT_VG_NO_ERROR();
}

const PaintOpenVG& PainterOpenVG::strokePaint() const
{
    ASSERT(m_state);
    return m_state->strokePaint;
}

void PainterOpenVG::setStrokePaint(const PaintOpenVG& paint)
{
    ASSERT(m_state);

    m_state->modifiedStateCategories |= StrokeState;
    m_currentSurface->makeCurrent();

    m_state->strokePaint = paint;
    m_state->strokePaint.apply(VG_FILL_PATH, m_currentSurface->cachedPaint(SurfaceOpenVG::CachedStrokePaint));
}

void PainterOpenVG::setStrokeColor(const Color& color)
{
    if (!color.isValid()) {
        setStrokeColor(Color::transparent);
        return;
    }

    ASSERT(m_state);
    m_state->modifiedStateCategories |= StrokeState;
    m_currentSurface->makeCurrent();

    m_state->strokePaint.setColor(color);
    m_state->strokePaint.apply(VG_STROKE_PATH, m_currentSurface->cachedPaint(SurfaceOpenVG::CachedStrokePaint));
}

void PainterOpenVG::setStrokeGradient(const GradientOpenVG& gradient)
{
    if (gradient.isEmpty()) {
        // See http://www.w3.org/TR/2dcontext/#colors-and-styles:
        // "When there are no stops, the gradient is transparent black."
        setStrokeColor(Color::transparent);
        return;
    }

    ASSERT(m_state);

    m_state->modifiedStateCategories |= StrokeState;
    m_currentSurface->makeCurrent();

    m_state->strokePaint.setGradient(gradient);
    m_state->strokePaint.apply(VG_STROKE_PATH, m_currentSurface->cachedPaint(SurfaceOpenVG::CachedStrokePaint));
}

void PainterOpenVG::setStrokePattern(const PatternOpenVG& pattern)
{
    if (!pattern.isValid()) {
        setStrokeColor(Color::transparent);
        return;
    }

    ASSERT(m_state);

    m_state->modifiedStateCategories |= StrokeState;
    m_currentSurface->makeCurrent();

    m_state->strokePaint.setPattern(pattern);
    m_state->strokePaint.apply(VG_STROKE_PATH, m_currentSurface->cachedPaint(SurfaceOpenVG::CachedStrokePaint));
}

const PaintOpenVG& PainterOpenVG::fillPaint() const
{
    ASSERT(m_state);
    return m_state->fillPaint;
}

void PainterOpenVG::setFillPaint(const PaintOpenVG& paint)
{
    ASSERT(m_state);

    m_state->modifiedStateCategories |= FillState;
    m_currentSurface->makeCurrent();

    m_state->fillPaint = paint;
    m_state->fillPaint.apply(VG_FILL_PATH, m_currentSurface->cachedPaint(SurfaceOpenVG::CachedFillPaint));
}

void PainterOpenVG::setFillColor(const Color& color)
{
    if (!color.isValid()) {
        setFillColor(Color::transparent);
        return;
    }

    ASSERT(m_state);
    m_state->modifiedStateCategories |= FillState;
    m_currentSurface->makeCurrent();

    m_state->fillPaint.setColor(color);
    m_state->fillPaint.apply(VG_FILL_PATH, m_currentSurface->cachedPaint(SurfaceOpenVG::CachedFillPaint));
}

void PainterOpenVG::setFillGradient(const GradientOpenVG& gradient)
{
    if (gradient.isEmpty()) {
        // See http://www.w3.org/TR/2dcontext/#colors-and-styles:
        // "When there are no stops, the gradient is transparent black."
        setFillColor(Color::transparent);
        return;
    }

    ASSERT(m_state);

    m_state->modifiedStateCategories |= FillState;
    m_currentSurface->makeCurrent();

    m_state->fillPaint.setGradient(gradient);
    m_state->fillPaint.apply(VG_FILL_PATH, m_currentSurface->cachedPaint(SurfaceOpenVG::CachedFillPaint));
}

void PainterOpenVG::setFillPattern(const PatternOpenVG& pattern)
{
    if (!pattern.isValid()) {
        setFillColor(Color::transparent);
        return;
    }

    ASSERT(m_state);

    m_state->modifiedStateCategories |= FillState;
    m_currentSurface->makeCurrent();

    m_state->fillPaint.setPattern(pattern);
    m_state->fillPaint.apply(VG_FILL_PATH, m_currentSurface->cachedPaint(SurfaceOpenVG::CachedFillPaint));
}

bool PainterOpenVG::shadowEnabled() const
{
    ASSERT(m_state);
    return m_state->shadowColor.isValid();
}

void PainterOpenVG::setShadow(const IntSize& offset, float blur, const Color& color)
{
    ASSERT(m_state);

    if (color.isValid() && color.alpha() && (blur > FLT_EPSILON || offset.width() || offset.height())) {
        m_state->shadowOffset = offset;
        m_state->shadowBlur = blur;
        m_state->shadowColor = color;
    } else
        clearShadow();
}

void PainterOpenVG::clearShadow()
{
    m_state->shadowOffset = IntSize();
    m_state->shadowBlur = 0.0;
    m_state->shadowColor = Color();
}

void PainterOpenVG::beginDrawShadow()
{
    ASSERT(shadowEnabled());

#if USE(OPENVG_BLUR)
    if (m_state->shadowBlur > FLT_EPSILON)
        beginTransparencyLayer(1.0);
    else
#endif
        save();

    m_drawingShadow = true;
    m_state->modifiedStateCategories |= BlendingState;
    m_state->applyBlending(this);
    translate(m_state->shadowOffset.width(), m_state->shadowOffset.height());
}

void PainterOpenVG::endDrawShadow()
{
    ASSERT(shadowEnabled());
    m_drawingShadow = false;

#if USE(OPENVG_BLUR)
    if (m_state->shadowBlur > FLT_EPSILON)
        endTransparencyLayer(static_cast<int>((m_state->shadowBlur + 1) / 2));
    else
#endif
        restore();
}

int PainterOpenVG::textDrawingMode() const
{
    ASSERT(m_state);
    return m_state->textDrawingMode;
}

void PainterOpenVG::setTextDrawingMode(int mode)
{
    ASSERT(m_state);
    m_state->textDrawingMode = mode;
}

bool PainterOpenVG::antialiasingEnabled() const
{
    ASSERT(m_state);
    return m_state->antialiasingEnabled;
}

void PainterOpenVG::setAntialiasingEnabled(bool enabled)
{
    ASSERT(m_state);
    m_state->modifiedStateCategories |= QualitySettingState;
    m_currentSurface->makeCurrent();

    m_state->antialiasingEnabled = enabled;

    if (enabled)
        vgSeti(VG_RENDERING_QUALITY, VG_RENDERING_QUALITY_FASTER);
    else
        vgSeti(VG_RENDERING_QUALITY, VG_RENDERING_QUALITY_NONANTIALIASED);
}

void PainterOpenVG::scale(const FloatSize& scaleFactors)
{
    ASSERT(m_state);
    m_currentSurface->makeCurrent();

    AffineTransform transformation = m_state->surfaceTransformation;
    transformation.scaleNonUniform(scaleFactors.width(), scaleFactors.height());
    setSurfaceTransformation(transformation);
}

void PainterOpenVG::rotate(float radians)
{
    ASSERT(m_state);
    m_currentSurface->makeCurrent();

    AffineTransform transformation = m_state->surfaceTransformation;
    transformation.rotate(rad2deg(radians));
    setSurfaceTransformation(transformation);
}

void PainterOpenVG::translate(float dx, float dy)
{
    ASSERT(m_state);
    m_currentSurface->makeCurrent();

    AffineTransform transformation = m_state->surfaceTransformation;
    transformation.translate(dx, dy);
    setSurfaceTransformation(transformation);
}

void PainterOpenVG::beginPath()
{
    if (!m_currentPath) {
        m_currentPath = new Path();
        return;
    }
    m_currentPath->clear();
}

void PainterOpenVG::addPath(const Path& path)
{
    addPath(path.platformPath()->vgPath());
}

void PainterOpenVG::addPath(VGPath path)
{
    m_currentPath->platformPath()->makeCompatibleContextCurrent();

    vgAppendPath(m_currentPath->platformPath()->vgPath(), path);
    ASSERT_VG_NO_ERROR();
}

Path* PainterOpenVG::currentPath() const
{
    return m_currentPath;
}

void PainterOpenVG::drawPath(const Path& path, VGbitfield specifiedPaintModes, WindRule fillRule)
{
    drawPath(path.platformPath()->vgPath(), specifiedPaintModes, fillRule);
}

void PainterOpenVG::drawPath(VGPath path, VGbitfield specifiedPaintModes, WindRule fillRule)
{
    ASSERT(m_state);

    VGbitfield paintModes = 0;
    if (!m_state->strokeDisabled())
        paintModes |= VG_STROKE_PATH;
    if (!m_state->fillDisabled())
        paintModes |= VG_FILL_PATH;

    paintModes &= specifiedPaintModes;

    if (!paintModes)
        return;

    if (shadowEnabled() && !m_drawingShadow) {
        beginDrawShadow();
        drawPath(path, paintModes, fillRule);
        endDrawShadow();
    } else
        m_currentSurface->makeCurrent();

    vgSeti(VG_FILL_RULE, toVGFillRule(fillRule));
    vgDrawPath(path, paintModes);
    ASSERT_VG_NO_ERROR();
}

void PainterOpenVG::intersectScissorRect(const FloatRect& rect)
{
    // Scissor rectangles are defined by float values, but e.g. painting
    // something red to a float-clipped rectangle and then painting something
    // white to the same rectangle will leave some red remnants as it is
    // rendered to full pixels in between. Also, some OpenVG implementations
    // are likely to clip to integer coordinates anyways because of the above
    // effect. So considering the above (and confirming through tests) the
    // visual result is better if we clip to the enclosing integer rectangle
    // rather than the exact float rectangle for scissoring.
    FloatRect intersectingRect = enclosingIntRect(rect);

    if (m_state->scissoringEnabled) {
        if (intersectingRect.contains(m_state->scissorRect))
            return;

        m_state->scissorRect.intersect(intersectingRect);
    } else {
        m_state->scissoringEnabled = true;
        m_state->scissorRect = intersectingRect;
    }

    m_state->modifiedStateCategories |= ScissorState;
    m_state->applyScissorRect();
}

void PainterOpenVG::clipRect(const FloatRect& rect, PainterOpenVG::ClipOperation maskOp)
{
    ASSERT(m_state);
    m_currentSurface->makeCurrent();

    if (maskOp == PainterOpenVG::SubtractClip) {
        AffineTransform localTransformation;
        localTransformation.setA(rect.width());
        localTransformation.setD(rect.height());
        localTransformation.setE(rect.x());
        localTransformation.setF(rect.y());

        AffineTransform transformation = m_state->surfaceTransformation;
        transformation.multLeft(localTransformation);

        Vector<VGfloat> currentMatrix(9);
        vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
        vgGetMatrix(currentMatrix.data());
        ASSERT_VG_NO_ERROR();

        vgLoadMatrix(VGMatrix(transformation).toVGfloat());
        ASSERT_VG_NO_ERROR();

        clipPath(m_currentSurface->cachedPath(SurfaceOpenVG::CachedRectPath), PainterOpenVG::SubtractClip);

        vgLoadMatrix(currentMatrix.data());
        ASSERT_VG_NO_ERROR();
        return;
    }

    if (m_state->surfaceTransformation.isIdentity()) {
        // No transformation required, skip all the complex stuff.
        intersectScissorRect(rect);
        return;
    }

    // Check if the actual destination rectangle is still rectilinear (can be
    // represented as FloatRect) so we could apply scissoring instead of
    // (potentially more expensive) path clipping. Note that scissoring is not
    // subject to transformations, so we need to do the transformation to
    // surface coordinates by ourselves.
    FloatQuad effectiveScissorQuad = m_state->surfaceTransformation.mapQuad(FloatQuad(rect));

    if (effectiveScissorQuad.isRectilinear())
        intersectScissorRect(effectiveScissorQuad.boundingBox());
    else {
        // The transformed scissorRect cannot be represented as FloatRect
        // anymore, so we need to perform masking instead.
        Path scissorRectPath;
        scissorRectPath.addRect(rect);
        clipPath(scissorRectPath, PainterOpenVG::IntersectClip);
    }
}

void PainterOpenVG::clipPath(const Path& path, PainterOpenVG::ClipOperation maskOp, WindRule clipRule)
{
    clipPath(path.platformPath()->vgPath(), maskOp, clipRule);
}

void PainterOpenVG::clipPath(VGPath path, PainterOpenVG::ClipOperation maskOp, WindRule clipRule)
{
#ifdef OPENVG_VERSION_1_1
    ASSERT(m_state);

    m_state->modifiedStateCategories |= MaskState;
    m_currentSurface->makeCurrent();

    if (m_state->mask != VG_INVALID_HANDLE && !m_state->maskingChangedAndEnabled) {
        // The parent's mask has been inherited - dispose the handle so that
        // it won't be overwritten.
        m_state->maskingChangedAndEnabled = true;
        m_state->mask = VG_INVALID_HANDLE;
    } else if (!m_state->maskingEnabled()) {
        // None of the parent painter states had a mask enabled yet.
        m_state->maskingChangedAndEnabled = true;
        vgSeti(VG_MASKING, VG_TRUE);
        // Make sure not to inherit previous mask state from previously written
        // (but disabled) masks. For VG_FILL_MASK the first argument is ignored,
        // we pass VG_INVALID_HANDLE which is what the OpenVG spec suggests.
        vgMask(VG_INVALID_HANDLE, VG_FILL_MASK, 0, 0, m_currentSurface->width(), m_currentSurface->height());
    }

    // Intersect the path from the mask, or subtract it from there.
    // (In either case we always decrease the visible area, never increase it,
    // which means masking never has to modify scissor rectangles.)
    vgSeti(VG_FILL_RULE, toVGFillRule(clipRule));
    vgRenderToMask(path, VG_FILL_PATH, (VGMaskOperation) maskOp);
    ASSERT_VG_NO_ERROR();
#else
    notImplemented();
#endif
}

void PainterOpenVG::drawRect(const FloatRect& rect, VGbitfield specifiedPaintModes)
{
    ASSERT(m_state);

    if (rect.isEmpty())
        return;

    VGbitfield paintModes = 0;
    if (!m_state->strokeDisabled())
        paintModes |= VG_STROKE_PATH;
    if (!m_state->fillDisabled())
        paintModes |= VG_FILL_PATH;

    paintModes &= specifiedPaintModes;

    if (!paintModes)
        return;

    if (shadowEnabled() && !m_drawingShadow) {
        beginDrawShadow();

        // Improve draw performance for shadows of patterns and gradients.
        if (m_state->compositeOperation == CompositeCopy
            && m_state->fillPaint.type() != PaintOpenVG::ColorPaint) {
            // Fill with any opaque color, the color transformation does the rest.
            setFillColor(Color::black);
        }

        drawRect(rect, specifiedPaintModes);
        endDrawShadow();
    } else
        m_currentSurface->makeCurrent();

    // If possible, transform the cached rectangle path. Creating paths
    // in OpenVG is pretty expensive, whereas changing transformations to draw
    // our cached 1x1 rectangle at the right coordinates... not so much.
    if (!(paintModes & VG_STROKE_PATH) && m_state->fillPaint.type() == PaintOpenVG::ColorPaint) {
        AffineTransform localTransformation;
        localTransformation.setA(rect.width());
        localTransformation.setD(rect.height());
        localTransformation.setE(rect.x());
        localTransformation.setF(rect.y());

        // Make sure rects don't disappear when zooming out a lot.
        if (rect.width() * fabs(m_state->surfaceTransformation.a()) < 1.0)
            localTransformation.setA(1.0 / fabs(m_state->surfaceTransformation.a()));
        if (rect.height() * fabs(m_state->surfaceTransformation.d()) < 1.0)
            localTransformation.setD(1.0 / fabs(m_state->surfaceTransformation.d()));

        AffineTransform transformation = m_state->surfaceTransformation;
        transformation.multLeft(localTransformation);

        Vector<VGfloat> currentMatrix(9);
        vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
        vgGetMatrix(currentMatrix.data());
        ASSERT_VG_NO_ERROR();

        vgLoadMatrix(VGMatrix(transformation).toVGfloat());
        ASSERT_VG_NO_ERROR();

        vgDrawPath(m_currentSurface->cachedPath(SurfaceOpenVG::CachedRectPath), paintModes);
        ASSERT_VG_NO_ERROR();

        vgLoadMatrix(currentMatrix.data());
        ASSERT_VG_NO_ERROR();
    } else {
        VGPath path = vgCreatePath(
            VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F,
            1.0 /* scale */, 0.0 /* bias */,
            5 /* expected number of segments */,
            5 /* expected number of total coordinates */,
            VG_PATH_CAPABILITY_APPEND_TO);
        ASSERT_VG_NO_ERROR();

        if (vguRect(path, rect.x(), rect.y(), rect.width(), rect.height()) == VGU_NO_ERROR) {
            vgDrawPath(path, paintModes);
            ASSERT_VG_NO_ERROR();
        }

        vgDestroyPath(path);
        ASSERT_VG_NO_ERROR();
    }
}

void PainterOpenVG::drawRoundedRect(const FloatRect& rect, const IntSize& topLeft, const IntSize& topRight, const IntSize& bottomLeft, const IntSize& bottomRight, VGbitfield specifiedPaintModes)
{
    ASSERT(m_state);

    VGbitfield paintModes = 0;
    if (!m_state->strokeDisabled())
        paintModes |= VG_STROKE_PATH;
    if (!m_state->fillDisabled())
        paintModes |= VG_FILL_PATH;

    paintModes &= specifiedPaintModes;

    if (!paintModes)
        return;

    if (shadowEnabled() && !m_drawingShadow) {
        beginDrawShadow();
        drawRect(rect, specifiedPaintModes);
        endDrawShadow();
    } else
        m_currentSurface->makeCurrent();

    VGPath path = vgCreatePath(
        VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F,
        1.0 /* scale */, 0.0 /* bias */,
        10 /* expected number of segments */,
        25 /* expected number of total coordinates */,
        VG_PATH_CAPABILITY_APPEND_TO);
    ASSERT_VG_NO_ERROR();

    // clamp corner arc sizes
    FloatSize clampedTopLeft = FloatSize(topLeft).shrunkTo(rect.size()).expandedTo(FloatSize());
    FloatSize clampedTopRight = FloatSize(topRight).shrunkTo(rect.size()).expandedTo(FloatSize());
    FloatSize clampedBottomLeft = FloatSize(bottomLeft).shrunkTo(rect.size()).expandedTo(FloatSize());
    FloatSize clampedBottomRight = FloatSize(bottomRight).shrunkTo(rect.size()).expandedTo(FloatSize());

    // As OpenVG's coordinate system is flipped in comparison to WebKit's,
    // we have to specify the opposite value for the "clockwise" value.
    static const VGubyte pathSegments[] = {
        VG_MOVE_TO_ABS,
        VG_HLINE_TO_REL,
        VG_SCCWARC_TO_REL,
        VG_VLINE_TO_REL,
        VG_SCCWARC_TO_REL,
        VG_HLINE_TO_REL,
        VG_SCCWARC_TO_REL,
        VG_VLINE_TO_REL,
        VG_SCCWARC_TO_REL,
        VG_CLOSE_PATH
    };
    // Also, the rounded rectangle path proceeds from the top to the bottom,
    // requiring height distances and clamped radius sizes to be flipped.
    const VGfloat pathData[] = {
        rect.x() + clampedTopLeft.width(), rect.y(),
        rect.width() - clampedTopLeft.width() - clampedTopRight.width(),
        clampedTopRight.width(), clampedTopRight.height(), 0, clampedTopRight.width(), clampedTopRight.height(),
        rect.height() - clampedTopRight.height() - clampedBottomRight.height(),
        clampedBottomRight.width(), clampedBottomRight.height(), 0, -clampedBottomRight.width(), clampedBottomRight.height(),
        -(rect.width() - clampedBottomLeft.width() - clampedBottomRight.width()),
        clampedBottomLeft.width(), clampedBottomLeft.height(), 0, -clampedBottomLeft.width(), -clampedBottomLeft.height(),
        -(rect.height() - clampedTopLeft.height() - clampedBottomLeft.height()),
        clampedTopLeft.width(), clampedTopLeft.height(), 0, clampedTopLeft.width(), -clampedTopLeft.height(),
    };

    vgAppendPathData(path, 10, pathSegments, pathData);
    vgDrawPath(path, paintModes);
    vgDestroyPath(path);
    ASSERT_VG_NO_ERROR();
}

void PainterOpenVG::drawLine(const IntPoint& from, const IntPoint& to)
{
    ASSERT(m_state);

    if (m_state->strokeDisabled())
        return;

    if (shadowEnabled() && !m_drawingShadow) {
        beginDrawShadow();
        drawLine(from, to);
        endDrawShadow();
    } else
        m_currentSurface->makeCurrent();

    // If possible, transform the cached line path. Creating paths in OpenVG
    // is pretty expensive, whereas changing transformations to draw our
    // cached 1x1 rectangle at the right coordinates... not so much.
    if (m_state->strokePaint.type() == PaintOpenVG::ColorPaint && m_state->strokeStyle == SolidStroke
        && fabs(m_state->surfaceTransformation.a()) == fabs(m_state->surfaceTransformation.d())) {
        IntSize delta(to.x() - from.x(), to.y() - from.y());
        float len = sqrtf(delta.width() * delta.width() + delta.height() * delta.height());
        float angleRadians = asin(delta.height() / len);

        AffineTransform localTransformation;
        localTransformation.rotate(rad2deg(angleRadians));
        localTransformation.scaleNonUniform(len, 1.0);
        localTransformation.setE(from.x());
        localTransformation.setF(from.y());

        AffineTransform transformation = m_state->surfaceTransformation;
        transformation.multLeft(localTransformation);

        float strokeThickness = m_state->strokeThickness * abs(m_state->surfaceTransformation.a());
        if (strokeThickness > 1.0)
            strokeThickness = roundf(strokeThickness);

        // Try to align lines to pixels, centering them between pixels for odd thickness values.
        if (fmod(strokeThickness + 0.5, 2.0) < 1.0) {
            // Even thickness value: just round the coordinates.
            transformation.setE(roundf(transformation.e()));
            transformation.setF(roundf(transformation.f()));
        } else if (fmod(fabs(transformation.b()) + piDouble / 4.0, piDouble) > piDouble / 2.0) {
            // More vertical than horizontal: center the x coordinate.
            transformation.setE(floorf(transformation.e()) + 0.5);
        } else {
            // More horizontal than vertical: center the y coordinate.
            transformation.setF(floorf(transformation.f()) + 0.5);
        }

        strokeThickness /= fabs(m_state->surfaceTransformation.a());

        Vector<VGfloat> currentMatrix(9);
        vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
        vgGetMatrix(currentMatrix.data());
        ASSERT_VG_NO_ERROR();

        // Unset the transformation and make up for it with transformed line width.
        vgSetf(VG_STROKE_LINE_WIDTH, strokeThickness);
        vgLoadMatrix(VGMatrix(transformation).toVGfloat());
        ASSERT_VG_NO_ERROR();

        vgDrawPath(m_currentSurface->cachedPath(SurfaceOpenVG::CachedLinePath), VG_STROKE_PATH);
        ASSERT_VG_NO_ERROR();

        vgSetf(VG_STROKE_LINE_WIDTH, m_state->strokeThickness);
        vgLoadMatrix(currentMatrix.data());
    } else {
        VGPath path = vgCreatePath(
            VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F,
            1.0 /* scale */, 0.0 /* bias */,
            2 /* expected number of segments */,
            4 /* expected number of total coordinates */,
            VG_PATH_CAPABILITY_APPEND_TO);
        ASSERT_VG_NO_ERROR();

        if (vguLine(path, from.x(), from.y(), to.x(), to.y()) == VGU_NO_ERROR) {
            vgDrawPath(path, VG_STROKE_PATH);
            ASSERT_VG_NO_ERROR();
        }

        vgDestroyPath(path);
        ASSERT_VG_NO_ERROR();
    }
}

void PainterOpenVG::drawArc(const IntRect& rect, int startAngle, int angleSpan, VGbitfield specifiedPaintModes)
{
    ASSERT(m_state);

    VGbitfield paintModes = 0;
    if (!m_state->strokeDisabled())
        paintModes |= VG_STROKE_PATH;
    if (!m_state->fillDisabled())
        paintModes |= VG_FILL_PATH;

    paintModes &= specifiedPaintModes;

    if (!paintModes)
        return;

    if (shadowEnabled() && !m_drawingShadow) {
        beginDrawShadow();
        drawArc(rect, startAngle, angleSpan, paintModes);
        endDrawShadow();
    } else
        m_currentSurface->makeCurrent();

    VGPath path = vgCreatePath(
        VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F,
        1.0 /* scale */, 0.0 /* bias */,
        2 /* expected number of segments */,
        4 /* expected number of total coordinates */,
        VG_PATH_CAPABILITY_APPEND_TO);
    ASSERT_VG_NO_ERROR();

    if (vguArc(path, rect.x() + rect.width() / 2.0, rect.y() + rect.height() / 2.0, rect.width(), rect.height(), -startAngle, -angleSpan, VGU_ARC_OPEN) == VGU_NO_ERROR) {
        vgDrawPath(path, VG_STROKE_PATH);
        ASSERT_VG_NO_ERROR();
    }

    vgDestroyPath(path);
    ASSERT_VG_NO_ERROR();
}

void PainterOpenVG::drawEllipse(const IntRect& rect, VGbitfield specifiedPaintModes)
{
    ASSERT(m_state);

    VGbitfield paintModes = 0;
    if (!m_state->strokeDisabled())
        paintModes |= VG_STROKE_PATH;
    if (!m_state->fillDisabled())
        paintModes |= VG_FILL_PATH;

    paintModes &= specifiedPaintModes;

    if (!paintModes)
        return;

    if (shadowEnabled() && !m_drawingShadow) {
        beginDrawShadow();
        drawEllipse(rect, paintModes);
        endDrawShadow();
    } else
        m_currentSurface->makeCurrent();

    VGPath path = vgCreatePath(
        VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F,
        1.0 /* scale */, 0.0 /* bias */,
        4 /* expected number of segments */,
        12 /* expected number of total coordinates */,
        VG_PATH_CAPABILITY_APPEND_TO);
    ASSERT_VG_NO_ERROR();

    if (vguEllipse(path, rect.x() + rect.width() / 2.0, rect.y() + rect.height() / 2.0, rect.width(), rect.height()) == VGU_NO_ERROR) {
        vgDrawPath(path, paintModes);
        ASSERT_VG_NO_ERROR();
    }

    vgDestroyPath(path);
    ASSERT_VG_NO_ERROR();
}

void PainterOpenVG::drawPolygon(size_t numPoints, const FloatPoint* points, VGbitfield specifiedPaintModes)
{
    ASSERT(m_state);

    VGbitfield paintModes = 0;
    if (!m_state->strokeDisabled())
        paintModes |= VG_STROKE_PATH;
    if (!m_state->fillDisabled())
        paintModes |= VG_FILL_PATH;

    paintModes &= specifiedPaintModes;

    if (!paintModes)
        return;

    if (shadowEnabled() && !m_drawingShadow) {
        beginDrawShadow();
        drawPolygon(numPoints, points, paintModes);
        endDrawShadow();
    } else
        m_currentSurface->makeCurrent();

    // Path segments: all points + "close path".
    const VGint numSegments = numPoints + 1;
    const VGint numCoordinates = numPoints * 2;

    VGPath path = vgCreatePath(
        VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F,
        1.0 /* scale */, 0.0 /* bias */,
        numSegments /* expected number of segments */,
        numCoordinates /* expected number of total coordinates */,
        VG_PATH_CAPABILITY_APPEND_TO);
    ASSERT_VG_NO_ERROR();

    Vector<VGfloat> vgPoints(numCoordinates);
    for (int i = 0; i < numPoints; ++i) {
        vgPoints[i*2]     = points[i].x();
        vgPoints[i*2 + 1] = points[i].y();
    }

    if (vguPolygon(path, vgPoints.data(), numPoints, VG_TRUE /* closed */) == VGU_NO_ERROR) {
        vgDrawPath(path, paintModes);
        ASSERT_VG_NO_ERROR();
    }

    vgDestroyPath(path);
    ASSERT_VG_NO_ERROR();
}

void PainterOpenVG::drawImage(TiledImageOpenVG* tiledImage, const FloatRect& dst, const FloatRect& src)
{
    ASSERT(m_state);

    if (shadowEnabled() && !m_drawingShadow) {
        beginDrawShadow();

        if (m_state->compositeOperation == CompositeCopy) {
            // Fill with any opaque color, the color transformation does the rest.
            setFillColor(Color::black);
            drawRect(dst, VG_FILL_PATH);
        } else
            drawImage(tiledImage, dst, src);

        endDrawShadow();
    } else
        m_currentSurface->makeCurrent();

    // If buffers can be larger than the maximum OpenVG image sizes,
    // we split them into tiles.
    IntRect drawnTiles = tiledImage->tilesInRect(src);
    AffineTransform srcToDstTransformation = makeMapBetweenRects(
        FloatRect(FloatPoint(0.0, 0.0), src.size()), dst);
    srcToDstTransformation.translate(-src.x(), -src.y());

    for (int yIndex = drawnTiles.y(); yIndex < drawnTiles.bottom(); ++yIndex) {
        for (int xIndex = drawnTiles.x(); xIndex < drawnTiles.right(); ++xIndex) {
            // The srcTile rectangle is an aligned tile cropped by the src rectangle.
            FloatRect tile(tiledImage->tileRect(xIndex, yIndex));
            FloatRect srcTile = intersection(src, tile);

            save();

            // If the image is drawn in full, all we need is the proper transformation
            // in order to get it drawn at the right spot on the surface.
            concatTransformation(AffineTransform(srcToDstTransformation).translate(tile.x(), tile.y()));

            // If only a part of the tile is drawn, we also need to clip the surface.
            if (srcTile != tile) {
                // Put boundaries relative to tile origin, as we already
                // translated to (x, y) with the transformation matrix.
                srcTile.move(-tile.x(), -tile.y());
                clipRect(srcTile, PainterOpenVG::IntersectClip);
            }

            VGImage image = tiledImage->tile(xIndex, yIndex);
            if (image != VG_INVALID_HANDLE) {
                vgDrawImage(image);
                ASSERT_VG_NO_ERROR();
            }

            restore();
        }
    }
}

#ifdef OPENVG_VERSION_1_1
void PainterOpenVG::drawText(VGFont vgFont, Vector<VGuint>& characters, VGfloat* adjustmentsX, VGfloat* adjustmentsY, const FloatPoint& point)
{
    ASSERT(m_state);

    VGbitfield paintModes = 0;

    if (m_state->textDrawingMode & cTextClip)
        return; // unsupported for every port except CG at the time of writing
    if (m_state->textDrawingMode & cTextFill && !m_state->fillDisabled())
        paintModes |= VG_FILL_PATH;
    if (m_state->textDrawingMode & cTextStroke && !m_state->strokeDisabled())
        paintModes |= VG_STROKE_PATH;

    if (shadowEnabled() && !m_drawingShadow) {
        beginDrawShadow();
        drawText(vgFont, characters, adjustmentsX, adjustmentsY, point);
        endDrawShadow();
    } else
        m_currentSurface->makeCurrent();

    FloatPoint effectivePoint = m_state->surfaceTransformation.mapPoint(point);
    FloatPoint p = point;
    AffineTransform* originalTransformation = 0;

    // In case the font isn't drawn at a pixel-exact baseline and we can easily
    // fix that (which is the case for non-rotated affine transforms), let's
    // align the starting point to the pixel boundary in order to prevent
    // font rendering issues such as glyphs that appear off by a pixel.
    // This causes us to have inconsistent spacing between baselines in a
    // larger paragraph, but that seems to be the least of all evils.
    if ((fmod(effectivePoint.x() + 0.01, 1.0) > 0.02 || fmod(effectivePoint.y() + 0.01, 1.0) > 0.02)
        && isNonRotatedAffineTransformation(m_state->surfaceTransformation))
    {
        originalTransformation = new AffineTransform(m_state->surfaceTransformation);
        setSurfaceTransformation(AffineTransform(
            m_state->surfaceTransformation.a(), 0,
            0, m_state->surfaceTransformation.d(),
            roundf(effectivePoint.x()), roundf(effectivePoint.y())));
        p = FloatPoint();
    }

    const VGfloat vgPoint[2] = { p.x(), p.y() };
    vgSetfv(VG_GLYPH_ORIGIN, 2, vgPoint);
    ASSERT_VG_NO_ERROR();

    vgDrawGlyphs(vgFont, characters.size(), characters.data(),
        adjustmentsX, adjustmentsY, paintModes, VG_TRUE /* allow autohinting */);
    ASSERT_VG_NO_ERROR();

    if (originalTransformation) {
        setSurfaceTransformation(*originalTransformation);
        delete originalTransformation;
    }
}
#endif

PassRefPtr<TiledImageOpenVG> PainterOpenVG::asNewNativeImage(const IntRect& src, VGImageFormat format)
{
    return asNewNativeImage(m_baseSurface, src, format);
}

PassRefPtr<TiledImageOpenVG> PainterOpenVG::asNewNativeImage(SurfaceOpenVG* surface, const IntRect& src, VGImageFormat format)
{
    ASSERT(m_state);
    surface->sharedSurface()->makeCurrent();

    const IntSize vgMaxImageSize(vgGeti(VG_MAX_IMAGE_WIDTH), vgGeti(VG_MAX_IMAGE_HEIGHT));
    ASSERT_VG_NO_ERROR();

    const IntRect rect = intersection(src, IntRect(0, 0, surface->width(), surface->height()));
    PassRefPtr<TiledImageOpenVG> tiledImage = adoptRef(new TiledImageOpenVG(rect.size(), vgMaxImageSize));

    const int numColumns = tiledImage->numColumns();
    const int numRows = tiledImage->numRows();

    // Create the images as resources of the shared surface/context.
    for (int yIndex = 0; yIndex < numRows; ++yIndex) {
        for (int xIndex = 0; xIndex < numColumns; ++xIndex) {
            IntRect tileRect = tiledImage->tileRect(xIndex, yIndex);
            VGImage image = vgCreateImage(format, tileRect.width(), tileRect.height(), VG_IMAGE_QUALITY_FASTER);
            ASSERT_VG_NO_ERROR();

            tiledImage->setTile(xIndex, yIndex, image);
        }
    }

    // Fill the image contents with our own surface/context being current.
    surface->makeCurrent();

    for (int yIndex = 0; yIndex < numRows; ++yIndex) {
        for (int xIndex = 0; xIndex < numColumns; ++xIndex) {
            IntRect tileRect = tiledImage->tileRect(xIndex, yIndex);

            vgGetPixels(tiledImage->tile(xIndex, yIndex), 0, 0,
                rect.x() + tileRect.x(), rect.y() + tileRect.y(),
                tileRect.width(), tileRect.height());
            ASSERT_VG_NO_ERROR();
        }
    }

    return tiledImage;
}

void PainterOpenVG::save(PainterOpenVG::SaveMode saveMode)
{
    ASSERT(m_state);

    // If the underlying context/surface was switched away by someone without
    // telling us, it might not correspond to the one assigned to this painter.
    // Switch back so we can save the state properly. (Should happen rarely.)
    // Use DontSaveOrApplyPainterState mode in order to avoid recursion.
    m_currentSurface->makeCurrent(SurfaceOpenVG::DontSaveOrApplyPainterState);

    if (saveMode == PainterOpenVG::CreateNewState) {
        m_state->saveMaskIfNecessary(this);
        PlatformPainterState* state = new PlatformPainterState(*m_state);
        m_stateStack.append(state);
        m_state = m_stateStack.last();
    } else if (saveMode == PainterOpenVG::CreateNewStateWithPaintStateOnly) {
        m_state->saveMaskIfNecessary(this);
        PlatformPainterState* state = new PlatformPainterState();
        state->copyPaintState(m_state);
        state->modifiedStateCategories = MaskState | ScissorState | TransformationState;
        m_stateStack.append(state);
        m_state = m_stateStack.last();
    } else // if (saveMode == PainterOpenVG::KeepCurrentState)
        m_state->saveMaskIfNecessary(this);
}

void PainterOpenVG::restore()
{
    ASSERT(m_stateStack.size() >= 2);
    m_currentSurface->makeCurrent(SurfaceOpenVG::DontApplyPainterState);

    PlatformPainterState* state = m_stateStack.last();
    PainterStateCategoryFlags modifiedStateCategories = state->modifiedStateCategories;
    m_stateStack.removeLast();
    delete state;

    m_state = m_stateStack.last();
    m_state->applyState(this, modifiedStateCategories);
}

}
