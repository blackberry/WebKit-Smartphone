/*
 * This file is part of the WebKit project.
 *
 * Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies)
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
 *
 */

#include "config.h"
#include "PlatformTouchPoint.h"
#include "OlympiaPlatformInputEvents.h"

#if ENABLE(TOUCH_EVENTS)

namespace WebCore {

PlatformTouchPoint::PlatformTouchPoint(const Olympia::Platform::TouchPoint& point)
    : m_id(point.m_id)
    , m_screenPos(point.m_screenPos)
    , m_pos(point.m_pos)
{
    switch (point.m_state) {
        case Olympia::Platform::TouchPoint::TouchReleased:
            m_state = TouchReleased;
            break;
        case Olympia::Platform::TouchPoint::TouchMoved:
            m_state = TouchMoved;
            break;
        case Olympia::Platform::TouchPoint::TouchPressed:
            m_state = TouchPressed;
            break;
        case Olympia::Platform::TouchPoint::TouchStationary:
            m_state = TouchStationary;
            break;
    }
}

}

#endif
