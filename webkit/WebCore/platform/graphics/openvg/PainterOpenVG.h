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

#ifndef PainterOpenVG_h
#define PainterOpenVG_h

#include "Color.h"
#include "GraphicsContext.h"

#include <openvg.h>

#include <wtf/Noncopyable.h>
#include <wtf/PassRefPtr.h>
#include <wtf/Vector.h>

namespace WebCore {

class AffineTransform;
class FloatPoint;
class FloatRect;
class IntRect;
class IntSize;
class GradientOpenVG;
class PaintOpenVG;
class Path;
class PatternOpenVG;
class SurfaceOpenVG;
class TiledImageOpenVG;

struct TransparencyLayer;
struct PlatformPainterState;

class PainterOpenVG : public Noncopyable {
public:
    friend class SurfaceOpenVG;
    friend struct PlatformPainterState;

    enum SaveMode {
        CreateNewState,
        KeepCurrentState,
        CreateNewStateWithPaintStateOnly // internal usage only, do not use outside PainterOpenVG
    };
    enum ClipOperation {
        IntersectClip = VG_INTERSECT_MASK,
        SubtractClip = VG_SUBTRACT_MASK
    };
    enum PainterStateCategory {
        NoStateCategories = 0,
        MaskState = 1 << 0,
        ScissorState = 1 << 1,
        TransformationState = 1 << 2,
        BlendingState = 1 << 3,
        FillState = 1 << 4,
        StrokeState = 1 << 5,
        QualitySettingState = 1 << 6,
        AllStateCategories = MaskState | ScissorState | TransformationState
            | BlendingState | FillState | StrokeState | QualitySettingState
    };
    typedef int PainterStateCategoryFlags;

    PainterOpenVG();
    PainterOpenVG(SurfaceOpenVG*);
    ~PainterOpenVG();

    void begin(SurfaceOpenVG*);
    void end();

    /**
     * PainterOpenVG keeps track of OpenVG context changes so that restore()
     * only restores context parameters that have actually changed from the
     * previously saved state (with save()). If you call vgSet*(),
     * vgSetParameter*() or other state-changing functionality directly,
     * bypassing this class, then you need to notify it about changes instead.
     */
    void setStateModified(PainterStateCategoryFlags modifiedStateCategories);

    void beginTransparencyLayer(float opacity);
    void endTransparencyLayer();

    AffineTransform transformation() const;
    void setTransformation(const AffineTransform&);
    void concatTransformation(const AffineTransform&);

    static void transformPath(VGPath dst, VGPath src, const AffineTransform&);

    CompositeOperator compositeOperation() const;
    void setCompositeOperation(CompositeOperator);
    float opacity() const;
    void setOpacity(float);

    float strokeThickness() const;
    void setStrokeThickness(float);
    StrokeStyle strokeStyle() const;
    void setStrokeStyle(const StrokeStyle&);

    void setLineDash(const DashArray&, float dashOffset);
    void setLineCap(LineCap);
    void setLineJoin(LineJoin);
    void setMiterLimit(float);

    const PaintOpenVG& strokePaint() const;
    void setStrokePaint(const PaintOpenVG&);
    void setStrokeColor(const Color&);
    void setStrokeGradient(const GradientOpenVG&);
    void setStrokePattern(const PatternOpenVG&);

    const PaintOpenVG& fillPaint() const;
    void setFillPaint(const PaintOpenVG&);
    void setFillColor(const Color&);
    void setFillGradient(const GradientOpenVG&);
    void setFillPattern(const PatternOpenVG&);

    bool shadowEnabled() const;
    void setShadow(const FloatSize& offset, float blur, const Color&);
    void clearShadow();

    // Low-level functions that can be used if shadowEnabled() == true and
    // you call vg*() draw functions directly. All of the drawing methods in
    // PainterOpenVG use this implicitely, you don't need to care about shadows
    // for those once it has been set with setShadow() previously.
    void beginDrawShadow();
    void endDrawShadow();

    int textDrawingMode() const;
    void setTextDrawingMode(int mode);

    bool antialiasingEnabled() const;
    void setAntialiasingEnabled(bool);

    void drawRect(const FloatRect&, VGbitfield paintModes = (VG_STROKE_PATH | VG_FILL_PATH));
    void drawRoundedRect(const FloatRect&, const IntSize& topLeft, const IntSize& topRight, const IntSize& bottomLeft, const IntSize& bottomRight, VGbitfield paintModes = (VG_STROKE_PATH | VG_FILL_PATH));
    void drawLine(const IntPoint& from, const IntPoint& to);
    void drawArc(const IntRect& ellipseBounds, int startAngle, int angleSpan, VGbitfield paintModes = (VG_STROKE_PATH | VG_FILL_PATH));
    void drawEllipse(const IntRect& bounds, VGbitfield paintModes = (VG_STROKE_PATH | VG_FILL_PATH));
    void drawPolygon(size_t numPoints, const FloatPoint* points, VGbitfield paintModes = (VG_STROKE_PATH | VG_FILL_PATH));
    void drawImage(TiledImageOpenVG*, const FloatRect& dst, const FloatRect& src);
#ifdef OPENVG_VERSION_1_1
    void drawText(VGFont, Vector<VGuint>& characters, VGfloat* adjustmentsX, VGfloat* adjustmentsY, const FloatPoint&);
#endif

    void scale(const FloatSize& scaleFactors);
    void rotate(float radians);
    void translate(float dx, float dy);

    void beginPath();
    void addPath(const Path&);
    void addPath(VGPath);
    Path* currentPath() const;
    void drawPath(const Path&, VGbitfield paintModes = (VG_STROKE_PATH | VG_FILL_PATH), WindRule fillRule = RULE_NONZERO);
    void drawPath(VGPath, VGbitfield paintModes = (VG_STROKE_PATH | VG_FILL_PATH), WindRule fillRule = RULE_NONZERO);

    void clipRect(const FloatRect&, PainterOpenVG::ClipOperation);
    void clipEllipse(const FloatRect&, PainterOpenVG::ClipOperation);
    void clipPath(const Path&, PainterOpenVG::ClipOperation, WindRule clipRule = RULE_NONZERO);
    void clipPath(VGPath, PainterOpenVG::ClipOperation, WindRule clipRule = RULE_NONZERO);

    PassRefPtr<TiledImageOpenVG> asNewNativeImage(const IntRect& src, VGImageFormat);

    void save(PainterOpenVG::SaveMode saveMode = CreateNewState);
    void restore();

    SurfaceOpenVG* baseSurface() { return m_baseSurface; }
    SurfaceOpenVG* currentSurface() { return m_currentSurface; }
    void blitToSurface();

private:
    void destroyTransparencyLayers();
    void destroyPainterStates();
    void applyState();

    void endTransparencyLayer(int blurRadius);

    void intersectScissorRect(const FloatRect&);
    void transformAndClipPath(VGPath, const FloatRect&, PainterOpenVG::ClipOperation);
    void transformAndFillPath(VGPath, const FloatRect&);

    void setSurfaceTransformation(const AffineTransform&);
    PassRefPtr<TiledImageOpenVG> asNewNativeImage(SurfaceOpenVG* surface, const IntRect& src, VGImageFormat format);

private:
    Vector<TransparencyLayer*> m_layers;
    Vector<PlatformPainterState*> m_stateStack;
    PlatformPainterState* m_state;
    SurfaceOpenVG* m_baseSurface;
    SurfaceOpenVG* m_currentSurface;
    Path* m_currentPath;

    bool m_drawingShadow;
};

}

#endif
