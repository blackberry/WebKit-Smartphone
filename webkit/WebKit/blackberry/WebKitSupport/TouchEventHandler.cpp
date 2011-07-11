/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 */

#include "config.h"
#include "TouchEventHandler.h"

#include "HTMLNames.h"
#include "CSSComputedStyleDeclaration.h"
#include "CSSParser.h"
#include "Document.h"
#include "FocusController.h"
#include "Frame.h"
#include "HTMLInputElement.h"
#include "HitTestResult.h"
#include "IntRect.h"
#include "IntSize.h"
#include "Node.h"
#include "NodeList.h"
#include "Page.h"
#include "PlatformMouseEvent.h"
#include "PlatformTouchEvent.h"
#include "WebPage_p.h"
#include "WebSettings.h"

#include <wtf/MathExtras.h>

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

static bool hasMouseListener(WebCore::Element* element)
{
    ASSERT(element);
    return element->hasEventListeners(eventNames().clickEvent)
        || element->hasEventListeners(eventNames().mousedownEvent)
        || element->hasEventListeners(eventNames().mouseupEvent);
}

static bool isElementClickable(WebCore::Element* element)
{
    ASSERT(element);

    ExceptionCode ec = 0;

    // FIXME: We fall back to checking for the presence of CSS style "cursor: pointer" to indicate whether the element A
    // can be clicked when A neither registers mouse events handlers nor is a hyperlink or form control. This workaround
    // ensures that we don't break various Google web apps, including <http://maps.google.com>. Ideally, we should walk
    // up the DOM hierarchy to determine the first parent element that accepts mouse events.
    // Consider the HTML snippet: <div id="A" onclick="..."><div id="B">Example</div></div>
    // Notice, B is not a hyperlink, or form control, and does not register any mouse event handler. Then B cannot
    // be clicked. Suppose B specified the CSS property "cursor: pointer". Then, B will be considered as clickable.
    return hasMouseListener(element)
        || element->webkitMatchesSelector("a,*:link,*:visited,*[role=button],button,input,select,label,area[href]", ec)
        || computedStyle(element)->getPropertyValue(cssPropertyID("cursor")) == "pointer";
}

static inline WebCore::IntRect fingerRectFromPoint(const WebCore::IntPoint& point)
{
    // NOTE: The implementation of this method has to be sync'ed with HitTestResult::regionFromPoint.
    int length = 2 * fatFingerRad + 1;
    return WebCore::IntRect(point.x() - fatFingerRad, point.y() - fatFingerRad, length, length);
}

TouchEventHandler::TouchEventHandler(WebPagePrivate* webpage)
    : m_webPage(webpage)
    , m_didCancelTouch(0)
    , m_subframeNode(0)
    , m_convertTouchToMouse(false)
{
}

TouchEventHandler::~TouchEventHandler()
{
}

void TouchEventHandler::touchEventCancel()
{
    m_convertTouchToMouse = false; // We cancel converting touch event to mouse event when touch event has been cancelled.
    m_webPage->m_mainFrame->eventHandler()->setMousePressed(false);
    m_didCancelTouch = true;
    m_subframeNode = 0;
}

void TouchEventHandler::touchEventCancelAndClearFocusedNode()
{
    touchEventCancel();
    m_webPage->clearFocusNode();
}

bool TouchEventHandler::handleTouchEvent(Olympia::Platform::SingleTouchEvent& event)
{
    switch (event.m_singleType) {
    case Olympia::Platform::SingleTouchEvent::SinglePressed:
        {
            m_didCancelTouch = false;
            m_lastScreenPoint = event.m_screenPos;
            m_fatFingerPoint = m_webPage->mapFromViewportToContents(event.m_pos);
            m_subframeNode = 0;
            handleFatFingerPressed();
            HitTestResult result = m_webPage->m_mainFrame->eventHandler()->hitTestResultAtPoint(m_fatFingerPoint, false);
            WebCore::Node* node = result.innerNode();
            if (node && node->hasTagName(HTMLNames::inputTag)) {
                HTMLInputElement* inputElement = static_cast<HTMLInputElement*>(node);
                if (inputElement->inputType() == HTMLInputElement::RANGE) {
                    // We flag that subsequent touch events should be treated as mouse event
                    // so as to support touch-dragging the range element's slider handle.
                    m_convertTouchToMouse = true;
                }
            }
            return true;
        }
    case Olympia::Platform::SingleTouchEvent::SingleReleased:
        {
            // If it is on a range slider, we convert touch event to mouse event, and go back to normal when touch released.
            if (m_convertTouchToMouse) {
                WebCore::PlatformMouseEvent mouseEvent(event.m_pos, event.m_screenPos, WebCore::MouseEventReleased, 1, WebCore::TouchScreen);
                m_webPage->handleMouseEvent(mouseEvent);
                m_convertTouchToMouse = false;
                return true;
            }
            m_subframeNode = 0;
            // Create Mouse Event.
            WebCore::PlatformMouseEvent mouseEvent(m_webPage->mapFromContentsToViewport(m_fatFingerPoint), m_lastScreenPoint, WebCore::MouseEventMoved, 1, WebCore::TouchScreen);

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
            return true;
        }
    case Olympia::Platform::SingleTouchEvent::SingleMoved:
        {
            // If it is on a range slider, we convert touch event to mouse event and do not scroll the page. This is for range slider only.
            if (m_convertTouchToMouse) {
                WebCore::PlatformMouseEvent mouseEvent(event.m_pos, event.m_screenPos, WebCore::MouseEventMoved, 1, WebCore::TouchScreen);
                m_webPage->handleMouseEvent(mouseEvent);
                return true;
            }
            // If there is a subframe or div to scroll try scrolling it otherwise returns false and let chrome scroll the page.
            if (!m_subframeNode) {
                HitTestResult result = m_webPage->m_mainFrame->eventHandler()->hitTestResultAtPoint(m_webPage->mapFromViewportToContents(event.m_pos), false);
                m_subframeNode = result.innerNode();
            }
            WebCore::IntSize delta = m_webPage->mapToTransformed(m_lastScreenPoint) - m_webPage->mapToTransformed(event.m_screenPos);
            m_lastScreenPoint = event.m_screenPos;
            return m_webPage->scrollNodeRecursively(m_subframeNode.get(), delta);
        }
    default:
        break;
    }
    return false;
}

void TouchEventHandler::handleFatFingerPressed()
{
    if (!m_didCancelTouch) {

        m_fatFingerPoint = getFatFingerPos(m_fatFingerPoint);

        // Convert touch event to a mouse event
        // First update the mouse position with a MouseMoved event
        WebCore::PlatformMouseEvent mouseEvent(m_webPage->mapFromContentsToViewport(m_fatFingerPoint), m_lastScreenPoint, WebCore::MouseEventMoved, 1, WebCore::TouchScreen);
        m_webPage->handleMouseEvent(mouseEvent);

        // Then send the MousePressed event
        mouseEvent.setType(WebCore::MouseEventPressed);
        m_webPage->handleMouseEvent(mouseEvent);
    }
}

WebCore::IntPoint TouchEventHandler::getFatFingerPos(const WebCore::IntPoint contentPos)
{
    Document* doc = m_webPage->m_mainFrame->document();
    if (!doc)
        return contentPos;

    // The layout needs to be up-to-date to determine if a node is focusable.
    doc->updateLayoutIgnorePendingStylesheets();

    // Create fingerRect.
    WebCore::IntPoint screenPoint = m_webPage->mapToTransformed(contentPos);
    // screenFingerRect is a square with length fatFingerRad*2 pixels on the physical screen.
    WebCore::IntRect screenFingerRect(fingerRectFromPoint(screenPoint));
    WebCore::IntRect fingerRect = m_webPage->mapFromTransformed(screenFingerRect);

    // Convert 'contentPos' to viewport coordinates as required by Document::nodesFromRect().
    // Internally, nodesFromRect() converts it back to contents coordinates.
    // Both the scroll offset and the current zoom factor of the page are considered during this
    // conversion.
    WebCore::IntPoint viewportPoint = m_webPage->mapFromContentsToViewport(contentPos);

    WTF::RefPtr<WebCore::NodeList> intersectedNodes = doc->nodesFromRect(viewportPoint.x(), viewportPoint.y(), fatFingerRad, fatFingerRad, false);
    if (!intersectedNodes)
        return contentPos;

    Element* bestElement = 0;
    unsigned bestDistance = halfFatFingerDiagonal;
    WebCore::IntPoint bestPoint;

    // Iterate over the list of nodes hit looking for the closest point on the
    // outline of the rect to the given point.
    unsigned length = intersectedNodes->length();
    for (unsigned i = 0; i < length; i++) {
        WebCore::Node* curNode = intersectedNodes->item(i);
        if (!curNode || !curNode->isElementNode())
            continue;

        WebCore::Element* curElement = static_cast<Element*>(curNode);
        if (!curElement || !isElementClickable(curElement))
            continue;

        WebCore::IntRect curRect = curElement->getRect();
        if (fingerRect.intersects(curRect)) {
            WebCore::IntPoint closestPoint = closestPointInRect(curRect, contentPos);
            int dist = distanceBetweenPoints(closestPoint, contentPos);
            if (!bestElement || dist < bestDistance) {
                bestElement = curElement;
                bestDistance = dist;
                bestPoint = closestPoint;
            }
        }
    }

    if (!bestElement || bestPoint == contentPos)
        return contentPos;

    bool shouldUseCenterPointOfBestElement = bestElement->isLink();
    if (!shouldUseCenterPointOfBestElement && bestElement->hasTagName(HTMLNames::inputTag)) {
        HTMLInputElement* input = static_cast<HTMLInputElement*>(bestElement);
        shouldUseCenterPointOfBestElement = input->isTextButton() || input->isRadioButton() || input->inputType() == HTMLInputElement::CHECKBOX;
    }

    if (shouldUseCenterPointOfBestElement)
        bestPoint = bestElement->getRect().center();

    return bestPoint;
}

}
}
