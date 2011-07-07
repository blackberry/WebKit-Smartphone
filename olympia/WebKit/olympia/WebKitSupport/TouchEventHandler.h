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
template <typename T> class Timer;
}

namespace Olympia {
namespace WebKit {

class WebPagePrivate;

class TouchEventHandler {
public:
    TouchEventHandler(WebPagePrivate* webpage);
    ~TouchEventHandler();

    bool handleTouchEvent(Olympia::Platform::TouchEvent& event);

    void touchPressedTimeout(WebCore::Timer<TouchEventHandler>*);
    void touchReleaseTimeout(WebCore::Timer<TouchEventHandler>*);

    void setDidScrollPage(bool scrolled) { m_didScrollPage = scrolled; }
    void setDidZoomPage(bool zoomed) { m_didZoomPage = zoomed; }
    void setDidMoveFinger(bool moved) { m_didMoveFinger = moved; }

private:
    WebCore::IntPoint getFatFingerPos(const WebCore::IntPoint winPos);
    void touchPressedTimeout(WebCore::Timer<WebPagePrivate>*);

    WebPagePrivate* m_webPage;

    bool m_didScrollPage;
    bool m_didMoveFinger;
    bool m_didZoomPage;
    bool m_didCancelTouch;

    bool m_touchDownTimerStarted;

    WebCore::IntPoint m_touchDownPoint; // Screen Position
    WebCore::IntPoint m_fatFingerPoint; // Content Position
    WebCore::IntPoint m_lastTouchPoint; // Screen Position

    WebCore::Timer<TouchEventHandler>* m_touchDownTimer;
    RefPtr<WebCore::Node> m_subframeNode;
};

}
}

#endif // TouchEventHandler_h
