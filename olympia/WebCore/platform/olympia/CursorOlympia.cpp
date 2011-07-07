/*
 * Copyright (C) 2009 Torch Mobile Inc. http://www.torchmobile.com/
 */

#include "config.h"

#include "CachedImage.h"
#include "Cursor.h"
#include "Image.h"

#include "NotImplemented.h"

#include <string>

typedef Olympia::Platform::CursorType CursorType;

namespace WebCore {

struct AllCursors {
    AllCursors()
    {
        for (int i = 0; i < Olympia::Platform::NumCursorTypes; ++i)
            m_cursors[i] = Cursor(Olympia::Platform::OlympiaCursor((CursorType)i));
    }
    Cursor m_cursors[Olympia::Platform::NumCursorTypes];
};

static const Cursor& getCursor(CursorType type)
{
    static AllCursors allCursors;
    return allCursors.m_cursors[type];
}

Cursor::Cursor(const Cursor& other)
    : m_impl(other.m_impl)
{
}

Cursor::Cursor(Image* image, const IntPoint& hotspot)
: m_impl(Olympia::Platform::CursorCustomized, std::string(static_cast<CachedImage*>(image->imageObserver())->url().utf8().data()), hotspot)
{
}

Cursor::~Cursor()
{
}

Cursor& Cursor::operator=(const Cursor& other)
{
    m_impl = other.m_impl;
    return *this;
}

Cursor::Cursor(PlatformCursor c)
: m_impl(c)
{
}

const Cursor& aliasCursor()
{
    return getCursor(Olympia::Platform::CursorAlias);
}

const Cursor& cellCursor()
{
    return getCursor(Olympia::Platform::CursorCell);
}

const Cursor& columnResizeCursor()
{
    return getCursor(Olympia::Platform::CursorColumnResize);
}

const Cursor& contextMenuCursor()
{
    return getCursor(Olympia::Platform::CursorContextMenu);
}

const Cursor& copyCursor()
{
    return getCursor(Olympia::Platform::CursorCopy);
}

const Cursor& crossCursor()
{
    return getCursor(Olympia::Platform::CursorCross);
}

const Cursor& eastResizeCursor()
{
    return getCursor(Olympia::Platform::CursorEastResize);
}

const Cursor& eastWestResizeCursor()
{
    return getCursor(Olympia::Platform::CursorEastWestResize);
}

const Cursor& grabbingCursor()
{
    return getCursor(Olympia::Platform::CursorHand);
}

const Cursor& grabCursor()
{
    return getCursor(Olympia::Platform::CursorHand);
}

const Cursor& handCursor()
{
    return getCursor(Olympia::Platform::CursorHand);
}

const Cursor& helpCursor()
{
    return getCursor(Olympia::Platform::CursorHelp);
}

const Cursor& iBeamCursor()
{
    return getCursor(Olympia::Platform::CursorBeam);
}

const Cursor& moveCursor()
{
    return getCursor(Olympia::Platform::CursorMove);
}

const Cursor& noDropCursor()
{
    return getCursor(Olympia::Platform::CursorNoDrop);
}

const Cursor& noneCursor()
{
    return getCursor(Olympia::Platform::CursorNone);
}

const Cursor& northEastResizeCursor()
{
    return getCursor(Olympia::Platform::CursorNorthEastResize);
}

const Cursor& northEastSouthWestResizeCursor()
{
    return getCursor(Olympia::Platform::CursorNorthEastSouthWestResize);
}

const Cursor& northResizeCursor()
{
    return getCursor(Olympia::Platform::CursorNorthResize);
}

const Cursor& northSouthResizeCursor()
{
    return getCursor(Olympia::Platform::CursorNorthSouthResize);
}

const Cursor& northWestResizeCursor()
{
    return getCursor(Olympia::Platform::CursorNorthWestResize);
}

const Cursor& northWestSouthEastResizeCursor()
{
    return getCursor(Olympia::Platform::CursorNorthWestSouthEastResize);
}

const Cursor& notAllowedCursor()
{
    return getCursor(Olympia::Platform::CursorNotAllowed);
}

const Cursor& pointerCursor()
{
    return getCursor(Olympia::Platform::CursorPointer);
}

const Cursor& progressCursor()
{
    return getCursor(Olympia::Platform::CursorProgress);
}

const Cursor& rowResizeCursor()
{
    return getCursor(Olympia::Platform::CursorRowResize);
}

const Cursor& southEastResizeCursor()
{
    return getCursor(Olympia::Platform::CursorSouthEastResize);
}

const Cursor& southResizeCursor()
{
    return getCursor(Olympia::Platform::CursorSouthResize);
}

const Cursor& southWestResizeCursor()
{
    return getCursor(Olympia::Platform::CursorSouthWestResize);
}

const Cursor& verticalTextCursor()
{
    return getCursor(Olympia::Platform::CursorVerticalText);
}

const Cursor& waitCursor()
{
    return getCursor(Olympia::Platform::CursorWait);
}

const Cursor& westResizeCursor()
{
    return getCursor(Olympia::Platform::CursorWestResize);
}

const Cursor& zoomInCursor()
{
    return getCursor(Olympia::Platform::CursorZoomIn);
}

const Cursor& zoomOutCursor()
{
    return getCursor(Olympia::Platform::CursorZoomOut);
}

const Cursor& middlePanningCursor()
{
    return moveCursor();
}

const Cursor& eastPanningCursor()
{
    return eastResizeCursor();
}

const Cursor& northPanningCursor()
{
    return northResizeCursor();
}

const Cursor& northEastPanningCursor()
{
    return northEastResizeCursor();
}

const Cursor& northWestPanningCursor()
{
    return northWestResizeCursor();
}

const Cursor& southPanningCursor()
{
    return southResizeCursor();
}

const Cursor& southEastPanningCursor()
{
    return southEastResizeCursor();
}

const Cursor& southWestPanningCursor()
{
    return southWestResizeCursor();
}

const Cursor& westPanningCursor()
{
    return westResizeCursor();
}

} // namespace WebCore
