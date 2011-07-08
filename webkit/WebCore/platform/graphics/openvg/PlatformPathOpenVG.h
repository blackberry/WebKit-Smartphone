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

#ifndef PlatformPathOpenVG_h
#define PlatformPathOpenVG_h

#include "FloatPoint.h"
#include "SharedResourceOpenVG.h"

#include <openvg.h>
#include <wtf/Vector.h>

namespace WebCore {

class PlatformPathOpenVG : public SharedResourceOpenVG {
public:
    PlatformPathOpenVG();
    PlatformPathOpenVG(const PlatformPathOpenVG&);
    ~PlatformPathOpenVG();

    PlatformPathOpenVG& operator=(const PlatformPathOpenVG&);

    void appendPathData(VGuint commandCount, const VGubyte* commands, const VGfloat* coords);
    void flushPathDataBuffer() const;
    VGPath vgPath() const;
    void clear();

    static inline unsigned int numberOfCoordinatesForCommand(VGubyte command)
    {
        switch (command) {
        case VG_CLOSE_PATH:
            return 0;
        case VG_MOVE_TO_ABS:
        case VG_MOVE_TO_REL:
        case VG_LINE_TO_ABS:
        case VG_LINE_TO_REL:
            return 2;
        case VG_HLINE_TO_ABS:
        case VG_HLINE_TO_REL:
        case VG_VLINE_TO_ABS:
        case VG_VLINE_TO_REL:
            return 1;
        case VG_QUAD_TO_ABS:
        case VG_QUAD_TO_REL:
            return 4;
        case VG_CUBIC_TO_ABS:
        case VG_CUBIC_TO_REL:
            return 6;
        case VG_SQUAD_TO_ABS:
        case VG_SQUAD_TO_REL:
            return 2;
        case VG_SCUBIC_TO_ABS:
        case VG_SCUBIC_TO_REL:
            return 4;
        case VG_SCCWARC_TO_ABS:
        case VG_SCCWARC_TO_REL:
        case VG_SCWARC_TO_ABS:
        case VG_SCWARC_TO_REL:
        case VG_LCCWARC_TO_ABS:
        case VG_LCCWARC_TO_REL:
        case VG_LCWARC_TO_ABS:
        case VG_LCWARC_TO_REL:
            return 5;
        default:
            ASSERT_NOT_REACHED();
            return 0;
        }
    }

public:
    FloatPoint m_currentPoint;
    FloatPoint m_subpathStartPoint;

private:
    void createPath();

    VGPath m_vgPath;
    mutable Vector<VGubyte> m_commandBuffer;
    mutable Vector<VGfloat> m_coordinateBuffer;
};

}

#endif
