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

#ifndef PaintOpenVG_h
#define PaintOpenVG_h

#include "Color.h"
#include "GradientOpenVG.h"
#include "PatternOpenVG.h"

#include <openvg.h>

namespace WebCore {

class GradientOpenVG;
class PatternOpenVG;

class PaintOpenVG {
public:
    enum PaintType {
        ColorPaint,
        GradientPaint,
        PatternPaint,
        NoPaint
    };

    PaintOpenVG();
    PaintOpenVG(const PaintOpenVG&);
    PaintOpenVG(const Color&);
    PaintOpenVG(RGBA32);
    PaintOpenVG(const GradientOpenVG&);
    PaintOpenVG(const PatternOpenVG&);
    ~PaintOpenVG();

    PaintOpenVG& operator=(const PaintOpenVG&);
    PaintOpenVG& operator=(const Color&);
    PaintOpenVG& operator=(RGBA32);
    PaintOpenVG& operator=(const GradientOpenVG&);
    PaintOpenVG& operator=(const PatternOpenVG&);

    void setColor(const Color&);
    void setColor(RGBA32);
    void setGradient(const GradientOpenVG&);
    void setPattern(const PatternOpenVG&);
    void clear();

    PaintType type() const { return m_type; }
    bool isEmpty() const { return m_isEmpty; }

    const Color& color() const;
    const GradientOpenVG& gradient() const;
    const PatternOpenVG& pattern() const;

    /**
     * Apply this object's paint settings to either stroke or fill paint of
     * the current OpenVG context. For gradient or pattern paints,
     * fill/stroke-to-user transformation matrices will also be set.
     * If this->isEmpty(), no changes will be made to any paint or state.
     *
     * If VG_INVALID_HANDLE is passed for existingPaint, a temporary VGPaint
     * will be created for the duration of this method call and immediately
     * destroyed (it stays in use because of reference counting by OpenVG).
     * If an existing valid VGPaint is passed, its parameters will be
     * overwritten. This is the preferred mode of operation, as you want to
     * avoid creation and destruction of VGPaints.
     * If an invalid VGPaint is passed, this function will fail with an
     * assertion. This may happen when you retrieve the VGPaint handle
     * via vgGetPaint(), OpenVG provides no means to check its validity
     * so avoid calling vgGetPaint() altogether.
     */
    void apply(VGPaintMode paintMode, VGPaint existingPaint = VG_INVALID_HANDLE) const;

private:
    void deletePaint();

private:
    union PaintPtr {
        Color* color;
        GradientOpenVG* gradient;
        PatternOpenVG* pattern;

        PaintPtr() : color(0) {}
        void clear() { color = 0; }
    };

    PaintPtr m_paint;
    PaintType m_type;
    bool m_isEmpty;
};

}

#endif
