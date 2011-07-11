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
 *
 *
 * Portions adapted from the Cairo Graphics Library:
 *
 * cairo - a vector graphics library with display and print output
 *
 * Copyright Ac 2004 Red Hat, Inc
 * Copyright Ac 2005-2007 Emmanuel Pacaud <emmanuel.pacaud@free.fr>
 * Copyright Ac 2006 Red Hat, Inc
 *
 * This library is free software; you can redistribute it and/or
 * modify it either under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * (the "LGPL") or, at your option, under the terms of the Mozilla
 * Public License Version 1.1 (the "MPL"). If you do not alter this
 * notice, a recipient may use your version of this file under either
 * the MPL or the LGPL.
 *
 * You should have received a copy of the LGPL along with this library
 * in the file COPYING-LGPL-2.1; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 * You should have received a copy of the MPL along with this library
 * in the file COPYING-MPL-1.1
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY
 * OF ANY KIND, either express or implied. See the LGPL or the MPL for
 * the specific language governing rights and limitations.
 *
 * The Original Code is the cairo graphics library.
 *
 * The Initial Developer of the Original Code is University of Southern
 * California.
 *
 * Contributor(s):
 *  Kristian HA,gsberg <krh@redhat.com>
 *  Emmanuel Pacaud <emmanuel.pacaud@free.fr>
 *  Carl Worth <cworth@cworth.org>
 */

#include "config.h"
#include "GradientOpenVG.h"

#include "Gradient.h"
#include "FloatPoint.h"
#include "FloatRect.h"
#include "GraphicsContext.h"

#include <algorithm>
#include <openvg.h>

namespace WebCore {

static inline bool compareStops(const Gradient::ColorStop& a, const Gradient::ColorStop& b)
{
    return a.stop < b.stop;
}

GradientOpenVG::GradientOpenVG()
    : m_radial(false)
    , m_spread(VG_COLOR_RAMP_SPREAD_REPEAT)
{
}

GradientOpenVG::GradientOpenVG(const GradientOpenVG& other)
    : m_coordinates(other.m_coordinates)
    , m_colorStops(other.m_colorStops)
    , m_radial(other.m_radial)
    , m_spread(other.m_spread)
    , m_transformation(other.m_transformation)
{
}

bool GradientOpenVG::isValid() const
{
    return !m_colorStops.isEmpty();
}

bool GradientOpenVG::isEmpty() const
{
    return !isValid()
        || (!m_radial && m_coordinates[0] == m_coordinates[2] && m_coordinates[1] == m_coordinates[3])
        || (m_radial && m_coordinates[4] <= 0);
}

GradientOpenVG* Gradient::platformGradient()
{
    if (m_gradient)
        return m_gradient;

    m_gradient = new GradientOpenVG;
    m_gradient->m_radial = m_radial;
    m_gradient->m_transformation = m_gradientSpaceTransformation;

    switch (m_spreadMethod) {
    case SpreadMethodPad:
        m_gradient->m_spread = VG_COLOR_RAMP_SPREAD_PAD;
        break;
    case SpreadMethodReflect:
        m_gradient->m_spread = VG_COLOR_RAMP_SPREAD_REFLECT;
        break;
    case SpreadMethodRepeat:
        m_gradient->m_spread = VG_COLOR_RAMP_SPREAD_REPEAT;
        break;
    }

    if (m_stops.size() > 0) {
        if (!m_stopsSorted) {
            std::stable_sort(m_stops.begin(), m_stops.end(), compareStops);
            m_stopsSorted = true;
        }
    }

    if (!m_radial) {
        // linear gradient
        m_gradient->m_coordinates.resize(4);
        m_gradient->m_coordinates[0] = m_p0.x();
        m_gradient->m_coordinates[1] = m_p0.y();
        m_gradient->m_coordinates[2] = m_p1.x();
        m_gradient->m_coordinates[3] = m_p1.y();

        if (!m_stops.isEmpty()) {
            m_gradient->m_colorStops.resize(m_stops.size()*5);
            for (int i = 0; i < m_stops.size(); i++) {
                m_gradient->m_colorStops[i*5] = m_stops[i].stop;
                m_gradient->m_colorStops[i*5 + 1] = m_stops[i].red;
                m_gradient->m_colorStops[i*5 + 2] = m_stops[i].green;
                m_gradient->m_colorStops[i*5 + 3] = m_stops[i].blue;
                m_gradient->m_colorStops[i*5 + 4] = m_stops[i].alpha;
            }
        }

        return m_gradient;
    }

    // radial gradient
    // OpenVG takes centre, focus, radius.  WebKit provides p0+r0, p1+r1.
    FloatPoint c0;
    FloatPoint c1;
    FloatPoint focus;
    double r0;
    double r1;

    bool reverseStops;
    // c0 should be the centre of the circle with the smaller radius r0
    if (m_r0 < m_r1) {
        c0 = m_p0;
        c1 = m_p1;
        r0 = m_r0;
        r1 = m_r1;
        reverseStops = false;
    } else {
        c0 = m_p1;
        c1 = m_p0;
        r0 = m_r1;
        r1 = m_r0;
        reverseStops = true;
    }

    if (r0 == r1) {
        if (c0 == c1)
            return m_gradient;

        m_gradient->m_coordinates.resize(5);
        m_gradient->m_coordinates[0] = c1.x();    // centre
        m_gradient->m_coordinates[1] = c1.y();
        m_gradient->m_coordinates[2] = c1.x();    // focal point
        m_gradient->m_coordinates[3] = c1.y();
        m_gradient->m_coordinates[4] = r1;      // radius of centre

        if (m_stops.size() < 1) {
            // if there's no stop, we should use transparent black
            m_gradient->m_colorStops.resize(5);
            m_gradient->m_colorStops[0] = 0;
            m_gradient->m_colorStops[1] = 0.0;
            m_gradient->m_colorStops[2] = 0.0;
            m_gradient->m_colorStops[3] = 0.0;
            m_gradient->m_colorStops[4] = 0.0;
        } else {
            int nStops = m_stops.size() > 1 ? 1 : 2;
            m_gradient->m_colorStops.resize(nStops * 5);

            m_gradient->m_colorStops[0] = 0;
            m_gradient->m_colorStops[1] = m_stops[0].red;
            m_gradient->m_colorStops[2] = m_stops[0].green;
            m_gradient->m_colorStops[3] = m_stops[0].blue;
            m_gradient->m_colorStops[4] = m_stops[0].alpha;
            if (nStops > 1) {
                m_gradient->m_colorStops[5] = 0;
                m_gradient->m_colorStops[6] = m_stops[nStops - 1].red;
                m_gradient->m_colorStops[7] = m_stops[nStops - 1].green;
                m_gradient->m_colorStops[8] = m_stops[nStops - 1].blue;
                m_gradient->m_colorStops[9] = m_stops[nStops - 1].alpha;
            }
        }

        return m_gradient;
    }

    double startOffset;
    bool emulateReflect = false;

    focus.setX((r1 * c0.x() - r0 * c1.x()) / (r1 - r0));
    focus.setY((r1 * c0.y() - r0 * c1.y()) / (r1 - r0));

    /* OpenVG doesn't support the inner circle and use instead a gradient focal.
     * That means we need to emulate the WebKit behaviour by processing the
     * WebKit gradient stops.
     * The SpreadMethodPad mode is quite easy to handle,
     * it's just a matter of stop position translation and calculation of
     * the corresponding OpenVG radial gradient focal.
     * The SpreadMethodReflect and SpreadMethodRepeat modes require a new
     * radial gradient, with a new outer circle equal to r1 - r0 in the SpreadMethodRepeat
     * case and 2 * (r1 - r0) in the SpreadMethodReflect case, and a new gradient stop
     * list that maps to the original WebKit stop list.
     */

    if ((m_spreadMethod == SpreadMethodReflect || m_spreadMethod == SpreadMethodRepeat) && r0 > 0.0) {
        double rOrig = r1;

        if (m_spreadMethod == SpreadMethodReflect) {
            r1 = 2 * r1 - r0;
            emulateReflect = true;
        }

        startOffset = fmod(r1, r1 - r0) / (r1 - r0) - 1.0;
        double r = r1 - r0;

        /* New position of outer circle. */
        double x = r * (c1.x() - focus.x()) / rOrig + focus.x();
        double y = r * (c1.y() - focus.y()) / rOrig + focus.y();

        c1.setX(x);
        c1.setY(y);
        r1 = r;
        r0 = 0.0;
    } else
        startOffset = r0 / r1;

    m_gradient->m_coordinates.resize(5);
    m_gradient->m_coordinates[0] = c1.x();       // centre
    m_gradient->m_coordinates[1] = c1.y();
    m_gradient->m_coordinates[2] = focus.x();    // focal point
    m_gradient->m_coordinates[3] = focus.y();
    m_gradient->m_coordinates[4] = r1;         // radius of centre

    if (emulateReflect)
        m_gradient->m_spread = VG_COLOR_RAMP_SPREAD_REPEAT;

    if (m_stops.size() < 1)
        return m_gradient;

    if (m_stops.size() == 1) {
        m_gradient->m_colorStops.resize(5);
        m_gradient->m_colorStops[0] = m_stops[0].stop;
        m_gradient->m_colorStops[1] = m_stops[0].red;
        m_gradient->m_colorStops[2] = m_stops[0].green;
        m_gradient->m_colorStops[3] = m_stops[0].blue;
        m_gradient->m_colorStops[4] = m_stops[0].alpha;
        return m_gradient;
    }

    int nStops;
    Vector<ColorStop> actualStops;
    if (emulateReflect || reverseStops) {
        nStops = emulateReflect ? m_stops.size() * 2 - 2: m_stops.size();
        actualStops.resize(nStops);

        for (int i = 0; i < nStops; i++) {
            if (reverseStops) {
                actualStops[i] = m_stops[nStops - i - 1];
                actualStops[i].stop = 1.0 - actualStops[i].stop;
            } else
                actualStops[i] = m_stops[i];

            if (emulateReflect) {
                actualStops[i].stop /= 2;
                if (i > 0 && i < (nStops - 1)) {
                    if (reverseStops) {
                        actualStops[i + nStops - 1] = m_stops[i];
                        actualStops[i + nStops - 1].stop = 0.5 + 0.5 * actualStops[i + nStops - 1].stop;
                    } else {
                        actualStops[i + nStops - 1] = m_stops[nStops - i - 1];
                        actualStops[i + nStops - 1].stop = 1 - 0.5 * actualStops[i + nStops - 1].stop;
                    }
                }
            }
        }
    } else {
        nStops = m_stops.size();
        actualStops = m_stops;
    }

    if (startOffset >= 0.0) {
        m_gradient->m_colorStops.resize(actualStops.size() * 5);
        for (int i = 0; i < nStops; i++) {
            double offset = startOffset + (1 - startOffset) * actualStops[i].stop;
            m_gradient->m_colorStops[i*5] = offset;
            m_gradient->m_colorStops[i*5 + 1] = actualStops[i].red;
            m_gradient->m_colorStops[i*5 + 2] = actualStops[i].green;
            m_gradient->m_colorStops[i*5 + 3] = actualStops[i].blue;
            m_gradient->m_colorStops[i*5 + 4] = actualStops[i].alpha;
        }

        return m_gradient;
    }

    bool found = FALSE;
    unsigned int offsetIndex;
    float startRed, startGreen, startBlue, startAlpha;
    float stopRed, stopGreen, stopBlue, stopAlpha;

    for (int i = 0; i < nStops; i++) {
        if (actualStops[i].stop >= -startOffset) {
            if (i > 0) {
                if (actualStops[i].stop != actualStops[i-1].stop) {
                    double x0, x1;

                    x0 = actualStops[i-1].stop;
                    x1 = actualStops[i].stop;
                    startRed = stopRed = actualStops[i-1].red + (actualStops[i].red - actualStops[i-1].red) * (-startOffset - x0) / (x1 - x0);
                    startGreen = stopGreen = actualStops[i-1].green + (actualStops[i].green - actualStops[i-1].green) * (-startOffset - x0) / (x1 - x0);
                    startBlue = stopBlue = actualStops[i-1].blue + (actualStops[i].blue - actualStops[i-1].blue) * (-startOffset - x0) / (x1 - x0);
                    startAlpha = stopAlpha = actualStops[i-1].alpha + (actualStops[i].alpha - actualStops[i-1].alpha) * (-startOffset - x0) / (x1 - x0);
                } else {
                    stopRed = actualStops[i-1].red;
                    stopGreen = actualStops[i-1].green;
                    stopBlue = actualStops[i-1].blue;
                    stopAlpha = actualStops[i-1].alpha;
                    startRed = actualStops[i].red;
                    startGreen = actualStops[i].green;
                    startBlue = actualStops[i].blue;
                    startAlpha = actualStops[i].alpha;
                }
            } else {
                startRed = stopRed = actualStops[i].red;
                startGreen = stopGreen = actualStops[i].green;
                startBlue = stopBlue = actualStops[i].blue;
                startAlpha = stopAlpha = actualStops[i].alpha;
            }
            offsetIndex = i;
            found = true;
            break;
        }
    }

    if (!found) {
        offsetIndex = nStops - 1;
        startRed = stopRed = actualStops[offsetIndex].red;
        startGreen = stopGreen = actualStops[offsetIndex].green;
        startBlue = stopBlue = actualStops[offsetIndex].blue;
        startAlpha = stopAlpha = actualStops[offsetIndex].alpha;
    }

    m_gradient->m_colorStops.resize((nStops + 2) * 5);
    int stopNum = 0;
    m_gradient->m_colorStops[stopNum * 5] = 0;
    m_gradient->m_colorStops[stopNum * 5 + 1] = startRed;
    m_gradient->m_colorStops[stopNum * 5 + 2] = startGreen;
    m_gradient->m_colorStops[stopNum * 5 + 3] = startBlue;
    m_gradient->m_colorStops[stopNum * 5 + 4] = startAlpha;
    ++stopNum;

    for (int i = offsetIndex; i < nStops; i++) {
        m_gradient->m_colorStops[stopNum * 5] = actualStops[i].stop + startOffset;
        m_gradient->m_colorStops[stopNum * 5 + 1] = actualStops[i].red;
        m_gradient->m_colorStops[stopNum * 5 + 2] = actualStops[i].green;
        m_gradient->m_colorStops[stopNum * 5 + 3] = actualStops[i].blue;
        m_gradient->m_colorStops[stopNum * 5 + 4] = actualStops[i].alpha;
        ++stopNum;
    }

    for (int i = 0; i < offsetIndex; i++) {
        m_gradient->m_colorStops[stopNum * 5] = 1.0 + actualStops[i].stop + startOffset;
        m_gradient->m_colorStops[stopNum * 5 + 1] = actualStops[i].red;
        m_gradient->m_colorStops[stopNum * 5 + 2] = actualStops[i].green;
        m_gradient->m_colorStops[stopNum * 5 + 3] = actualStops[i].blue;
        m_gradient->m_colorStops[stopNum * 5 + 4] = actualStops[i].alpha;
        ++stopNum;
    }

    m_gradient->m_colorStops[stopNum * 5] = 0;
    m_gradient->m_colorStops[stopNum * 5 + 1] = stopRed;
    m_gradient->m_colorStops[stopNum * 5 + 2] = stopGreen;
    m_gradient->m_colorStops[stopNum * 5 + 3] = stopBlue;
    m_gradient->m_colorStops[stopNum * 5 + 4] = stopAlpha;

    return m_gradient;
}

void Gradient::platformDestroy()
{
    delete m_gradient;
    m_gradient = 0;
}

void Gradient::fill(GraphicsContext* context, const FloatRect& rect)
{
    platformGradient();
    context->save();
    context->setFillGradient(this);
    context->fillRect(rect);
    context->restore();
}

}
