/*
 * Copyright (C) 2009 Torch Mobile Inc. http://www.torchmobile.com/
 */

#ifndef DragClientOlympia_h
#define DragClientOlympia_h

#include "DragClient.h"

namespace WebCore {

class DragClientOlympia : public DragClient {
public:
    virtual void willPerformDragDestinationAction(DragDestinationAction, DragData*);
    virtual void willPerformDragSourceAction(DragSourceAction, const IntPoint&, Clipboard*);
    virtual DragDestinationAction actionMaskForDrag(DragData*);
    virtual DragSourceAction dragSourceActionMaskForPoint(const IntPoint&);
    virtual void startDrag(void*, const IntPoint&, const IntPoint&, Clipboard*, Frame*, bool);
    virtual void* createDragImageForLink(KURL&, const String&, Frame*);
    virtual void dragControllerDestroyed();
};

} // WebCore

#endif // DragClientOlympia_h
