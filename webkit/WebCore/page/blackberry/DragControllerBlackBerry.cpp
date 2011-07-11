/*
 * Copyright (C) 2009 Torch Mobile Inc. http://www.torchmobile.com/
 */

#include "config.h"
#if ENABLE(DRAG_SUPPORT)
#include "DragController.h"

#include "DragData.h"
#include "EventHandler.h"
#include "IntSize.h"

#include "NotImplemented.h"

namespace WebCore {

// FIXME: remove this when no stubs use it anymore
static const WebCore::IntSize _nullSize = WebCore::IntSize();

const double EventHandler::TextDragDelay = 0.0;
const int DragController::LinkDragBorderInset = 2;
const int DragController::MaxOriginalImageArea = 1500 * 1500;
const int DragController::DragIconRightInset = 7;
const int DragController::DragIconBottomInset = 3;
const float DragController::DragImageAlpha = 0.75f;

bool DragController::isCopyKeyDown()
{
    notImplemented();
    return false;
}

const IntSize& DragController::maxDragImageSize()
{
    notImplemented();
    return _nullSize;
}

void DragController::cleanupAfterSystemDrag()
{
    notImplemented();
}

DragOperation DragController::dragOperation(DragData*)
{
    notImplemented();
    return DragOperationNone;
}
} // namespace WebCore
#endif
