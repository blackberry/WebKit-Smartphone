/*
 * Copyright (C) 2009 Torch Mobile Inc. http://www.torchmobile.com/
 */

#include "config.h"
#include "DragClientBlackBerry.h"

#include "NotImplemented.h"

namespace WebCore {

void DragClientBlackBerry::willPerformDragDestinationAction(DragDestinationAction, DragData*)
{
    notImplemented();
}

void DragClientBlackBerry::willPerformDragSourceAction(DragSourceAction, const IntPoint&, Clipboard*) 
{
    notImplemented();
}

DragDestinationAction DragClientBlackBerry::actionMaskForDrag(DragData*)
{
    notImplemented();
    return DragDestinationActionNone;
}

DragSourceAction DragClientBlackBerry::dragSourceActionMaskForPoint(const IntPoint&)
{
    notImplemented();
    return DragSourceActionNone;
}

void DragClientBlackBerry::startDrag(void*, const IntPoint&, const IntPoint&, Clipboard*, Frame*, bool)
{
    notImplemented();
}

void* DragClientBlackBerry::createDragImageForLink(KURL&, const String&, Frame*)
{
    notImplemented();
    return 0;
}

void DragClientBlackBerry::dragControllerDestroyed()
{
    delete this;
}

} // namespace WebCore
