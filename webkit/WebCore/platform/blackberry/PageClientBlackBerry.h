/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 */

#ifndef PageClientOlympia_h
#define PageClientOlympia_h

#include "Cursor.h"

namespace Olympia {
namespace Platform {
    class NetworkStreamFactory;
}
}

namespace WebCore {
    class FloatRect;
}

class PageClientBlackBerry {
public:
    virtual void setCursor(WebCore::PlatformCursorHandle handle) = 0;
    virtual Olympia::Platform::NetworkStreamFactory* networkStreamFactory() = 0;
    virtual WebCore::FloatRect screenAvailableRect() = 0;
    virtual WebCore::FloatRect screenRect() = 0;
};

#endif
