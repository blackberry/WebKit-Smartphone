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

#ifndef GradientOpenVG_h
#define GradientOpenVG_h

#include "Color.h"
#include "AffineTransform.h"

#include <openvg.h>
#include <wtf/Vector.h>

namespace WebCore {

class GradientOpenVG {
public:
    GradientOpenVG();
    GradientOpenVG(const GradientOpenVG&);

    bool isValid() const;
    bool isEmpty() const; // empty == invalid, or valid but not drawn

    bool isRadial() const { return m_radial; }
    VGColorRampSpreadMode spreadMode() { return m_spread; }
    const Vector<VGfloat>& coordinates() { return m_coordinates; }
    const Vector<VGfloat>& colorStops() { return m_colorStops; }

    void setTransformation(const AffineTransform& transformation) { m_transformation = transformation; }
    const AffineTransform& transformation() const { return m_transformation; }

//private: // FIXME: make private and use accessors once Eli's gradient refactoring is done
    bool m_radial;
    VGColorRampSpreadMode m_spread;
    // m_coordinates contains 4 coordinates {x0, y0, x1, y1} for linear gradients
    // or 5 coordinates {cx, cy, fx, fy, r} for radial gradients.
    Vector<VGfloat> m_coordinates;
    Vector<VGfloat> m_colorStops;
    AffineTransform m_transformation;
};

}

#endif
