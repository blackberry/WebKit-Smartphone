/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 */

#include "config.h"

#include "FrameNetworkingContextBlackBerry.h"

namespace WebCore {

FrameNetworkingContextBlackBerry::FrameNetworkingContextBlackBerry(Frame* frame)
    : FrameNetworkingContext(frame)
{
}

PassRefPtr<FrameNetworkingContextBlackBerry> FrameNetworkingContextBlackBerry::create(Frame* frame)
{
    return adoptRef(new FrameNetworkingContextBlackBerry(frame));
}

}
