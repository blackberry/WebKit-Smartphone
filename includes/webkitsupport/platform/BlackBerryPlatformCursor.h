/*
 * Copyright (C) 2009-2010 Research In Motion Limited. http://www.rim.com/
 */

#ifndef BlackBerryPlatformCursor_h
#define BlackBerryPlatformCursor_h

#include "BlackBerryPlatformPrimitives.h"

#include <string>

namespace BlackBerry {
    namespace Platform {

        enum CursorType {
            CursorNone = 0,
            CursorAlias,
            CursorPointer,
            CursorCell,
            CursorCopy,
            CursorCross,
            CursorHand,
            CursorMove,
            CursorBeam,
            CursorWait,
            CursorHelp,
            CursorEastResize,
            CursorNorthResize,
            CursorNorthEastResize,
            CursorNorthWestResize,
            CursorSouthResize,
            CursorSouthEastResize,
            CursorSouthWestResize,
            CursorWestResize,
            CursorNorthSouthResize,
            CursorEastWestResize,
            CursorNorthEastSouthWestResize,
            CursorNorthWestSouthEastResize,
            CursorColumnResize,
            CursorRowResize,
            CursorVerticalText,
            CursorContextMenu,
            CursorNoDrop,
            CursorNotAllowed,
            CursorProgress,
            CursorZoomIn,
            CursorZoomOut,
            CursorCustomized,
            NumCursorTypes
        };

        struct BlackBerryCursor {
            BlackBerryCursor(const CursorType type = CursorNone, const std::string& url = std::string(), const IntPoint& point = IntPoint(0, 0)) : m_type(type), m_url(url), m_hotspot(point) {}

            CursorType type() const { return m_type; }
            std::string url() const { return m_url; }
            IntPoint hotspot() const { return m_hotspot; }

            BlackBerryCursor& operator=(const BlackBerryCursor& other)
            {
                m_type = other.type();
                m_url = other.url();
                m_hotspot = other.hotspot();
                return *this;
            }

            CursorType m_type;
            std::string m_url;
            IntPoint m_hotspot;
        };

    } // Platform

} // Olympia
#endif // BlackBerryPlatformCursor_h
