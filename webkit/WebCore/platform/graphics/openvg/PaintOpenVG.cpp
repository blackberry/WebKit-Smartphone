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
#include "PaintOpenVG.h"

#include <wtf/Assertions.h>

namespace WebCore {

PaintOpenVG::PaintOpenVG()
    : m_type(NoPaint)
    , m_isEmpty(true)
{
}

PaintOpenVG::PaintOpenVG(const PaintOpenVG& other)
    : m_type(other.m_type)
    , m_isEmpty(other.m_isEmpty)
{
    if (m_type == ColorPaint)
        m_paint.color = new Color(*other.m_paint.color);
    else if (m_type == GradientPaint)
        m_paint.gradient = new GradientOpenVG(*other.m_paint.gradient);
    else if (m_type == PatternPaint)
        m_paint.pattern = new PatternOpenVG(*other.m_paint.pattern);
}

PaintOpenVG::PaintOpenVG(const Color& color)
    : m_type(ColorPaint)
{
    m_paint.color = new Color(color);
    m_isEmpty = !color.isValid() || !color.alpha();
}

PaintOpenVG::PaintOpenVG(RGBA32 rgba)
    : m_type(ColorPaint)
{
    m_paint.color = new Color(rgba);
    m_isEmpty = !m_paint.color->alpha();
}

PaintOpenVG::PaintOpenVG(const GradientOpenVG& gradient)
    : m_type(GradientPaint)
{
    m_paint.gradient = new GradientOpenVG(gradient);
    m_isEmpty = m_paint.gradient->isEmpty();
}

PaintOpenVG::PaintOpenVG(const PatternOpenVG& pattern)
    : m_type(PatternPaint)
{
    m_paint.pattern = new PatternOpenVG(pattern);
    m_isEmpty = !m_paint.pattern->isValid();
}

PaintOpenVG::~PaintOpenVG()
{
    deletePaint();
}

PaintOpenVG& PaintOpenVG::operator=(const PaintOpenVG& other)
{
    if (&other == this)
        return *this;

    if (other.m_type == ColorPaint)
        setColor(*other.m_paint.color);
    else if (other.m_type == GradientPaint)
        setGradient(*other.m_paint.gradient);
    else if (other.m_type == PatternPaint)
        setPattern(*other.m_paint.pattern);
    else if (other.m_type == NoPaint)
        clear();
    else
        ASSERT_NOT_REACHED();

    return *this;
}

PaintOpenVG& PaintOpenVG::operator=(const Color& color)
{
    setColor(color);
    return *this;
}

PaintOpenVG& PaintOpenVG::operator=(RGBA32 rgba)
{
    setColor(rgba);
    return *this;
}

PaintOpenVG& PaintOpenVG::operator=(const GradientOpenVG& gradient)
{
    setGradient(gradient);
    return *this;
}

PaintOpenVG& PaintOpenVG::operator=(const PatternOpenVG& pattern)
{
    setPattern(pattern);
    return *this;
}

void PaintOpenVG::setColor(const Color& color)
{
    if (m_type == ColorPaint)
        *m_paint.color = color;
    else {
        deletePaint();
        m_paint.color = new Color(color);
    }
    m_type = ColorPaint;
    m_isEmpty = !color.isValid() || !color.alpha();
}

void PaintOpenVG::setColor(RGBA32 rgba)
{
    if (m_type == ColorPaint)
        m_paint.color->setRGB(rgba);
    else {
        deletePaint();
        m_paint.color = new Color(rgba);
    }
    m_type = ColorPaint;
    m_isEmpty = !m_paint.color->alpha();
}

void PaintOpenVG::setGradient(const GradientOpenVG& gradient)
{
    if (m_type == GradientPaint)
        *m_paint.gradient = gradient;
    else {
        deletePaint();
        m_paint.gradient = new GradientOpenVG(gradient);
    }
    m_type = GradientPaint;
    m_isEmpty = gradient.isEmpty();
}

void PaintOpenVG::setPattern(const PatternOpenVG& pattern)
{
    if (m_type == PatternPaint)
        *m_paint.pattern = pattern;
    else {
        deletePaint();
        m_paint.pattern = new PatternOpenVG(pattern);
    }
    m_type = PatternPaint;
    m_isEmpty = !pattern.isValid();
}

void PaintOpenVG::clear()
{
    deletePaint();
    m_type = NoPaint;
    m_isEmpty = true;
    m_paint.clear();
}

void PaintOpenVG::deletePaint()
{
    if (m_type == ColorPaint)
        delete m_paint.color;
    else if (m_type == GradientPaint)
        delete m_paint.gradient;
    else if (m_type == PatternPaint)
        delete m_paint.pattern;
}

const Color& PaintOpenVG::color() const
{
    ASSERT(m_type == ColorPaint);
    return *m_paint.color;
}

const GradientOpenVG& PaintOpenVG::gradient() const
{
    ASSERT(m_type == GradientPaint);
    return *m_paint.gradient;
}

const PatternOpenVG& PaintOpenVG::pattern() const
{
    ASSERT(m_type == PatternPaint);
    return *m_paint.pattern;
}

void PaintOpenVG::apply(VGPaintMode paintMode, VGPaint existingPaint) const
{
    VGPaint vgPaint = existingPaint;

    if (existingPaint == VG_INVALID_HANDLE) {
        vgPaint = vgCreatePaint();
        ASSERT_VG_NO_ERROR();
    }

    if (isEmpty()) {
        vgSetParameteri(vgPaint, VG_PAINT_TYPE, VG_PAINT_TYPE_COLOR);
        vgSetColor(vgPaint, 0); // transparent black (r: 0, g: 0, b: 0, a: 0)

        ASSERT_VG_NO_ERROR();
    } else if (m_type == ColorPaint) {
        VGuint vgColor = m_paint.color->red();
        vgColor = (vgColor << 8) | m_paint.color->green();
        vgColor = (vgColor << 8) | m_paint.color->blue();
        vgColor = (vgColor << 8) | m_paint.color->alpha();

        vgSetParameteri(vgPaint, VG_PAINT_TYPE, VG_PAINT_TYPE_COLOR);
        vgSetColor(vgPaint, vgColor);

        ASSERT_VG_NO_ERROR();
    } else if (m_type == GradientPaint) {
        const VGuint matrixMode = paintMode == VG_FILL_PATH
            ? VG_MATRIX_FILL_PAINT_TO_USER
            : VG_MATRIX_STROKE_PAINT_TO_USER;

        vgSeti(VG_MATRIX_MODE, matrixMode);
        vgLoadMatrix(VGMatrix(m_paint.gradient->transformation()).toVGfloat());

        if (m_paint.gradient->m_radial) {
            vgSetParameteri(vgPaint, VG_PAINT_TYPE, VG_PAINT_TYPE_RADIAL_GRADIENT);
            vgSetParameterfv(vgPaint, VG_PAINT_RADIAL_GRADIENT, 5, m_paint.gradient->coordinates().data());
        } else {
            vgSetParameteri(vgPaint, VG_PAINT_TYPE, VG_PAINT_TYPE_LINEAR_GRADIENT);
            vgSetParameterfv(vgPaint, VG_PAINT_LINEAR_GRADIENT, 4, m_paint.gradient->coordinates().data());
        }
        vgSetParameterfv(vgPaint, VG_PAINT_COLOR_RAMP_STOPS,
            m_paint.gradient->colorStops().size(), m_paint.gradient->colorStops().data());
        vgSetParameteri(vgPaint, VG_PAINT_COLOR_RAMP_SPREAD_MODE, m_paint.gradient->spreadMode());

        ASSERT_VG_NO_ERROR();
    } else if (m_type == PatternPaint) {
        const VGuint matrixMode = paintMode == VG_FILL_PATH
            ? VG_MATRIX_FILL_PAINT_TO_USER
            : VG_MATRIX_STROKE_PAINT_TO_USER;

        vgSeti(VG_MATRIX_MODE, matrixMode);
        vgLoadMatrix(VGMatrix(m_paint.pattern->effectiveTransformation()).toVGfloat());

        vgSetParameteri(vgPaint, VG_PAINT_TYPE, VG_PAINT_TYPE_PATTERN);
        vgSetParameteri(vgPaint, VG_PAINT_PATTERN_TILING_MODE, VG_TILE_REPEAT);
        vgPaintPattern(vgPaint, m_paint.pattern->vgImage());

        ASSERT_VG_NO_ERROR();
    }

    vgSetPaint(vgPaint, paintMode);
    ASSERT_VG_NO_ERROR();

    if (existingPaint == VG_INVALID_HANDLE) {
        vgDestroyPaint(vgPaint);
        ASSERT_VG_NO_ERROR();
    }
}

}