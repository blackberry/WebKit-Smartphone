/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 */

#include "config.h"
#include "TouchEventHandler.h"

#include "Document.h"
#include "FocusController.h"
#include "Frame.h"
#include "HitTestResult.h"
#include "HTMLInputElement.h"
#include "IntPoint.h"
#include "IntRect.h"
#include "IntSize.h"
#include "Node.h"
#include "Page.h"
#include "PlatformMouseEvent.h"
#include "PlatformTouchEvent.h"
#include "Timer.h"
#include "WebPage_p.h"
#include "WebSettings.h"

#include <wtf/MathExtras.h>

const float touchDownTimeout = 0.15; // By trial and error.
const unsigned fatFingerRad = 20; // Half of the width of the fat finger rect.
const unsigned halfFatFingerDiagonal = ceilf(fatFingerRad * sqrt(2.0));

using namespace Olympia::Platform;
using namespace WebCore;

namespace Olympia {
namespace WebKit {

static inline int distanceBetweenPoints(WebCore::IntPoint p1, WebCore::IntPoint p2)
{
    float dx = p1.x() - p2.x();
    float dy = p1.y() - p2.y();
    return sqrt((dx * dx) + (dy * dy));
}

static inline WebCore::IntPoint closestPointInRect(WebCore::IntRect rect, WebCore::IntPoint point)
{
    if (rect.contains(point)) // point inside rect.
        return point;

    if (point.x() > rect.x() && point.x() < rect.right()) { // directly on top or bottom
        if (point.y() > rect.y())
            return WebCore::IntPoint(point.x(), rect.y() + 1); // top
        return WebCore::IntPoint(point.x(), rect.bottom() - 1); // bottom
    }

    if (point.y() > rect.y() && point.y() < rect.bottom()) { // directly left or right
        if (point.x() < rect.x())
            return WebCore::IntPoint(rect.x() + 1, point.y()); // left
        return WebCore::IntPoint(rect.right() - 1, point.y()); // right
    }

    if (point.x() < rect.x() && point.y() > rect.y()) // top left
        return WebCore::IntPoint(rect.x() + 1, rect.y() - 1);
    if (point.x() < rect.x() && point.y() < rect.bottom()) // bottom left
        return WebCore::IntPoint(rect.x() + 1, rect.bottom() + 1);
    if (point.x() > rect.right() && point.y() < rect.y()) // top right
        return WebCore::IntPoint(rect.right() - 1, rect.y() - 1);

    return WebCore::IntPoint(rect.right() - 1, rect.bottom() + 1); // bottom right
}

TouchEventHandler::TouchEventHandler(WebPagePrivate* webpage)
    : m_webPage(webpage)
    , m_didScrollPage(0)
    , m_didMoveFinger(0)
    , m_didZoomPage(0)
    , m_touchDownTimerStarted(0)
    , m_subframeNode(0)
{
    m_touchDownTimer = new Timer<TouchEventHandler>(this, &TouchEventHandler::touchPressedTimeout);
}

TouchEventHandler::~TouchEventHandler()
{
    delete m_touchDownTimer;
    m_touchDownTimer = 0;
}

bool TouchEventHandler::handleTouchEvent(Olympia::Platform::TouchEvent& event)
{
    if (event.m_type == Olympia::Platform::TouchEvent::TouchStart)
        m_didCancelTouch = false;

    // Clear stuff if canceled and consume event.
    // FIXME: Look into clearing focus node even if javascript consumes cancel event.
    if (event.m_type == Olympia::Platform::TouchEvent::TouchCancel) {
        m_webPage->clearFocusNode();
        m_didCancelTouch = true;
        m_subframeNode = 0;
        return true;
    }

    // Do not handle touch event if multi-touch
    if (event.m_singleType == Olympia::Platform::TouchEvent::SingleNone)
        return false;

    if (event.m_singleType == Olympia::Platform::TouchEvent::SinglePressed) {
        setDidScrollPage(false);
        setDidMoveFinger(false);
        setDidZoomPage(false);
        m_didCancelTouch = false;
        m_touchDownPoint = event.m_points[0].m_screenPos;
        m_lastTouchPoint = event.m_points[0].m_screenPos;
        m_fatFingerPoint = event.m_points[0].m_pos;
        m_subframeNode = 0;
        m_touchDownTimerStarted = true;
        m_touchDownTimer->startOneShot(touchDownTimeout);

    } else if (event.m_singleType == Olympia::Platform::TouchEvent::SingleReleased) {
        m_subframeNode = 0;
        // Create Mouse Event.
        WebCore::PlatformMouseEvent mouseEvent(m_webPage->mapFromContentsToViewport(m_fatFingerPoint), m_fatFingerPoint, WebCore::MouseEventMoved, 1, WebCore::TouchScreen);

        // Update the mouse position with a MouseMoved event
        m_webPage->handleMouseEvent(mouseEvent);

        // Send a Mouse press event if not already pressed
        // This can occur if we receive events before the touchPressed timeout occurs.
        if (!m_webPage->m_mainFrame->eventHandler()->mousePressed()) {
            mouseEvent.setType(WebCore::MouseEventPressed);
            m_webPage->handleMouseEvent(mouseEvent);
        }

        // Finally send a MouseReleased event.
        mouseEvent.setType(WebCore::MouseEventReleased);
        m_webPage->handleMouseEvent(mouseEvent);

    } else if (event.m_singleType == Olympia::Platform::TouchEvent::SingleMoved) {
        // If there is a subframe or div to scroll try scrolling it otherwise returns false and let chrome scroll the page.
        if (!m_subframeNode)
            m_subframeNode = m_webPage->m_mainFrame->eventHandler()->hitTestResultAtPoint(event.m_points[0].m_pos, false).innerNode();
        WebCore::IntSize delta = m_lastTouchPoint - event.m_points[0].m_screenPos;
        m_lastTouchPoint = event.m_points[0].m_screenPos;
        return m_webPage->scrollNodeRecursively(m_subframeNode.get(), delta);
    }
    return true;
}

void TouchEventHandler::touchPressedTimeout(WebCore::Timer<TouchEventHandler>*)
{
    m_touchDownTimerStarted = false;
    if (!m_didZoomPage && !m_didScrollPage && !m_didCancelTouch) {

        Node* node = m_webPage->m_mainFrame->eventHandler()->hitTestResultAtPoint(m_fatFingerPoint, false).innerNode();
        if (!(node && node->isFocusable()))
            m_fatFingerPoint = getFatFingerPos(m_fatFingerPoint);

        // Convert touch event to a mouse event
        // First update the mouse position with a MouseMoved event
        WebCore::PlatformMouseEvent mouseEvent(m_webPage->mapFromContentsToViewport(m_fatFingerPoint), m_fatFingerPoint, WebCore::MouseEventMoved, 1, WebCore::TouchScreen);
        m_webPage->handleMouseEvent(mouseEvent);

        // Then send the MousePressed event
        mouseEvent.setType(WebCore::MouseEventPressed);
        m_webPage->handleMouseEvent(mouseEvent);
    }
}

WebCore::IntPoint TouchEventHandler::getFatFingerPos(const WebCore::IntPoint pos)
{
    Document* doc = m_webPage->m_mainFrame->document();
    if (!doc)
        return pos;

    // The layout needs to be up-to-date to determine if a node is focusable.
    doc->updateLayoutIgnorePendingStylesheets();

    Node* startNode = doc->nextFocusableNode(0, 0);
    if (!startNode)
        return pos;

    // Create fingerRect.
    WebCore::IntPoint screenPoint = m_webPage->mapToTransformed(pos);
    // screenFingerRect is a square with length fatFingerRad*2 pixels on the physical screen.
    WebCore::IntRect screenFingerRect(screenPoint.x() - fatFingerRad, screenPoint.y() - fatFingerRad, fatFingerRad * 2, fatFingerRad * 2);
    WebCore::IntRect fingerRect = m_webPage->mapFromTransformed(screenFingerRect);

    Node* curNode = startNode;
    Node* bestNode = 0;
    unsigned bestDistance = halfFatFingerDiagonal;
    WebCore::IntPoint bestPoint;

    do {
        WebCore::IntRect curRect = curNode->getRect();
        if (fingerRect.intersects(curNode->getRect())) {
            WebCore::IntPoint closestPoint = closestPointInRect(curNode->getRect(), pos);
            int dist = distanceBetweenPoints(closestPoint, pos);
            if (!bestNode || dist < bestDistance) {
                bestNode = curNode;
                bestDistance = dist;
                bestPoint = closestPoint;
            }
        }
        curNode = doc->nextFocusableNode(curNode, 0);
    } while (curNode && curNode != startNode);

    if (bestNode && bestPoint != pos) {
        if (bestNode->hasTagName(HTMLNames::inputTag)) {
            HTMLInputElement* input = static_cast<HTMLInputElement*>(bestNode);
            if (input && (input->isTextButton() || input->isRadioButton() || input->inputType() == HTMLInputElement::CHECKBOX)) {
                WebCore::IntRect nodeRect = bestNode->getRect();
                bestPoint.setX(nodeRect.x() + nodeRect.width() / 2);
                bestPoint.setY(nodeRect.y() + nodeRect.height() / 2);
            }
        } else if (bestNode->isLink()) {
            WebCore::IntRect nodeRect = bestNode->getRect();
            bestPoint.setX(nodeRect.x() + nodeRect.width() / 2);
            bestPoint.setY(nodeRect.y() + nodeRect.height() / 2);
        }
        return bestPoint;
    }
    return pos;
}

}
}
