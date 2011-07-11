/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 */

#ifndef FrameNetworkingContextOlympia_h
#define FrameNetworkingContextOlympia_h

#include "FrameNetworkingContext.h"

namespace WebCore {

class FrameNetworkingContextBlackBerry : public FrameNetworkingContext {
public:
    static PassRefPtr<FrameNetworkingContextBlackBerry> create(Frame*);

    virtual Frame* wrappedFrame() const { return frame(); }

private:
    FrameNetworkingContextBlackBerry(Frame*);
};

}

#endif // FrameNetworkingContextOlympia_h
