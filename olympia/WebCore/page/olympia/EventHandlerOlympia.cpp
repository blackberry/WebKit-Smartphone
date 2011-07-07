/*
 * Copyright (C) 2009 Torch Mobile Inc. http://www.torchmobile.com/
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 */

#include "config.h"
#include "EventHandler.h"

#include "Clipboard.h"
#include "FocusController.h"
#include "Frame.h"
#include "KeyboardEvent.h"
#include "MouseEventWithHitTestResults.h"
#include "Page.h"
#include "PlatformKeyboardEvent.h"
#include "PlatformMouseEvent.h"
#include "PlatformWheelEvent.h"
#include "Widget.h"

#include "NotImplemented.h"

namespace WebCore {

bool EventHandler::eventActivatedView(PlatformMouseEvent const&) const
{
    notImplemented();
    return false;
}

bool EventHandler::passMouseMoveEventToSubframe(MouseEventWithHitTestResults& mev, Frame* subframe, HitTestResult* hoveredNode)
{
    subframe->eventHandler()->handleMouseMoveEvent(mev.event(), hoveredNode);
    return true;
}

bool EventHandler::passMousePressEventToSubframe(MouseEventWithHitTestResults& mev, Frame* subframe)
{
    subframe->eventHandler()->handleMousePressEvent(mev.event());
    return true;
}

bool EventHandler::passMouseReleaseEventToSubframe(MouseEventWithHitTestResults& mev, Frame* subframe)
{
    subframe->eventHandler()->handleMouseReleaseEvent(mev.event());
    return true;
}

bool EventHandler::passWheelEventToWidget(PlatformWheelEvent&, Widget*)
{
    notImplemented();
    return false;
}

bool EventHandler::passWidgetMouseDownEventToWidget(MouseEventWithHitTestResults const&)
{
    notImplemented();
    return false;
}

bool EventHandler::tabsToAllControls(KeyboardEvent*) const
{
    return true;
}

unsigned int EventHandler::accessKeyModifiers()
{
    return PlatformKeyboardEvent::AltKey;
}

void EventHandler::focusDocumentView()
{
    Page* page = m_frame->page();
    if (!page)
        return;
    page->focusController()->setFocusedFrame(m_frame);
}

#if ENABLE(DRAG_SUPPORT)
WTF::PassRefPtr<Clipboard> EventHandler::createDraggingClipboard() const
{
    notImplemented();
    return 0;
}
#endif

} // namespace WebCore
