/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 */

#ifndef TouchEventHandler_h
#define TouchEventHandler_h

#include "IntPoint.h"
#include "OlympiaPlatformInputEvents.h"

namespace WebCore {
class IntRect;
class Node;
}

namespace Olympia {
namespace WebKit {

class WebPagePrivate;

class TouchEventHandler {
public:
    TouchEventHandler(WebPagePrivate* webpage);
    ~TouchEventHandler();

    bool handleTouchEvent(Olympia::Platform::SingleTouchEvent&);
    void touchEventCancel();
    void touchEventCancelAndClearFocusedNode();

private:
    WebCore::IntPoint getFatFingerPos(const WebCore::IntPoint contentPos);
    void handleFatFingerPressed();

    WebPagePrivate* m_webPage;

    bool m_didCancelTouch;
    bool m_convertTouchToMouse; // This is only for range slider event, showing we need to convert Touch to Mouse event or not.

    WebCore::IntPoint m_fatFingerPoint; // Content Position
    WebCore::IntPoint m_lastScreenPoint; // Screen Position

    RefPtr<WebCore::Node> m_subframeNode;
};

}
}

#endif // TouchEventHandler_h
