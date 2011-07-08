/*
 * Copyright (C) 2009 Torch Mobile Inc. http://www.torchmobile.com/
 */

#include "config.h"
#include "DragClientOlympia.h"

#include "NotImplemented.h"

namespace WebCore {

void DragClientOlympia::willPerformDragDestinationAction(DragDestinationAction, DragData*)
{
    notImplemented();
}

void DragClientOlympia::willPerformDragSourceAction(DragSourceAction, const IntPoint&, Clipboard*) 
{
    notImplemented();
}

DragDestinationAction DragClientOlympia::actionMaskForDrag(DragData*)
{
    notImplemented();
    return DragDestinationActionNone;
}

DragSourceAction DragClientOlympia::dragSourceActionMaskForPoint(const IntPoint&)
{
    notImplemented();
    return DragSourceActionNone;
}

void DragClientOlympia::startDrag(void*, const IntPoint&, const IntPoint&, Clipboard*, Frame*, bool)
{
    notImplemented();
}

void* DragClientOlympia::createDragImageForLink(KURL&, const String&, Frame*)
{
    notImplemented();
    return 0;
}

void DragClientOlympia::dragControllerDestroyed()
{
    delete this;
}

} // namespace WebCore
